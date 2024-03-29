/*
 *   Copyright (C) 2010,2012,2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <cassert>
#include <boost/algorithm/string.hpp>

#include "NMEASentenceCollector.h"
#include "StringUtils.h"
#include "Log.h"
#include "APRSUtils.h"

CNMEASentenceCollector::CNMEASentenceCollector(const std::string& sentence) :
CSentenceCollector(SLOW_DATA_TYPE_GPS, sentence, '\x0A')
{

}

bool CNMEASentenceCollector::isValidSentence(const std::string& sentence)
{
    return isValidNMEA(sentence);
}

bool CNMEASentenceCollector::isValidNMEA(const std::string& nmea)
{
    if(nmea.empty() || nmea[0] != '$')
        return false;

    auto posStar = nmea.find_last_of('*');
    if(posStar == 0U || posStar == std::string::npos)
        return false;

    auto csum = calcXOR(nmea);
    auto csumStr = CStringUtils::string_format("%02X", csum);
    auto expctedCSumStr = nmea.substr(posStar + 1, 2U);
    auto res =  ::strcasecmp(expctedCSumStr.c_str(), csumStr.c_str()) == 0;
    return res;
}

unsigned char CNMEASentenceCollector::calcXOR(const std::string& nmea)
{
    unsigned char res = 0U;
    
    if(!nmea.empty()) {
        unsigned int i =  nmea[0] == '$' ?  1U : 0U; //skip $ if it is there
        while(i < nmea.length())
        {
            if(nmea[i] != '*') {
                res ^= (unsigned char)nmea[i];
            }
            else {
                break;
            }
            i++;
        }
    }

    return res;
}

unsigned int CNMEASentenceCollector::getDataInt(unsigned char * data, unsigned int length)
{   
    if(data == nullptr || length == 0U)
        return 0U;

    std::string aprsFrame;
    if(!getDataInt(aprsFrame))
        return 0U;

    auto aprsFrameLen = aprsFrame.length();
    if(length < aprsFrameLen) {
        CLog::logDebug("Not enough space to copy NMEA APRS frame");
        return 0U;
    }

    for(unsigned int i = 0U; i < aprsFrameLen; i++){
        data[i] = aprsFrame[i];
    }

    return aprsFrameLen;
}

bool CNMEASentenceCollector::getDataInt(std::string& data)
{
    if(getMyCall1().empty() || getSentence().empty())
        return false;

    data.clear();
    auto nmea = getSentence();
    fixUpNMEATimeStamp(nmea);

    std::string fromCall = getMyCall1();
    CAPRSUtils::dstarCallsignToAPRS(fromCall);

    // 20230425 Fix for https://github.com/F4FXL/DStarGateway/issues/33
    size_t hyphenIndex = fromCall.find('-');
    if(hyphenIndex != std::string::npos) {
        fromCall = fromCall.substr(0, hyphenIndex);
	}

    std::string aprsFrame(fromCall);
    aprsFrame.append("-5>GPS30,DSTAR*:")
             .append(nmea);

    data.assign(aprsFrame);
    return true;     
}

// When set on manual position Icom radios send 000000.00 as NMEA timestamps
// this is a dirty hack to correct this issue. Actually I am not sure about APRS
// software being peeky about this except APRS.fi
void CNMEASentenceCollector::fixUpNMEATimeStamp(std::string& nmea)
{
    auto posStar = nmea.find_last_of('*');
    if(posStar != std::string::npos)
        nmea = nmea.substr(0, posStar);

    time_t now;
	::time(&now);
	struct tm* tm = ::gmtime(&now);
    auto timeStamp = CStringUtils::string_format(",%02d%02d%02d.00,", tm->tm_hour, tm->tm_min, tm->tm_sec);

    boost::replace_all(nmea, ",000000.00,", timeStamp);
    //recalculate checksum
    nmea = CStringUtils::string_format("%s*%02x", nmea.c_str(), calcXOR(nmea));
}
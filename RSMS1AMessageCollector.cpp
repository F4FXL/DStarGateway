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


#include <cstring>
#include <cassert>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <algorithm>
#include <cstdlib>

#include "RSMS1AMessageCollector.h"
#include "StringUtils.h"
#include "Log.h"
#include "Utils.h"
#include "APRSUtils.h"


const unsigned int APRS_CSUM_LENGTH = 4U;

CRSMS1AMessageCollector::CRSMS1AMessageCollector() :
CSentenceCollector(SLOW_DATA_TYPE_GPS, "$$Msg", '\x0D')
{

}

bool CRSMS1AMessageCollector::isValidSentence(const std::string& sentence)
{
    return isValidMsg(sentence);
}

bool CRSMS1AMessageCollector::isValidMsg(const std::string& msg)
{
    if(msg.empty() || !boost::starts_with(msg, "$$Msg"))
        return false;

    // CUtils::dump("RS-MS1A:", (unsigned char *)msg.c_str(), msg.length());
    std::vector<std::string> splits;
    boost::split(splits, msg, boost::is_any_of(","));

    bool ret = splits.size() >= 4
                && !splits[1].empty()
                && !splits[2].empty();
    return ret;

    //TODO 2022-01-01 figure out what the heck it is about thic strange CRCs

    // CUtils::dump("RS-MS1A:", (unsigned char *)gpsa.c_str(), gpsa.length() + 1U);
    // CLog::logDebug("RS-MS1A: %s", gpsa.c_str());

    // auto thirdCommaPos = CStringUtils::find_nth(gpsa, 0U, ',', 3);
    // auto csum = calcCRC(gpsa, thirdCommaPos + 6 + 1, gpsa.length() - thirdCommaPos - 2U - 6U);
    // auto csumStr = CStringUtils::string_format("%06X", csum);
    // CLog::logDebug("RS-MS1A CRC: %s", csumStr.c_str());

    // auto expectedCsum = gpsa.substr(5U, APRS_CSUM_LENGTH);
    // bool res = ::strcasecmp(csumStr.c_str(), expectedCsum.c_str()) == 0;
    // return res;
}

unsigned int CRSMS1AMessageCollector::calcCRC(const std::string& gpsa, unsigned int start, unsigned int length)
{
	unsigned int icomcrc = 0xFFFFU;
    auto end = start + length;
    if(end > gpsa.length()) {
        end = gpsa.length();
    }

	for (unsigned int j = start; j < end; j++) {
		unsigned char ch = (unsigned char)gpsa[j];

		for (unsigned int i = 0U; i < 8U; i++) {
			bool xorflag = (((icomcrc ^ ch) & 0x01U) == 0x01U);

			icomcrc >>= 1;

			if (xorflag)
				icomcrc ^= 0x8408U;

			ch >>= 1;
		} 
	}

	return ~icomcrc & 0xFFFFU;
}

unsigned int CRSMS1AMessageCollector::getDataInt(unsigned char * data, unsigned int length)
{
    if(data == nullptr || length == 0U || getSentence().empty())
        return 0U;

    auto sentence = getSentence();

    std::vector<std::string> splits;
    boost::split(splits, sentence, boost::is_any_of(","));

    bool ret = splits.size() >= 4
                && !splits[1].empty()
                && !splits[2].empty();
    if(!ret) {
        return 0U;
    }

    auto sender = splits[1];
    auto recipient = CUtils::ToUpper(splits[2]);

    for(unsigned int i = 0;i < 3; i++) {
        splits.erase(splits.begin());
    }

    auto message = boost::join(splits, ",");
    if(message.length() > 6)
        message = message.substr(6);

    auto seqNum = rand() % 0xFFFFFU;

    CAPRSUtils::dstarCallsignToAPRS(sender);
    CAPRSUtils::dstarCallsignToAPRS(recipient);
    recipient.resize(9, ' ');
    message.resize(std::max<int>(0 , ((int)message.length()) - 2));

    //unescape commas in message body
    boost::replace_all(message, "o,", ",");

    auto aprsFrame = CStringUtils::string_format("%s-5>APDPRS,DSTAR*::%s:%s{%05X\r\n", sender.c_str(), recipient.c_str(), message.c_str(), seqNum);
    
    auto aprsFrameLen = aprsFrame.length();

    if(length < aprsFrameLen) {
        CLog::logDebug("Not enough space to copy GPS-A APRS frame");
        return 0U;
    }

    for(unsigned int i = 0U; i < aprsFrameLen; i++){
        data[i] = aprsFrame[i];
    }

    return aprsFrameLen;
}
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
#include "RSMS1AMessageBuilder.h"


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
    CLog::logDebug("RS-MS1A");
    // checking validity involves parsing, so we do minumum checks here. Big chekc done in getDataInt
    return !msg.empty() && boost::starts_with(msg, "$$Msg");
}

unsigned int CRSMS1AMessageCollector::getDataInt(unsigned char * data, unsigned int length)
{
    if(data == nullptr || length == 0U)
        return 0U;

    std::string aprsFrame;
    if(!getDataInt(aprsFrame))
        return 0U;
    
    auto aprsFrameLen = aprsFrame.length();

    if(length < aprsFrameLen) {
        CLog::logDebug("Not enough space to copy RSMS1A message frame");
        return 0U;
    }

    for(unsigned int i = 0U; i < aprsFrameLen; i++){
        data[i] = aprsFrame[i];
    }

    return aprsFrameLen;
}

bool CRSMS1AMessageCollector::getDataInt(std::string& data)
{
    if(getSentence().empty())
        return false;

    std::string sender, recipient, body, sentence;
    sentence = getSentence();
    auto parseRes = CRSMS1AMessageBuilder::parseMessage(sender, recipient, body, sentence);
    
    CUtils::dump(CStringUtils::string_format("RS-MS1A Message parsed with result %s", parseRes == RSMS_OK ? "OK" : (parseRes == RSMS_ERRONEOUS_MSG ? "Error in msg body" : "failed")).c_str(),
                                                (unsigned char *)sentence.c_str(), sentence.length());

    if(parseRes == RSMS_FAIL)
        return false;

    CAPRSUtils::dstarCallsignToAPRS(sender);
    CAPRSUtils::dstarCallsignToAPRS(recipient);
    recipient.resize(9U, ' ');
    
    auto seqNum = rand() % 0xFFFFFU;
    CStringUtils::string_format_in_place(data, "%s-5>APDPRS,DSTAR*::%s:%s{%05X", sender.c_str(), recipient.c_str(), body.c_str(), seqNum);
    
    return true;
}
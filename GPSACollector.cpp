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

#include "GPSACollector.h"
#include "StringUtils.h"
#include "Log.h"

const unsigned int APRS_CSUM_LENGTH = 4U;

CGPSACollector::CGPSACollector() :
CSentenceCollector(SLOW_DATA_TYPE_GPS, "$$CRC", '\x0D')
{

}

bool CGPSACollector::isValidSentence(const std::string& sentence)
{
    return isValidGPSA(sentence);
}

bool CGPSACollector::isValidGPSA(const std::string& gpsa)
{
    if(gpsa.length() < 10U || !boost::starts_with(gpsa, "$$CRC"))
        return false;

    auto csum = calcCRC(gpsa);
    auto csumStr = CStringUtils::string_format("%04X", csum);
    auto expectedCsum = gpsa.substr(5U, APRS_CSUM_LENGTH);
    bool res = ::strcasecmp(csumStr.c_str(), expectedCsum.c_str()) == 0;
    return res;
}

unsigned int CGPSACollector::calcCRC(const std::string& gpsa)
{
	unsigned int icomcrc = 0xFFFFU;

    auto length = gpsa.length();
	for (unsigned int j = 10U; j < length; j++) {
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

unsigned int CGPSACollector::getDataInt(unsigned char * data, unsigned int length)
{
    if(data == nullptr || length == 0U || getSentence().empty())
        return 0U;

    auto aprsFrame = getSentence();
    if(aprsFrame.length() < 11U)
        return 0U;

    aprsFrame = aprsFrame.substr(10).append("\r\n");
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
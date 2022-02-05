/*
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

#include <boost/algorithm/string.hpp>

#include "APRSUtils.h"

void CAPRSUtils::dstarCallsignToAPRS(std::string& dstarCallsign)
{
	if(dstarCallsign[dstarCallsign.length() - 1] == ' ') {
		boost::trim(dstarCallsign);
	} else {
		//loop until got rid of all double blanks
		while(dstarCallsign.find("  ") != std::string::npos) {
			boost::replace_all(dstarCallsign, "  ", " ");
		}
		boost::replace_all(dstarCallsign, " ", "-");//replace remaining blank with a -
	}
}

unsigned int CAPRSUtils::calcGPSAIcomCRC(const std::string& gpsa)
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

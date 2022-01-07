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

#include "RSMS1AMessageBuilder.h"
#include "StringUtils.h"

std::vector<char> CRSMS1AMessageBuilder::m_charsToEscape = {-17, 0, 17, 19, -2, -25, 26, -3, -1, 36, 13, 44};

void CRSMS1AMessageBuilder::buildMessage(std::string& message, const std::string& sender, const std::string& recipient, const std::string body)
{
    auto bodyCrc = calculateBodyCRC(body);
    std::string bodyTmp;
    escapeBody(bodyTmp, body + bodyCrc);
    
    std::string header = CStringUtils::string_format("%s,%s,0011", sender.c_str(), recipient.c_str());
    char c1, c2;
    calcMsgIcomCRC(header, c1, c2);
    header.push_back(c1);
    header.push_back(c2);
    message = "$$Msg," + header + bodyTmp + '\n';
}   

char CRSMS1AMessageBuilder::calculateBodyCRC(const std::string& body)
{
    if(body.length() == 1)
        return body[0];

    int num = 0;
    for(auto c : body) {
        num += c;
    }

    return (char)((num & 255) - 128);
}

void CRSMS1AMessageBuilder::escapeBody(std::string& output, const std::string& body)
{
    output.clear();
    for(char c : body) {
        if(std::find(m_charsToEscape.begin(), m_charsToEscape.end(), c) != m_charsToEscape.end()) {
            output.push_back('o');
        }
        output.push_back(c);
    }
}

void CRSMS1AMessageBuilder::calcMsgIcomCRC(const std::string& msg, char& c1, char& c2)
{
	int num = 0;
	for(unsigned int i = 0U; i < msg.length(); i++) {
		num += msg[i];
	}

	c1 = doWhatever((char)((num >> 4) & 15));
	c2 = doWhatever((char)(num & 15));
}

char CRSMS1AMessageBuilder::doWhatever(char b2) {
    int i;
    int i2 = b2 & 255;
    if (i2 >= 0 && i2 <= 9) {
        i = b2 + 48;
    } else if (10 > i2 || i2 > 15) {
        return 0;
    } else {
        i = b2 + 55;
    }
    return (char) i;
}
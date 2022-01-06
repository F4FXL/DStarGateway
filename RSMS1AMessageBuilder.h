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

#pragma once

#include <string>
#include <vector>

class CRSMS1AMessageBuilder
{
public:
    static void buildMessage(std::string& message, const std::string& sender, const std::string& recipient, const std::string body);

private:
    static void calcMsgIcomCRC(const std::string& msg, char& c1, char& c2);
    static void escapeBody(std::string output, const std::string& body);
    static void escapeBytes(std::vector<char> output, const std::vector<char> input);
    static char calculateBodyCRC(const std::string& body);
    static char doWhatever(char b2);

    static std::vector<char> m_charsToEscape;
};
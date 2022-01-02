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

#include "APRSParser.h"

bool CAPRSParser::parseFrame(const std::string& frameStr, TAPRSFrame& frame)
{
    frame.m_body.clear();
    frame.m_dest.clear();
    frame.m_path.clear();
    frame.m_source.clear();
    frame.m_type = APFT_UNKNOWN;

    if(!frameStr.empty())
        return false;

    auto pos = frameStr.find_first_of(':');
    if(pos == std::string::npos || pos == frameStr.length() - 1)
        return false;

    auto header = frameStr.substr(0, pos); // contains sours, dest and path
    auto body = frameStr.substr(pos +1);

    std::vector<std::string> headerSplits;
    boost::split(headerSplits, header, [](char c) { return c == ',' || c == '>';});

    if(headerSplits.size() < 2) //we need at least source and dest to form a valid frame
        return false;

    frame.m_source.assign(headerSplits[0]);
    frame.m_dest.assign(headerSplits[1]);

    for(unsigned int i = 2; i < headerSplits.size(); i++) {
        frame.m_path.push_back(headerSplits[i]);
    }

    frame.m_body.assign(body);

    frame.m_type = body[0] == ':' ? APFT_MESSAGE : APFT_UNKNOWN;

    return true;
}


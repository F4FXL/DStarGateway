/*
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "StringUtils.h"

size_t CStringUtils::find_nth(const std::string& haystack, char needle, size_t nth) 
{
    size_t matches = 0U;
    auto haystackLength = haystack.length();

    for(size_t i = 0; i < haystackLength; i++) {
        if(haystack[i] == needle) {
            matches++;
            if(matches == nth)
                return i;
        }
    }

    return std::string::npos;
}

unsigned int CStringUtils::stringToPort(const std::string& s)
{
    unsigned int port = 0U;
    std::string ls = boost::trim_copy(s);

    if(!ls.empty() && std::all_of(ls.begin(), ls.end(), [](char c){ return c >= '0' && c <= '9'; })) {
        auto portTemp = boost::lexical_cast<unsigned int>(ls);
        if(portTemp > 0U && portTemp <= 65535U)
            port = portTemp;
    }

    return port;
}
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

#include "StringUtils.h"

size_t CStringUtils::find_nth(const std::string& haystack, size_t pos, char needle, size_t nth) 
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
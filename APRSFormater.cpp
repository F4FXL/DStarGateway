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

#include "APRSFormater.h"
#include "Log.h"
#include "StringUtils.h"

bool CAPRSFormater::frameToString(std::string& output, CAPRSFrame& frame)
{
    // make sur we have the minimal stuff to build a correct aprs string
    if(frame.getSource().empty()
        || frame.getDestination().empty()
        || frame.getBody().empty()) {
            CLog::logWarning("Invalid APRS frame, missing source, destination or body");
            return false;
    }

    // std::string path(frame.getPath().size() > 0U ? "," :  "");
    // for(auto pathItem : frame.getPath()) {
    //     path.append(boost::trim_copy(pathItem)).push_back(',');
    // }

    // boost::trim_right_if(path, [](char c){ return c == ','; });

    auto path = boost::join_if(frame.getPath(), ",", [](std::string s) { return  !string_is_blank_or_empty(s); });

    CStringUtils::string_format_in_place(output, "%s>%s%s%s:%s",
                                                    frame.getSource().c_str(),
                                                    frame.getDestination().c_str(),
                                                    path.empty() ? "" : ",",
                                                    path.c_str(),
                                                    frame.getBody().c_str());

    return true;
}


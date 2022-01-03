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
#include "Log.h"

bool CAPRSParser::parseFrame(const std::string& frameStr, CAPRSFrame& frame)
{
    frame.clear();
    bool ret = false;

    if(!frameStr.empty()) {
        auto pos = frameStr.find_first_of(':');
        if(pos != std::string::npos && pos != frameStr.length() - 1) {
            auto header = frameStr.substr(0, pos); // contains source, dest and path
            auto body = frameStr.substr(pos +1);

            std::vector<std::string> headerSplits;
            boost::split(headerSplits, header, [](char c) { return c == ',' || c == '>';});

             //we need at least source and dest to form a valid frame, also headers shall not contain empty strings
            if(headerSplits.size() >= 2 && std::none_of(headerSplits.begin(), headerSplits.end(), [](std::string s){ return s.empty(); })) {
                frame.getSource().assign(headerSplits[0]);
                frame.getDestination().assign(headerSplits[1]);

                for(unsigned int i = 2; i < headerSplits.size(); i++) {
                    frame.getPath().push_back(headerSplits[i]);
                }

                frame.getBody().assign(body);

                setFrameType(frame);
                if(frame.getType() == APFT_UNKNOWN) {
                    CLog::logInfo("Invalid or unsupported APRS frame : %s", frameStr);
                }
                else {
                    ret = true;
                }
            }
        }
    }

    return ret;
}

void CAPRSParser::setFrameType(CAPRSFrame& frame)
{
    APRS_FRAME_TYPE type = APFT_UNKNOWN;
    std::string body(frame.getBody());

    if(!body.empty()) {
        switch (body[0])
        {
            case ':':
                if(body[10] == ':' && std::all_of(body.begin() + 1, body.begin() + 10,
                                                [](char c){ return c == ' ' || c == '-' || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'); }))
                    type = APFT_MESSAGE;
                break;
            
            default:
                break;
        }
    }

    frame.getType() = type;
    if(type == APFT_UNKNOWN)
        frame.clear();
}
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

                ret = parseInt(frame);
                if(!ret) {
                    frame.clear();
                }
            }
        }
    }

    return ret;
}

bool CAPRSParser::parseInt(CAPRSFrame& frame)
{
    APRS_FRAME_TYPE type = APFT_UNKNOWN;
    unsigned char typeChar = frame.getBody()[0];
    std::string body(frame.getBody().substr(1));//strip the type char for processing purposes
    
    if(body.empty())
        return false;

    switch (typeChar)
    {
        case '!':
            if(body[0] == '!') {
                // This is ultimeter 200 weather station
                return false;
            }
            [[fallthrough]];
        case '=':
        case '/':
        case '@':
            {
                if(body.length() < 10) return false;//enough chars to have a chance to parse it ?
                /* Normal or compressed location packet, with or without
                * timestamp, with or without messaging capability
                *
                * ! and / have messaging, / and @ have a prepended timestamp
                */
                type = APFT_POSITION;
                if(typeChar == '/' || typeChar== '@')//With a prepended timestamp, jump over it. 
                    body = body.substr(7U);

                auto posChar = body[0];
                if(valid_sym_table_compressed(posChar)//Compressed format
                    && body.length() >= 13){//we need at least 13 char
                    //icom unsupported, ignore for now
                    return false;//parse_aprs_compressed(pb, body, body_end);
                }
                else if(posChar >= '0' && posChar <= '9' //Normal uncompressed format
                        && body.length() >=19){//we need at least 19 chars for it to be valid
                    
                    // if(ensureIsIcomCompatible(packet))
                    //     return Parse(packet.Raw(), packet);
                }
            }
            break;
        case '$' :
            if(body.length() > 10) {
                type = APFT_NMEA;
            }
            break;
        case ':':
            if(body[9] == ':' && std::all_of(body.begin(), body.begin() + 9,
                                            [](char c){ return c == ' ' || c == '-' || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'); }))
                type = APFT_MESSAGE;
            break;
        case '>':
            type = APFT_STATUS;
            break;
        case '#': /* Peet Bros U-II Weather Station */
        case '*': /* Peet Bros U-I  Weather Station */
        case '_': /* Weather report without position */
            type = APFT_WX;
            break;
        case '{':
            type = APFT_UNKNOWN; //
            break;
        default:
            type = APFT_UNKNOWN;
            break;
    }
    
    frame.getType() = type;
    return type != APFT_UNKNOWN;
}

bool CAPRSParser::valid_sym_table_compressed(unsigned char c)
{
	return (c == '/' || c == '\\' || (c >= 0x41 && c <= 0x5A)
		    || (c >= 0x61 && c <= 0x6A)); /* [\/\\A-Za-j] */
}

bool CAPRSParser::valid_sym_table_uncompressed(unsigned char  c)
{
	return (c == '/' || c == '\\' || (c >= 0x41 && c <= 0x5A)
		    || (c >= 0x30 && c <= 0x39)); /* [\/\\A-Z0-9] */
}
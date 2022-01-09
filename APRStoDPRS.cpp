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

#include "APRStoDPRS.h"
#include "Log.h"
#include "RSMS1AMessageBuilder.h"

bool CAPRSToDPRS::aprsToDPRS(std::string& dprs, CHeaderData& header, CAPRSFrame& frame)
{
    dprs.clear();
    switch (frame.getType())
    {
        case APFT_MESSAGE :
            return messageToDPRS(dprs, header, frame);
        default:
            break;
    }

    return false;
}

bool CAPRSToDPRS::messageToDPRS(std::string& dprs, CHeaderData& header, CAPRSFrame& frame)
{
    auto frameBody = frame.getBody();
    if(frameBody.length() < 11 || frameBody[0] != ':' || frameBody[10] != ':') {
        CLog::logDebug("Invalid APRS message body : %s", frameBody.c_str());
        return false;
    }

    // extract recipient
    auto recipient = boost::trim_copy(frameBody.substr(1, 9));
    if(recipient.length() == 0U) {
        CLog::logDebug("APRS message has no recipient");
        return false;
    }
    auto dashPos = recipient.find_first_of('-');
    if(dashPos != std::string::npos)
        recipient = recipient.substr(0, dashPos);


    auto messageBody = boost::trim_copy(frameBody.substr(11));

    header.setId(header.createId());
    header.setMyCall1(frame.getSource());
    header.setMyCall2("MSG");
    header.setYourCall(recipient);

    CRSMS1AMessageBuilder::buildMessage(dprs, frame.getSource(), recipient, messageBody);

    return true;
}
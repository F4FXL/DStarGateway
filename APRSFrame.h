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

// We only support these types for now
enum APRS_FRAME_TYPE {
    APFT_UNKNOWN = 0,
    APFT_MESSAGE,
    APFT_POSITION,
    APFT_NMEA,
    APFT_STATUS,
    APFT_OBJECT,
    APFT_WX
};

class CAPRSFrame {
public:
    CAPRSFrame();
    CAPRSFrame(const std::string& source, const std::string& destination, const std::vector<std::string>& path, const std::string& body, APRS_FRAME_TYPE type);
    ~CAPRSFrame();

    void clear();
    std::string& getSource() { return m_source; }
    std::string& getDestination() { return m_destination; }
    std::vector<std::string>& getPath() { return m_path; }
    std::string& getBody() { return m_body; }
    APRS_FRAME_TYPE& getType() { return m_type; }

private:
    std::string m_source;
    std::string m_destination;
    std::vector<std::string> m_path;
    std::string m_body;
    APRS_FRAME_TYPE m_type;
};
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

#include "APRSFrame.h"

CAPRSFrame::CAPRSFrame() :
m_source(),
m_destination(),
m_path(),
m_type(APFT_UNKNOWN)
{

}

CAPRSFrame::CAPRSFrame(const std::string& source, const std::string& destination, const std::vector<std::string>& path, APRS_FRAME_TYPE type) :
m_source(source),
m_destination(destination),
m_path(),
m_type(type)
{
    m_path.assign(path.begin(), path.end());
}

void CAPRSFrame::clear()
{
    m_source.clear();
    m_destination.clear();
    m_path.clear();
    m_type = APFT_UNKNOWN;
}
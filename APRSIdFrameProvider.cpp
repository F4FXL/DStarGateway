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

#include <cassert>

#include "APRSIdFrameProvider.h"

CAPRSIdFrameProvider::CAPRSIdFrameProvider(unsigned int timeout) :
m_timer(1000U)
{
    m_timer.start(timeout);
}

CAPRSIdFrameProvider::~CAPRSIdFrameProvider()
{

}

bool CAPRSIdFrameProvider::buildAPRSFrames(const std::string& gateway, const CAPRSEntry * entry, std::vector<std::string> & frames)
{
    assert(entry != nullptr);

    return buildAPRSFramesInt(gateway, entry, frames);
}

bool CAPRSIdFrameProvider::wantsToSend()
{
    if(m_timer.hasExpired())
    {
        m_timer.start();
        return true;
    }

    return false;
}

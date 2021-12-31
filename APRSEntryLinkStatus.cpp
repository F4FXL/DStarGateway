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

#include "APRSEntryLinkStatus.h"

CAPRSEntryLinkStatus::CAPRSEntryLinkStatus() :
m_linkStatus(LS_NONE),
m_linkDestination(),
m_linkStatusChanged(false),
m_timer(1000U)
{
    m_timer.setTimeout(20U * 60U); //statuses go out every 20 minutes or every change
}

bool CAPRSEntryLinkStatus::isOutOfDate()
{
    return m_linkStatusChanged || m_timer.hasExpired();
}

std::string CAPRSEntryLinkStatus::getLinkDestination() const
{
    return m_linkDestination;
}

LINK_STATUS CAPRSEntryLinkStatus::getLinkStatus() const
{
    return m_linkStatus;
}

void CAPRSEntryLinkStatus::setLink(LINK_STATUS linkstatus, const std::string& destination)
{
    bool changed = m_linkStatus != linkstatus || m_linkDestination != destination;
    if(changed) {
        m_linkStatus = linkstatus;
        m_linkDestination = destination;
    }
    m_linkStatusChanged = changed;
}

void CAPRSEntryLinkStatus::reset()
{
    m_linkStatusChanged = false;
    m_timer.start();
}

void CAPRSEntryLinkStatus::clock(unsigned int ms)
{
    m_timer.clock(ms);
}
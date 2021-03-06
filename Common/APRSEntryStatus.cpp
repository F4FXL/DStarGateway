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

#include "APRSEntryStatus.h"

CAPRSEntryStatus::CAPRSEntryStatus() :
m_status(),
m_statusChanged(false),
m_timer(1000U)
{
    m_timer.setTimeout(20U * 60U); //statuses go out every 20 minutes or every change
}

bool CAPRSEntryStatus::isOutOfDate()
{
    bool ouOfDate = m_statusChanged || m_timer.hasExpired();
    m_statusChanged = false;
    return ouOfDate;
}


std::string CAPRSEntryStatus::getStatus() const
{
    return m_status;
}

void CAPRSEntryStatus::setStatus(const std::string& linkstatus)
{
    bool changed = m_status != linkstatus;
    if(changed) {
        m_status = linkstatus;
    }
    m_statusChanged = changed;
}

void CAPRSEntryStatus::clock(unsigned int ms)
{
    m_timer.clock(ms);
}
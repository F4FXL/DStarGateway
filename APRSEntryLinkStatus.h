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

#pragma once

#include "Timer.h"
#include "Defs.h"

class CAPRSEntryLinkStatus
{
public:
	CAPRSEntryLinkStatus() ;

	LINK_STATUS getLinkStatus() const;
	std::string getLinkDestination() const;
	bool isOutOfDate();
	void setLink(LINK_STATUS linkstatus, const std::string& destination);
	void reset();
	void clock(unsigned int ms);

private:
	LINK_STATUS		m_linkStatus;
	std::string		m_linkDestination;
	bool 			m_linkStatusChanged;
	CTimer			m_timer;
};
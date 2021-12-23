/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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
#include <chrono>

#include "DVTOOLFileReader.h"
#include "RepeaterCallback.h"
#include "AMBEData.h"
#include "Timer.h"
#include "Defs.h"

enum ANNOUNCEMENT_STATUS {
	NS_IDLE,
	NS_WAIT,
	NS_TRANSMIT
};

class CAnnouncementUnit {
public:
	CAnnouncementUnit(IRepeaterCallback* handler, const std::string& callsign, const std::string& fileName, const std::string& name);
	~CAnnouncementUnit();

	void sendAnnouncement();

	void cancel();

	void clock(unsigned int ms);

private:
	IRepeaterCallback*  m_handler;
	std::string         m_callsign;
	std::string         m_fileName;
	std::string         m_name;
	CDVTOOLFileReader*  m_reader;
	ANNOUNCEMENT_STATUS m_status;
	CTimer              m_timer;
	unsigned int        m_out;
	unsigned int        m_id;
	std::chrono::high_resolution_clock::time_point m_start;
};

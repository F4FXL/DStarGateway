/*
 *   Copyright (C) 2011,2012,2013 by Jonathan Naylor G4KLX
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
#include <map>
#include <chrono>
#include <vector>

#include "RepeaterCallback.h"
#include "SlowDataEncoder.h"
#include "AMBEData.h"
#include "Timer.h"
#include "Defs.h"
#include "AMBEFileReader.h"

enum AUDIO_STATUS {
	AS_IDLE,
	AS_WAIT,
	AS_TRANSMIT
};

class CAudioUnit {
public:
	CAudioUnit(IRepeaterCallback* handler, const std::string& callsign);
	~CAudioUnit();

	void sendStatus();

	void setStatus(LINK_STATUS status, const std::string& reflector, const std::string& text);
	void setTempStatus(LINK_STATUS status, const std::string& reflector, const std::string& text);

	void cancel();

	void clock(unsigned int ms);

	static void initialise();

	static void setLanguage(const std::string & dir, TEXT_LANG language);

	static void finalise();

private:
	static TEXT_LANG      m_language;

	IRepeaterCallback* m_handler;
	std::string        m_callsign;
	AUDIO_STATUS       m_status;
	LINK_STATUS        m_linkStatus;
	LINK_STATUS        m_tempLinkStatus;
	std::string        m_text;
	std::string        m_tempText;
	std::string        m_reflector;
	std::string        m_tempReflector;
	bool               m_hasTemporary;
	CTimer             m_timer;
	std::vector<CAMBEData*>   m_data;
	static CAMBEFileReader*   m_ambeFilereader;
	unsigned int       m_out;
	std::chrono::high_resolution_clock::time_point m_time;

	void spellReflector(const std::string& reflector);
	void sendStatus(LINK_STATUS status, const std::string& reflector, const std::string& text);
};


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

#include "RepeaterCallback.h"
#include "SlowDataEncoder.h"
#include "AMBEData.h"
#include "Timer.h"
#include "Defs.h"

class CIndexRecord {
public:
	CIndexRecord(const std::string& name, unsigned int start, unsigned int length) :
	m_name(name),
	m_start(start),
	m_length(length)
	{
	}

	std::string getName() const
	{
		return m_name;
	}

	unsigned int getStart() const
	{
		return m_start;
	}

	unsigned int getLength() const
	{
		return m_length;
	}

private:
	std::string  m_name;
	unsigned int m_start;
	unsigned int m_length;
};

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

	static void setLanguage(TEXT_LANG language);

	static void finalise();

private:
	static std::map<std::string, CIndexRecord *> m_index;
	static unsigned char* m_ambe;
	static unsigned int   m_ambeLength;
	static TEXT_LANG      m_language;

	IRepeaterCallback* m_handler;
	std::string        m_callsign;
	CSlowDataEncoder   m_encoder;
	AUDIO_STATUS       m_status;
	LINK_STATUS        m_linkStatus;
	LINK_STATUS        m_tempLinkStatus;
	std::string        m_text;
	std::string        m_tempText;
	std::string        m_reflector;
	std::string        m_tempReflector;
	bool               m_hasTemporary;
	CTimer             m_timer;
	CAMBEData**        m_data;
	unsigned int       m_in;
	unsigned int       m_out;
	unsigned int       m_seqNo;
	std::chrono::high_resolution_clock::time_point m_time;

	bool lookup(unsigned int id, const std::string& name);
	void spellReflector(unsigned int id, const std::string& reflector);
	void sendStatus(LINK_STATUS status, const std::string& reflector, const std::string& text);

	static bool readAMBE(const std::string& name);
	static bool readIndex(const std::string& name);
};


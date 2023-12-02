/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
 *	 Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <sys/stat.h>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <chrono>

#include "TimeServerThread.h"
#include "DStarDefines.h"
#include "Utils.h"
#include "NetUtils.h"
#include "StringUtils.h"
#include "Log.h"

const unsigned int MAX_FRAMES = 60U * DSTAR_FRAMES_PER_SEC;

const unsigned int SILENCE_LENGTH = 10U;

CTimeServerThread::CTimeServerThread() :
CThread("Time Server"),
m_callsign(),
m_repeaters(),
m_callsignG(),
m_address(),
m_addressStr(),
m_language(LANG_ENGLISH_UK_1),
m_format(FORMAT_VOICE_TIME),
m_interval(INTERVAL_15MINS),
m_data(),
m_killed(false),
m_dataPath(""),
m_ambeFileReader(nullptr)
{
	CHeaderData::initialise();
	m_address.s_addr = INADDR_NONE; 
}

CTimeServerThread::~CTimeServerThread()
{
	for(auto d : m_data)
		delete d;

	m_data.clear();

	delete[] m_ambeFileReader;
}

void * CTimeServerThread::Entry()
{
	// Wait here until we have the essentials to run
	while (!m_killed && m_address.s_addr == INADDR_NONE && m_repeaters.size() == 0U)
		Sleep(500UL);		// 1/2 sec

	if (m_killed)
		return nullptr;

	if (m_format != FORMAT_TEXT_TIME) {
		bool ret = loadAMBE();
		if (!ret) {
			CLog::logWarning(("Cannot load the AMBE data, using text only time"));
			m_format = FORMAT_TEXT_TIME;
		}
	}

	CLog::logInfo(("Starting the Time Server thread"));

	unsigned int lastMin = 0U;

	while (!m_killed) {
		time_t now;
		::time(&now);

		struct tm* tm = ::localtime(&now);

		unsigned int hour = tm->tm_hour;
		unsigned int  min = tm->tm_min;

		// if (min != lastMin)
		// 	sendTime(15, 45);

		if (min != lastMin) {
			if (m_interval == INTERVAL_15MINS && (min == 0U || min == 15U || min == 30U || min == 45U))
				sendTime(hour, min);
			else if (m_interval == INTERVAL_30MINS && (min == 0U || min == 30U))
				sendTime(hour, min);
			else if (m_interval == INTERVAL_60MINS && min == 0U)
				sendTime(hour, min);
		}

		lastMin = min;

		Sleep(450UL);
	}

	CLog::logInfo(("Stopping the Time Server thread"));

	return nullptr;
}

void CTimeServerThread::kill()
{
	m_killed = true;
}

bool CTimeServerThread::setGateway(const std::string& callsign, const std::string& rpt1, const std::string& rpt2, const std::string& rpt3, const std::string& rpt4, const std::string& address, const std::string& dataPath)
{
	m_callsign = callsign;
	m_callsign.resize(LONG_CALLSIGN_LENGTH - 1U, (' '));

	m_callsignG = m_callsign;
	m_callsignG.push_back('G');

	if (!rpt1.empty()) {
		m_repeaters.push_back(m_callsign + rpt1);
	}

	if (!rpt2.empty()) {
		m_repeaters.push_back(m_callsign + rpt2);
	}

	if (!rpt3.empty()) {
		m_repeaters.push_back(m_callsign + rpt3);
	}

	if (!rpt4.empty()) {
		m_repeaters.push_back(m_callsign + rpt4);
	}

	m_callsign.push_back(' ');

	m_addressStr.assign(address);
	m_dataPath.assign(dataPath);
	m_address = CUDPReaderWriter::lookup(address);

	return true;
}

void CTimeServerThread::setAnnouncements(LANGUAGE language, FORMAT format, INTERVAL interval)
{
	m_language = language;
	m_format   = format;
	m_interval = interval;
}

void CTimeServerThread::sendTime(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	switch (m_language) {
		case LANG_ENGLISH_UK_1:
			words = sendTimeEnGB1(hour, min);
			break;
		case LANG_ENGLISH_UK_2:
			words = sendTimeEnGB2(hour, min);
			break;
		case LANG_ENGLISH_US_1:
			words = sendTimeEnUS1(hour, min);
			break;
		case LANG_ENGLISH_US_2:
			words = sendTimeEnUS2(hour, min);
			break;
		case LANG_DEUTSCH_1:
			words = sendTimeDeDE1(hour, min);
			break;
		case LANG_DEUTSCH_2:
			words = sendTimeDeDE2(hour, min);
			break;
		case LANG_FRANCAIS:
			words = sendTimeFrFR(hour, min);
			break;
		case LANG_NEDERLANDS:
			words = sendTimeNlNL(hour, min);
			break;
		case LANG_SVENSKA:
			words = sendTimeSeSE(hour, min);
			break;
		case LANG_ESPANOL:
			words = sendTimeEsES(hour, min);
			break;
		case LANG_NORSK:
			words = sendTimeNoNO(hour, min);
			break;
		case LANG_PORTUGUES:
			words = sendTimePtPT(hour, min);
			break;
		default:
			break;
	}

	send(words, hour, min);
}

std::vector<std::string> CTimeServerThread::sendTimeEnGB1(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("It_is"));

	switch (hour) {
		case 0U:
		case 12U:
			words.push_back(("twelve"));
			break;
		case 1U:
		case 13U:
			words.push_back(("one"));
			break;
		case 2U:
		case 14U:
			words.push_back(("two"));
			break;
		case 3U:
		case 15U:
			words.push_back(("three"));
			break;
		case 4U:
		case 16U:
			words.push_back(("four"));
			break;
		case 5U:
		case 17U:
			words.push_back(("five"));
			break;
		case 6U:
		case 18U:
			words.push_back(("six"));
			break;
		case 7U:
		case 19U:
			words.push_back(("seven"));
			break;
		case 8U:
		case 20U:
			words.push_back(("eight"));
			break;
		case 9U:
		case 21U:
			words.push_back(("nine"));
			break;
		case 10U:
		case 22U:
			words.push_back(("ten"));
			break;
		case 11U:
		case 23U:
			words.push_back(("eleven"));
			break;
		default:
			break;
	}
	
	switch (min) {
		case 15U:
			words.push_back(("fifteen"));
			break;
		case 30U:
			words.push_back(("thirty"));
			break;
		case 45U:
			words.push_back(("forty-five"));
			break;
		default:
			break;
	}

	if (hour >= 12U)
		words.push_back(("PM"));
	else
		words.push_back(("AM"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeEnGB2(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back("It_is");

	if (min == 15U) {
		words.push_back(("a_quarter_past"));
	} else if (min == 30U) {
		words.push_back(("half_past"));
	} else if (min == 45U) {
		words.push_back(("a_quarter_to"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	if (hour == 0U && min == 0U) {
		words.push_back(("midnight"));
	} else if (hour == 12U && min == 0U) {
		words.push_back(("twelve"));
		words.push_back(("noon"));
	} else if (hour == 0U || hour == 12U) {
		words.push_back(("twelve"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("one"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("two"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("three"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("four"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("five"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("six"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("seven"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("eight"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("nine"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("ten"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("eleven"));
	}

	if (hour != 0U && hour != 12U && min == 0U)
		words.push_back(("O_Clock"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeEnUS1(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("It_is"));

	switch (hour) {
		case 0U:
		case 12U:
			words.push_back(("twelve"));
			break;
		case 1U:
		case 13U:
			words.push_back(("one"));
			break;
		case 2U:
		case 14U:
			words.push_back(("two"));
			break;
		case 3U:
		case 15U:
			words.push_back(("three"));
			break;
		case 4U:
		case 16U:
			words.push_back(("four"));
			break;
		case 5U:
		case 17U:
			words.push_back(("five"));
			break;
		case 6U:
		case 18U:
			words.push_back(("six"));
			break;
		case 7U:
		case 19U:
			words.push_back(("seven"));
			break;
		case 8U:
		case 20U:
			words.push_back(("eight"));
			break;
		case 9U:
		case 21U:
			words.push_back(("nine"));
			break;
		case 10U:
		case 22U:
			words.push_back(("ten"));
			break;
		case 11U:
		case 23U:
			words.push_back(("eleven"));
			break;
		default:
			break;
	}
	
	switch (min) {
		case 15U:
			words.push_back(("fifteen"));
			break;
		case 30U:
			words.push_back(("thirty"));
			break;
		case 45U:
			words.push_back(("forty-five"));
			break;
		default:
			break;
	}

	if (hour >= 12U)
		words.push_back(("PM"));
	else
		words.push_back(("AM"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeEnUS2(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("It_is"));

	if (min == 15U) {
		words.push_back(("a_quarter_past"));
	} else if (min == 30U) {
		words.push_back(("half_past"));
	} else if (min == 45U) {
		words.push_back(("a_quarter_to"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	if (hour == 0U && min == 0U) {
		words.push_back(("midnight"));
	} else if (hour == 12U && min == 0U) {
		words.push_back(("twelve"));
		words.push_back(("noon"));
	} else if (hour == 0U || hour == 12U) {
		words.push_back(("twelve"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("one"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("two"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("three"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("four"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("five"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("six"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("seven"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("eight"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("nine"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("ten"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("eleven"));
	}

	if (hour != 0U && hour != 12U && min == 0U)
		words.push_back(("O_Clock"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeDeDE1(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("Es_ist"));

	switch (hour) {
		case 0U:  words.push_back(("null"));  break;
		case 1U:  words.push_back(("ein"));  break;
		case 2U:  words.push_back(("zwei"));  break;
		case 3U:  words.push_back(("drei"));  break;
		case 4U:  words.push_back(("vier"));  break;
		case 5U:  words.push_back(("fuenf"));  break;
		case 6U:  words.push_back(("sechs"));  break;
		case 7U:  words.push_back(("sieben"));  break;
		case 8U:  words.push_back(("acht"));  break;
		case 9U:  words.push_back(("neun"));  break;
		case 10U: words.push_back(("zehn")); break;
		case 11U: words.push_back(("elf")); break;
		case 12U: words.push_back(("zwoelf")); break;
		case 13U: words.push_back(("dreizehn")); break;
		case 14U: words.push_back(("vierzehn")); break;
		case 15U: words.push_back(("fuenfzehn")); break;
		case 16U: words.push_back(("sechzehn")); break;
		case 17U: words.push_back(("siebzehn")); break;
		case 18U: words.push_back(("achtzehn")); break;
		case 19U: words.push_back(("neunzehn")); break;
		case 20U: words.push_back(("zwanzig")); break;
		case 21U: words.push_back(("einundzwanzig")); break;
		case 22U: words.push_back(("zweiundzwanzig")); break;
		case 23U: words.push_back(("dreiundzwanzig")); break;
		default: break;
	}

	words.push_back(("Uhr"));

	switch (min) {
		case 15U:
			words.push_back(("fuenfzehn"));
			break;
		case 30U:
			words.push_back(("dreissig"));
			break;
		case 45U:
			words.push_back(("fuenfundvierzig"));
			break;
		default:
			break;
	}

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeDeDE2(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("Es_ist"));

	if (min == 15U) {
		words.push_back(("viertel_nach"));
	} else if (min == 30U) {
		words.push_back(("halb"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	} else if (min == 45U) {
		words.push_back(("viertel_vor"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	if (hour == 0U) {
		words.push_back(("null"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("ein"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("zwei"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("drei"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("vier"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("fuenf"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("sechs"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("sieben"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("acht"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("neun"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("zehn"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("elf"));
	} else if (hour == 12U) {
		words.push_back(("zwoelf"));
	}

	if (min == 0U)
		words.push_back(("Uhr"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeFrFR(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	// if (hour > 17U)
	//	words.push_back(("bonsoir"));
	// else
	//	words.push_back(("bonjour"));

	words.push_back(("il_est"));

	if (min == 45U)
		hour++;

	if (hour == 0U) {
		words.push_back(("minuit"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("une"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("deux"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("trois"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("quatre"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("cinq"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("six"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("sept"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("huit"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("neuf"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("dix"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("onze"));
	} else if (hour == 12U) {
		words.push_back(("midi"));
	}

	if (hour == 1U || hour == 13U)
		words.push_back(("heure"));
	else if (hour != 12U && hour != 0U)
		words.push_back(("heures"));

	if (min == 15U) {
		words.push_back(("et_quart"));
	} else if (min == 30U) {
		words.push_back(("et_demie"));
	} else if (min == 45U) {
		words.push_back(("moins_le_quart"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeNlNL(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("Het_is"));

	if (min == 15U) {
		words.push_back(("kwart_over"));
	} else if (min == 30U) {
		words.push_back(("half"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	} else if (min == 45U) {
		words.push_back(("kwart_voor"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	if (hour == 0U || hour == 12U) {
		words.push_back(("twaalf"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("een"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("twee"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("drie"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("vier"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("vijf"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("zes"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("zeven"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("acht"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("negen"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("tien"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("elf"));
	}

	if (min == 0U)
		words.push_back(("uur"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeSeSE(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("Klockan_ar"));

	if (min == 15U) {
		words.push_back(("kvart_over"));
	} else if (min == 30U) {
		words.push_back(("halv"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	} else if (min == 45U) {
		words.push_back(("kvart_i"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	if (hour == 0U || hour == 12U) {
		words.push_back(("tolv"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("ett"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("tva"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("tre"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("fyra"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("fem"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("sex"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("sju"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("atta"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("nio"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("tio"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("elva"));
	}

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeEsES(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	if (min == 45U) {
		hour++;
		if (hour == 24U)
			hour = 0U;
	}

	if (hour == 1U)
		words.push_back(("Es_la"));
	else if (hour == 0U || hour == 12U)
		words.push_back(("Es"));
	else
		words.push_back(("Son_las"));

	if (hour == 0U) {
		words.push_back(("medianoche"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("una"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("dos"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("tres"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("cuarto"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("cinco"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("seis"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("siete"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("ocho"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("nueve"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("diez"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("once"));
	} else {
		words.push_back(("mediodia"));
	}

	if (min == 15U)
		words.push_back(("y_cuarto"));
	else if (min == 30U)
		words.push_back(("y_media"));
	else if (min == 45U)
		words.push_back(("menos_cuarto"));

	if (hour > 0U && hour < 12U)
		words.push_back(("de_la_manana"));
	else if (hour > 12U && hour < 19U)
		words.push_back(("de_la_tarde"));
	else if (hour >= 19U && hour <= 23U)
		words.push_back(("de_la_noche"));

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimeNoNO(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	words.push_back(("Klokken_er"));

	if (min == 15U) {
		words.push_back(("kvart_over"));
	} else if (min == 30U) {
		words.push_back(("halv"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	} else if (min == 45U) {
		words.push_back(("kvart_pa"));
		if (hour == 23U)
			hour = 0U;
		else
			hour++;
	}

	if (hour == 0U || hour == 12U) {
		words.push_back(("tolv"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("ett"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("to"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("tre"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("fire"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("fem"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("seks"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("sju"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("atte"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("ni"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("ti"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("elleve"));
	}

	return words;
}

std::vector<std::string> CTimeServerThread::sendTimePtPT(unsigned int hour, unsigned int min)
{
	std::vector<std::string> words;

	if (min == 45U) {
		hour++;
		if (hour == 24U)
			hour = 0U;
	}

	if (hour == 1U || hour == 13U)
		words.push_back(("E"));
	else if (hour == 0U || hour == 12U)
		words.push_back(("Es"));
	else
		words.push_back(("Sao"));

	if (min == 45U) {
		if (hour == 0U || hour == 12U || hour == 1U || hour == 13U)
			words.push_back(("quinze_para"));
		else
			words.push_back(("quinze_para_as"));
	}

	if (hour == 0U) {
		words.push_back(("meia-noite"));
	} else if (hour == 1U || hour == 13U) {
		words.push_back(("uma"));
	} else if (hour == 2U || hour == 14U) {
		words.push_back(("duas"));
	} else if (hour == 3U || hour == 15U) {
		words.push_back(("tres"));
	} else if (hour == 4U || hour == 16U) {
		words.push_back(("quatro"));
	} else if (hour == 5U || hour == 17U) {
		words.push_back(("cinco"));
	} else if (hour == 6U || hour == 18U) {
		words.push_back(("seis"));
	} else if (hour == 7U || hour == 19U) {
		words.push_back(("sete"));
	} else if (hour == 8U || hour == 20U) {
		words.push_back(("oito"));
	} else if (hour == 9U || hour == 21U) {
		words.push_back(("nove"));
	} else if (hour == 10U || hour == 22U) {
		words.push_back(("dez"));
	} else if (hour == 11U || hour == 23U) {
		words.push_back(("onze"));
	} else {
		words.push_back(("meio-dia"));
	}

	if (min == 0U)
		words.push_back(("hora"));
	else if (min == 15U)
		words.push_back(("e_quinze"));
	else if (min == 30U)
		words.push_back(("e_meia"));

	return words;
}

bool CTimeServerThread::loadAMBE()
{
	std::string ambeFileName;
	std::string indxFileName;

	switch (m_language) {
		case LANG_ENGLISH_US_1:
		case LANG_ENGLISH_US_2:
			ambeFileName = "TIME_en_US.ambe";
			indxFileName = "TIME_en_US.indx";
			break;
		case LANG_DEUTSCH_1:
		case LANG_DEUTSCH_2:
			ambeFileName = "TIME_de_DE.ambe";
			indxFileName = "TIME_de_DE.indx";
			break;
		case LANG_FRANCAIS:
			ambeFileName = "TIME_fr_FR.ambe";
			indxFileName = "TIME_fr_FR.indx";
			break;
		case LANG_NEDERLANDS:
			ambeFileName = "TIME_nl_NL.ambe";
			indxFileName = "TIME_nl_NL.indx";
			break;
		case LANG_SVENSKA:
			ambeFileName = "TIME_se_SE.ambe";
			indxFileName = "TIME_se_SE.indx";
			break;
		case LANG_ESPANOL:
			ambeFileName = "TIME_es_ES.ambe";
			indxFileName = "TIME_es_ES.indx";
			break;
		case LANG_NORSK:
			ambeFileName = "TIME_no_NO.ambe";
			indxFileName = "TIME_no_NO.indx";
			break;
		case LANG_PORTUGUES:
			ambeFileName = "TIME_pt_PT.ambe";
			indxFileName = "TIME_pt_PT.indx";
			break;
		default:
			ambeFileName = "TIME_en_GB.ambe";
			indxFileName = "TIME_en_GB.indx";
			break;
	}

	m_ambeFileReader = new CAMBEFileReader(m_dataPath + "/" + indxFileName, m_dataPath + "/" + ambeFileName);
	bool ret = m_ambeFileReader->read();

	if (!ret) {
		delete[] m_ambeFileReader;
		m_ambeFileReader = nullptr;
		return false;
	}

	return true;
}

void CTimeServerThread::buildAudio(const std::vector<std::string>& words, CSlowDataEncoder& slowDataEncoder)
{
	unsigned int seqNo = 0U;

	m_data.clear();

	if(words.size() == 0U || m_ambeFileReader == nullptr)
		CLog::logWarning("No words, falling back to text only");

	if(m_format == FORMAT_VOICE_TIME && words.size() != 0U) {
		// Build the audio
		m_ambeFileReader->lookup(" ", m_data);
		m_ambeFileReader->lookup(" ", m_data);
		m_ambeFileReader->lookup(" ", m_data);
		m_ambeFileReader->lookup(" ", m_data);

		for (unsigned int i = 0U; i < words.size(); i++)
			m_ambeFileReader->lookup(words.at(i), m_data);

		m_ambeFileReader->lookup(" ", m_data);
		m_ambeFileReader->lookup(" ", m_data);
		m_ambeFileReader->lookup(" ", m_data);
		m_ambeFileReader->lookup(" ", m_data);


		// add the slow data
		for(unsigned int i = 0U; i < m_data.size(); i++) {
			m_data[i]->setDestination(m_address, G2_DV_PORT);
			m_data[i]->setSeq(seqNo);

			unsigned char buffer[DV_FRAME_LENGTH_BYTES];
			m_data[i]->getData(buffer, DV_FRAME_LENGTH_BYTES);

			// Insert sync bytes when the sequence number is zero, slow data otherwise
			if (seqNo == 0U) {
				::memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
				slowDataEncoder.sync();
			} else {
				slowDataEncoder.getInterleavedData(buffer + VOICE_FRAME_LENGTH_BYTES);
			}

			m_data[i]->setData(buffer, DV_FRAME_LENGTH_BYTES);

			seqNo++;
			if(seqNo >= 21U) seqNo = 0U;
		}
	}
	else {
		for (unsigned int i = 0U; i < 21U; i++, seqNo++) {
			CAMBEData* dataOut = new CAMBEData;
			unsigned char buffer[DV_FRAME_LENGTH_BYTES];
			dataOut->setDestination(m_address, G2_DV_PORT);
			dataOut->setSeq(i);

			::memcpy(buffer + 0U, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

			// Insert sync bytes when the sequence number is zero, slow data otherwise
			if (i == 0U) {
				::memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
				slowDataEncoder.sync();
			} else {
				slowDataEncoder.getTextData(buffer + VOICE_FRAME_LENGTH_BYTES);
			}

			dataOut->setData(buffer, DV_FRAME_LENGTH_BYTES);

			m_data.push_back(dataOut);
		}
	}

	if(seqNo >= 21U) {
		seqNo = 0U;
	}

	CAMBEData* dataOut = new CAMBEData;
	dataOut->setData(END_PATTERN_BYTES, DV_FRAME_LENGTH_BYTES);
	dataOut->setDestination(m_address, G2_DV_PORT);
	dataOut->setSeq(seqNo);
	dataOut->setEnd(true);

	m_data.push_back(dataOut);
}

bool CTimeServerThread::send(const std::vector<std::string> &words, unsigned int hour, unsigned int min)
{
	CSlowDataEncoder encoder;

	CHeaderData header;
	header.setMyCall1(m_callsign);
	header.setRptCall1(m_callsignG);
	header.setRptCall2(m_callsign);		// Just for the slow data header
	header.setYourCall("CQCQCQ  ");
	header.setDestination(m_address, G2_DV_PORT);

	std::string slowData;
	switch (m_language) {
		case LANG_DEUTSCH_1:
		case LANG_DEUTSCH_2:
			header.setMyCall2(("ZEIT"));
			slowData = CStringUtils::string_format(("Es ist %02u:%02u Uhr"), hour, min);
			break;
		case LANG_FRANCAIS:
			header.setMyCall2(("TIME"));
			slowData = CStringUtils::string_format(("Il est %02u:%02u"), hour, min);
			break;
		case LANG_NEDERLANDS:
			header.setMyCall2(("TIJD"));
			slowData = CStringUtils::string_format(("Het is %02u:%02u"), hour, min);
			break;
		case LANG_SVENSKA:
			header.setMyCall2(("TID "));
			slowData = CStringUtils::string_format(("Klockan ar %02u:%02u"), hour, min);
			break;
		case LANG_ENGLISH_US_1:
		case LANG_ENGLISH_UK_1:
			header.setMyCall2(("TIME"));
			if (hour == 0U)
				slowData = CStringUtils::string_format(("It is 12:%02u AM"), min);
			else if (hour == 12U)
				slowData = CStringUtils::string_format(("It is 12:%02u PM"), min);
			else if (hour > 12U)
				slowData = CStringUtils::string_format(("It is %02u:%02u PM"), hour - 12U, min);
			else
				slowData = CStringUtils::string_format(("It is %02u:%02u AM"), hour, min);
			break;
		case LANG_ESPANOL:
			header.setMyCall2(("HORA"));
			if (hour == 1U)
				slowData = CStringUtils::string_format(("Es la %02u:%02u"), hour, min);
			else
				slowData = CStringUtils::string_format(("Son las %02u:%02u"), hour, min);
			break;
		case LANG_NORSK:
			header.setMyCall2(("TID "));
			slowData = CStringUtils::string_format(("Klokken er %02u:%02u"), hour, min);
			break;
		case LANG_PORTUGUES:
			header.setMyCall2(("HORA"));
			if (hour == 1U)
				slowData = CStringUtils::string_format(("E %02u:%02u"), hour, min);
			else
				slowData = CStringUtils::string_format(("Sao %02u:%02u"), hour, min);
			break;
		default:
			header.setMyCall2(("TIME"));
			slowData = CStringUtils::string_format(("It is %02u:%02u"), hour, min);
			break;
	}

	encoder.setHeaderData(header);
	encoder.setTextData(slowData);

	buildAudio(words, encoder);

	if (m_data.size() == 0U) {
		CLog::logWarning(("Not sending, no audio files loaded"));
		return false;
	}

	if(m_format == FORMAT_VOICE_TIME) {
		std::string text = boost::algorithm::join(words, " ");
		boost::replace_all(text, "_", " ");
		CLog::logInfo("Sending voice \"%s\", sending text \"%s\"", text.c_str(), slowData.c_str());
	}
	else {
		CLog::logInfo("Sending text \"%s\"", slowData.c_str());
	}

	// Build id and socket lists
	std::vector<unsigned int> ids;
	std::vector<CUDPReaderWriter *> sockets;
	for(auto rpt : m_repeaters) {
		auto socket = new CUDPReaderWriter("", 0U);
		sockets.push_back(socket);
		ids.push_back(CHeaderData::createId());
	}

	// open them all
	bool allOpen = std::all_of(sockets.begin(), sockets.end(), [](CUDPReaderWriter* s) { return s->open(); });
	if(allOpen) {
		//send headers
		for(unsigned int i = 0; i < m_repeaters.size(); i++) {
			CHeaderData headerCopy(header);
			headerCopy.setId(ids[i]);
			headerCopy.setRptCall2(m_repeaters[i]);
			sendHeader(*(sockets[i]), headerCopy);
			Sleep(5);
		}

		// send audio
		bool loop = true;
		unsigned int out = 0U;
		auto start = std::chrono::high_resolution_clock::now();

		while(loop) {
			unsigned int needed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
			needed /= DSTAR_FRAME_TIME_MS;

			while (out < needed) {
				for(unsigned int i = 0; i < m_repeaters.size(); i++) {
					CAMBEData data(*(m_data[out]));
					data.setId(ids[i]);
					sendData(*(sockets[i]), data);
					Sleep(5);
				}

				delete m_data[out];
				m_data[out] = nullptr;
				out++;

				if (out >= m_data.size()) {
					loop = false;
					break;
				}
			}
		}
	}

	m_data.clear();

	for(auto socket : sockets) {
		socket->close();
		delete socket;
	}

	return true;
}


bool CTimeServerThread::sendHeader(CUDPReaderWriter& socket, const CHeaderData &header)
{
	unsigned char buffer[60U];
	unsigned int length = header.getG2Data(buffer, 60U, true);

#if defined(DUMP_TX)
	CUtils::dump(("Sending Header"), buffer, length);
	return true;
#else
	for (unsigned int i = 0U; i < 5U; i++) {
		bool res = socket.write(buffer, length, header.getYourAddress(), header.getYourPort());
		if (!res)
			return false;
	}

	return true;
#endif
}

bool CTimeServerThread::sendData(CUDPReaderWriter& socket, const CAMBEData& data)
{
	unsigned char buffer[40U];
	unsigned int length = data.getG2Data(buffer, 40U);

#if defined(DUMP_TX)
	CUtils::dump(("Sending Data"), buffer, length);
	return true;
#else
	return socket.write(buffer, length, data.getYourAddressAndPort());
#endif
}

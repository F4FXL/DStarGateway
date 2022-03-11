/*
 *   Copyright (C) 2011-2014 by Jonathan Naylor G4KLX
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

#include <sys/stat.h>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <thread>

#include "DStarDefines.h"
#include "HeaderData.h"
#include "AudioUnit.h"
#include "Utils.h"
#include "Log.h"


CAMBEFileReader * CAudioUnit::m_ambeFilereader = nullptr;

TEXT_LANG CAudioUnit::m_language = TL_ENGLISH_UK;

const unsigned int MAX_FRAMES = 60U * DSTAR_FRAMES_PER_SEC;

const unsigned int SILENCE_LENGTH = 10U;

void CAudioUnit::initialise()
{
}

void CAudioUnit::setLanguage(const std::string & dir, TEXT_LANG language)
{
	if(m_ambeFilereader != nullptr) {
		delete m_ambeFilereader;
		m_ambeFilereader = nullptr;
	}

	m_language = language;

	std::string ambeFileName;
	std::string indxFileName;

	switch (language) {
		case TL_DEUTSCH:
			ambeFileName = "de_DE.ambe";
			indxFileName = "de_DE.indx";
			break;
		case TL_DANSK:
			ambeFileName = "dk_DK.ambe";
			indxFileName = "dk_DK.indx";
			break;
		case TL_ITALIANO:
			ambeFileName = "it_IT.ambe";
			indxFileName = "it_IT.indx";
			break;
		case TL_FRANCAIS:
			ambeFileName = "fr_FR.ambe";
			indxFileName = "fr_FR.indx";
			break;
		case TL_ESPANOL:
			ambeFileName = "es_ES.ambe";
			indxFileName = "es_ES.indx";
			break;
		case TL_SVENSKA:
			ambeFileName = "se_SE.ambe";
			indxFileName = "se_SE.indx";
			break;
		case TL_POLSKI:
			ambeFileName = "pl_PL.ambe";
			indxFileName = "pl_PL.indx";
			break;
		case TL_ENGLISH_US:
			ambeFileName = "en_US.ambe";
			indxFileName = "en_US.indx";
			break;
		case TL_NORSK:
			ambeFileName = "no_NO.ambe";
			indxFileName = "no_NO.indx";
			break;
		default:
			ambeFileName = "en_GB.ambe";
			indxFileName = "en_GB.indx";
			break;
	}

	m_ambeFilereader = new CAMBEFileReader(dir + "/" + indxFileName, dir + "/" + ambeFileName);
	bool ret = m_ambeFilereader->read();
	if(!ret) {
		delete m_ambeFilereader;
		m_ambeFilereader = nullptr;
	}
}

void CAudioUnit::finalise()
{
	delete m_ambeFilereader;
}

CAudioUnit::CAudioUnit(IRepeaterCallback* handler, const std::string& callsign) :
m_handler(handler),
m_callsign(callsign),
m_status(AS_IDLE),
m_linkStatus(LS_NONE),
m_tempLinkStatus(LS_NONE),
m_text(),
m_tempText(),
m_reflector(),
m_tempReflector(),
m_hasTemporary(false),
m_timer(1000U, REPLY_TIME),
m_data(),
m_out(0U)
//m_time()
{
	assert(handler != NULL);
}

CAudioUnit::~CAudioUnit()
{
	for (auto item : m_data) {
		delete item;
	}
	m_data.clear();
}

void CAudioUnit::sendStatus()
{
	if (m_ambeFilereader == nullptr)
		return;

	if (m_status != AS_IDLE)
		return;

	m_status = AS_WAIT;
	m_timer.start();
}

void CAudioUnit::setStatus(LINK_STATUS status, const std::string& reflector, const std::string& text)
{
	m_linkStatus = status;
	m_reflector  = reflector;
	m_text       = text;
}

void CAudioUnit::setTempStatus(LINK_STATUS status, const std::string& reflector, const std::string& text)
{
	m_tempLinkStatus = status;
	m_tempReflector  = reflector;
	m_tempText       = text;
	m_hasTemporary   = true;
}

void CAudioUnit::clock(unsigned int ms)
{
	m_timer.clock(ms);

	if (m_status == AS_WAIT && m_timer.hasExpired()) {
		if (m_hasTemporary) {
			sendStatus(m_tempLinkStatus, m_tempReflector, m_tempText);
			m_hasTemporary = false;
		} else {
			sendStatus(m_linkStatus, m_reflector, m_text);
		}

		m_timer.stop();

		m_out    = 0U;
		m_status = AS_TRANSMIT;

		m_time = std::chrono::high_resolution_clock::now();

		return;
	}

	if (m_status == AS_TRANSMIT) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		unsigned int needed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_time).count();
		needed /= DSTAR_FRAME_TIME_MS;

		while (m_out < needed && m_out < m_data.size()) {
			CAMBEData* data = m_data[m_out];
			m_out++;
			CLog::logTrace("m_out %u, needed %u, m_data %u", m_out, needed, m_data.size());
			m_handler->process(*data, DIR_INCOMING, AS_INFO);
		}

		if (m_out >= m_data.size()) {
			m_out    = 0U;
			m_status = AS_IDLE;
			m_timer.stop();
		}

		return;
	}
}

void CAudioUnit::cancel()
{
	CLog::logTrace("Audio Unit Cancel");
	m_status = AS_IDLE;
	m_out    = 0U;

	m_timer.stop();
}

void CAudioUnit::spellReflector(const std::string &reflector)
{
	unsigned int length = reflector.size();

	for (unsigned int i = 0; i < (length - 1); i++) {
		std::string c = reflector.substr(i, 1);

		if (c.compare(" "))
			m_ambeFilereader->lookup(c, m_data);
	}

	char c = reflector.at(length - 1);
	if (c == ' ')
		return;

	std::string cstr;
	cstr.push_back(c);
	if (m_linkStatus == LS_LINKING_DCS || m_linkStatus == LS_LINKED_DCS ||
	    m_linkStatus == LS_LINKING_CCS || m_linkStatus == LS_LINKED_CCS) {
		m_ambeFilereader->lookup(cstr, m_data);
		return;
	}

	switch (c) {
		case 'A':
			m_ambeFilereader->lookup("alpha", m_data);
			break;
		case 'B':
			m_ambeFilereader->lookup("bravo", m_data);
			break;
		case 'C':
			m_ambeFilereader->lookup("charlie", m_data);
			break;
		case 'D':
			m_ambeFilereader->lookup("delta", m_data);
			break;
		default:
			m_ambeFilereader->lookup(cstr, m_data);
			break;
	}
}

void CAudioUnit::sendStatus(LINK_STATUS status, const std::string& reflector, const std::string &text)
{
	CLog::logTrace("Audio Unit sendStatus");

	// do some clean up, delete old message
	for (auto item : m_data) {
		delete item;
	}
	m_data.clear();

	// Create the message
	m_ambeFilereader->lookup(" ", m_data);
	m_ambeFilereader->lookup(" ", m_data);
	m_ambeFilereader->lookup(" ", m_data);
	m_ambeFilereader->lookup(" ", m_data);

	bool found;

	switch (status) {
		case LS_NONE:
			m_ambeFilereader->lookup("notlinked", m_data);
			break;
		case LS_LINKED_CCS:
		case LS_LINKED_DCS:
		case LS_LINKED_DPLUS:
		case LS_LINKED_DEXTRA:
		case LS_LINKED_LOOPBACK:
			found = m_ambeFilereader->lookup("linkedto", m_data);
			if (!found) {
				m_ambeFilereader->lookup("linked", m_data);
				m_ambeFilereader->lookup("2", m_data);
			}
			spellReflector(reflector);
			break;
		default:
			found = m_ambeFilereader->lookup("linkingto", m_data);
			if (!found) {
				m_ambeFilereader->lookup("linking", m_data);
				m_ambeFilereader->lookup("2", m_data);
			}
			spellReflector(reflector);
			break;
	}

	m_ambeFilereader->lookup(" ", m_data);
	m_ambeFilereader->lookup(" ", m_data);
	m_ambeFilereader->lookup(" ", m_data);
	m_ambeFilereader->lookup(" ", m_data);

	unsigned int id = CHeaderData::createId();
	// RPT1 and RPT2 will be filled in later
	CHeaderData header;
	header.setMyCall1(m_callsign);
	header.setMyCall2("INFO");
	header.setYourCall("CQCQCQ  ");
	header.setId(id);

	CSlowDataEncoder slowDataEncoder;
	slowDataEncoder.setTextData(text);
	unsigned int seqNo = 0U;

	// add the slow data, id, seq num etc ...
	for(unsigned int i = 0U; i < m_data.size(); i++) {
		m_data[i]->setId(id);
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

	m_data[m_data.size() - 1]->setEnd(true);

	m_handler->process(header, DIR_INCOMING, AS_INFO);
}

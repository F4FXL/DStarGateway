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

#include "DStarDefines.h"
#include "HeaderData.h"
#include "AudioUnit.h"
#include "Utils.h"
#include "Log.h"

unsigned char* CAudioUnit::m_ambe = NULL;
unsigned int   CAudioUnit::m_ambeLength = 0U;
std::map<std::string, CIndexRecord *> CAudioUnit::m_index;

TEXT_LANG CAudioUnit::m_language = TL_ENGLISH_UK;

const unsigned int MAX_FRAMES = 60U * DSTAR_FRAMES_PER_SEC;

const unsigned int SILENCE_LENGTH = 10U;

void CAudioUnit::initialise()
{
}

void CAudioUnit::setLanguage(const std::string & dir, TEXT_LANG language)
{
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

	bool ret = readAMBE(dir, ambeFileName);
	if (!ret) {
		delete[] m_ambe;
		m_ambe = NULL;
		return;
	}

	ret = readIndex(dir, indxFileName);
	if (!ret) {
		delete[] m_ambe;
		m_ambe = NULL;
	}
}

void CAudioUnit::finalise()
{
	for (std::map<std::string, CIndexRecord *>::iterator it = m_index.begin(); it != m_index.end(); ++it)
		delete it->second;

	delete[] m_ambe;
}

CAudioUnit::CAudioUnit(IRepeaterCallback* handler, const std::string& callsign) :
m_handler(handler),
m_callsign(callsign),
m_encoder(),
m_status(AS_IDLE),
m_linkStatus(LS_NONE),
m_tempLinkStatus(LS_NONE),
m_text(),
m_tempText(),
m_reflector(),
m_tempReflector(),
m_hasTemporary(false),
m_timer(1000U, REPLY_TIME),
m_data(NULL),
m_in(0U),
m_out(0U),
m_seqNo(0U) //,
//m_time()
{
	assert(handler != NULL);

	m_data = new CAMBEData*[MAX_FRAMES];

	for (unsigned int i = 0U; i < MAX_FRAMES; i++)
		m_data[i] = NULL;
}

CAudioUnit::~CAudioUnit()
{
	delete[] m_data;
}

void CAudioUnit::sendStatus()
{
	if (m_ambe == NULL)
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
		m_seqNo  = 0U;
		m_status = AS_TRANSMIT;

		m_time = std::chrono::high_resolution_clock::now();

		return;
	}

	if (m_status == AS_TRANSMIT) {
		std::chrono::high_resolution_clock::time_point hrctp = std::chrono::high_resolution_clock::now();
		auto elapse = std::chrono::duration_cast<std::chrono::milliseconds>(hrctp - m_time);
		unsigned int needed = elapse.count() / DSTAR_FRAME_TIME_MS;

		while (m_out < needed) {
			CAMBEData* data = m_data[m_out];
			m_data[m_out] = NULL;
			m_out++;

			if (m_in == m_out)
				data->setEnd(true);

			m_handler->process(*data, DIR_INCOMING, AS_INFO);

			delete data;

			if (m_in == m_out) {
				m_in     = 0U;
				m_out    = 0U;
				m_status = AS_IDLE;
				m_timer.stop();
				return;
			}
		}

		return;
	}
}

void CAudioUnit::cancel()
{
	for (unsigned int i = 0U; i < MAX_FRAMES; i++) {
		if (m_data[i] != NULL) {
			delete m_data[i];
			m_data[i] = NULL;
		}
	}

	m_status = AS_IDLE;
	m_out    = 0U;
	m_in     = 0U;

	m_timer.stop();
}

bool CAudioUnit::lookup(unsigned int id, const std::string &name)
{
	CIndexRecord* info = m_index[name];
	if (info == NULL) {
		// CLog::logError("Cannot find the AMBE index for *%s*", name.c_str());
		return false;
	}

	unsigned int  start = info->getStart();
	unsigned int length = info->getLength();

	for (unsigned int i = 0U; i < length; i++) {
		unsigned char* dataIn = m_ambe + (start + i) * VOICE_FRAME_LENGTH_BYTES;

		CAMBEData* dataOut = new CAMBEData;
		dataOut->setSeq(m_seqNo);
		dataOut->setId(id);

		unsigned char buffer[DV_FRAME_LENGTH_BYTES];
		memcpy(buffer + 0U, dataIn, VOICE_FRAME_LENGTH_BYTES);

		// Insert sync bytes when the sequence number is zero, slow data otherwise
		if (m_seqNo == 0U) {
			memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
			m_encoder.sync();
		} else {
			m_encoder.getTextData(buffer + VOICE_FRAME_LENGTH_BYTES);
		}

		dataOut->setData(buffer, DV_FRAME_LENGTH_BYTES);

		m_seqNo++;
		if (m_seqNo == 21)
			m_seqNo = 0;

		m_data[m_in] = dataOut;
		m_in++;
	}

	return true;
}

void CAudioUnit::spellReflector(unsigned int id, const std::string &reflector)
{
	unsigned int length = reflector.size();

	for (unsigned int i = 0; i < (length - 1); i++) {
		std::string c = reflector.substr(i, 1);

		if (c.compare(" "))
			lookup(id, c);
	}

	char c = reflector.at(length - 1);

	if (c == ' ')
		return;

	std::string cstr;
	cstr.push_back(c);
	if (m_linkStatus == LS_LINKING_DCS || m_linkStatus == LS_LINKED_DCS ||
	    m_linkStatus == LS_LINKING_CCS || m_linkStatus == LS_LINKED_CCS) {
		lookup(id, cstr);
		return;
	}

	switch (c) {
		case 'A':
			lookup(id, "alpha");
			break;
		case 'B':
			lookup(id, "bravo");
			break;
		case 'C':
			lookup(id, "charlie");
			break;
		case 'D':
			lookup(id, "delta");
			break;
		default:
			lookup(id, cstr);
			break;
	}
}

bool CAudioUnit::readAMBE(const std::string& dir, const std::string& name)
{
	std::string fileName = dir + "/" + name;
	struct stat sbuf;
	
	if (stat(fileName.c_str(), &sbuf)) {
		CLog::logInfo("File %s not readable\n", fileName.c_str());
		fileName.append("/data/");
		fileName += name;
		if (stat(fileName.c_str(), &sbuf)) {
			CLog::logInfo("File %s not readable\n", fileName.c_str());
			return false;
		}
	}
	unsigned int fsize = sbuf.st_size;

	FILE *file = fopen(fileName.c_str(), "rb");
	if (NULL == file) {
		CLog::logInfo("Cannot open %s for reading\n", fileName.c_str());
		return false;
	}

	CLog::logInfo("Reading %s\n", fileName.c_str());

	unsigned char buffer[VOICE_FRAME_LENGTH_BYTES];

	size_t n = fread(buffer, sizeof(unsigned char), 4, file);
	if (n != 4) {
		CLog::logError("Unable to read the header from %s\n", fileName.c_str());
		fclose(file);
		return false;
	}

	if (memcmp(buffer, "AMBE", 4)) {
		CLog::logError("Invalid header from %s\n", fileName.c_str());
		fclose(file);
		return false;
	}

	// Length of the file minus the header
	unsigned int length = fsize - 4U;

	// Hold the file data plus silence at the end
	m_ambe = new unsigned char[length + SILENCE_LENGTH * VOICE_FRAME_LENGTH_BYTES];
	m_ambeLength = length / VOICE_FRAME_LENGTH_BYTES;

	// Add silence to the beginning of the buffer
	unsigned char* p = m_ambe;
	for (unsigned int i = 0U; i < SILENCE_LENGTH; i++, p += VOICE_FRAME_LENGTH_BYTES)
		memcpy(p, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

	n = fread(p, 1, length, file);
	if (n != length) {
		CLog::logError("Unable to read the AMBE data from %s\n", fileName.c_str());
		fclose(file);
		delete[] m_ambe;
		m_ambe = NULL;
		return false;
	}

	fclose(file);

	return true;
}

bool CAudioUnit::readIndex(const std::string& dir, const std::string& name)
{
	std::string fileName = dir + "/" + name;
	struct stat sbuf;
	
	if (stat(fileName.c_str(), &sbuf)) {
		CLog::logInfo("File %s not readable\n", fileName.c_str());
		fileName.append("/data/");
		fileName += name;
		if (stat(fileName.c_str(), &sbuf)) {
			CLog::logInfo("File %s not readable\n", fileName.c_str());
			return false;
		}
	}

	FILE *file = fopen(fileName.c_str(), "r");
	if (NULL == file) {
		CLog::logInfo("Cannot open %s for reading\n", fileName.c_str());
		return false;
	}

	// Add a silence entry at the beginning
	m_index[" "] = new CIndexRecord(" ", 0, SILENCE_LENGTH);

	CLog::logInfo("Reading %s\n", fileName.c_str());

	char line[128];
	while (fgets(line, 128, file)) {

		if (strlen(line) && '#'!=line[0]) {
			const std::string space(" \t\r\n");
			std::string name(strtok(line, space.c_str()));
			std::string strt(strtok(NULL, space.c_str()));
			std::string leng(strtok(NULL, space.c_str()));

			if (name.size() && strt.size() && leng.size()) {
				unsigned long start  = std::stoul(strt);
				unsigned long length = std::stoul(leng);

				if (start >= m_ambeLength || (start + length) >= m_ambeLength)
					CLog::logInfo("The start or end for *%s* is out of range, start: %lu, end: %lu\n", name.c_str(), start, start + length);
				else
					m_index[name] = new CIndexRecord(name, start + SILENCE_LENGTH, length);
			}
		}
	}

	fclose(file);

	return true;
}

void CAudioUnit::sendStatus(LINK_STATUS status, const std::string& reflector, const std::string &text)
{
		m_encoder.setTextData(text);

		// Create the message
		unsigned int id = CHeaderData::createId();

		lookup(id, " ");
		lookup(id, " ");
		lookup(id, " ");
		lookup(id, " ");

		bool found;

		switch (status) {
			case LS_NONE:
				lookup(id, "notlinked");
				break;
			case LS_LINKED_CCS:
			case LS_LINKED_DCS:
			case LS_LINKED_DPLUS:
			case LS_LINKED_DEXTRA:
			case LS_LINKED_LOOPBACK:
				found = lookup(id, "linkedto");
				if (!found) {
					lookup(id, "linked");
					lookup(id, "2");
				}
				spellReflector(id, reflector);
				break;
			default:
				found = lookup(id, "linkingto");
				if (!found) {
					lookup(id, "linking");
					lookup(id, "2");
				}
				spellReflector(id, reflector);
				break;
		}

		lookup(id, " ");
		lookup(id, " ");
		lookup(id, " ");
		lookup(id, " ");

		// RPT1 and RPT2 will be filled in later
		CHeaderData header;
		header.setMyCall1(m_callsign);
		header.setMyCall2("INFO");
		header.setYourCall("CQCQCQ  ");
		header.setId(id);

		m_handler->process(header, DIR_INCOMING, AS_INFO);
}

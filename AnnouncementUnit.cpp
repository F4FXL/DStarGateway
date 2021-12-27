/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
 *   Copyright (c) Thomas A. Early N7TAE
 *   Copyright (c) 2021 Geoffrey Merck F4FXL / KC3FRA
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

#include "DStarDefines.h"
#include "HeaderData.h"
#include "AnnouncementUnit.h"
#include "Utils.h"
#include "Log.h"

CAnnouncementUnit::CAnnouncementUnit(IRepeaterCallback* handler, const std::string& callsign, const std::string& fileName, const std::string& name) :
m_handler(handler),
m_callsign(callsign),
m_fileName(fileName),
m_name(name),
m_reader(NULL),
m_status(NS_IDLE),
m_timer(1000U, REPLY_TIME),
m_out(0U),
m_id(0U)
{
	assert(handler != NULL);

	m_name.resize(SHORT_CALLSIGN_LENGTH, ' ');

	CLog::logInfo("Connected '%s' to %s using file - %s\n", name.c_str(), callsign.c_str(), fileName.c_str());
}

CAnnouncementUnit::~CAnnouncementUnit()
{
}

void CAnnouncementUnit::sendAnnouncement()
{
	if (m_status != NS_IDLE)
		return;

	struct stat sbuf;
	if (stat(m_fileName.c_str(), &sbuf))
		return;

	m_reader = new CDVTOOLFileReader;

	bool ret = m_reader->open(m_fileName);
	if (!ret) {
		delete m_reader;
		m_reader = NULL;
		return;
	}

	m_status = NS_WAIT;
	m_timer.start();
}

void CAnnouncementUnit::clock(unsigned int ms)
{
	m_timer.clock(ms);

	if (m_status == NS_WAIT && m_timer.hasExpired()) {
		m_reader->read();		// Pop the first record off the file

		m_id = CHeaderData::createId();

		CHeaderData header;
		header.setMyCall1(m_callsign);
		header.setMyCall2(m_name);
		header.setYourCall("CQCQCQ  ");
		header.setId(m_id);

		m_handler->process(header, DIR_INCOMING, AS_INFO);

		m_timer.stop();

		m_out    = 0U;
		m_status = NS_TRANSMIT;

//		m_time.Start();
		m_start = std::chrono::high_resolution_clock::now();

		return;
	}

	if (m_status == NS_TRANSMIT) {

		unsigned int needed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start).count();
		needed /= DSTAR_FRAME_TIME_MS;
//		unsigned int needed = m_time.Time() / DSTAR_FRAME_TIME_MS;

		while (m_out < needed) {
			DVTFR_TYPE type = m_reader->read();
			if (type != DVTFR_DATA) {
				m_out    = 0U;
				m_status = NS_IDLE;
				m_timer.stop();
				m_reader->close();
				delete m_reader;
				m_reader = NULL;
				return;
			}

			CAMBEData* data = m_reader->readData();
			data->setId(m_id);

			m_out++;

			m_handler->process(*data, DIR_INCOMING, AS_INFO);

			bool end = data->isEnd();

			delete data;

			if (end) {
				m_out    = 0U;
				m_status = NS_IDLE;
				m_reader->close();
				delete m_reader;
				m_reader = NULL;
				m_timer.stop();
				return;
			}
		}

		return;
	}
}

void CAnnouncementUnit::cancel()
{
	m_status = NS_IDLE;
	m_out    = 0U;
	m_id     = 0U;

	if (m_reader != NULL) {
		m_reader->close();
		delete m_reader;
		m_reader = NULL;
	}

	m_timer.stop();
}

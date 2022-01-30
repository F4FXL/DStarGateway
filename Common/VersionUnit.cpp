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

#include <cassert>
#include <cstdio>
#include <cstring>

#include "SlowDataEncoder.h"
#include "DStarDefines.h"
#include "VersionUnit.h"
#include "HeaderData.h"
#include "Version.h"
#include "Utils.h"
#include "Log.h"

const unsigned int NUM_FRAMES = 20U;

CVersionUnit::CVersionUnit(IRepeaterCallback* handler, const std::string& callsign) :
m_handler(handler),
m_callsign(callsign),
m_status(VS_IDLE),
m_timer(1000U, REPLY_TIME),
m_data(NULL),
m_id(0U),
m_out(0U)
{
	assert(handler != NULL);

	m_data = new CAMBEData*[NUM_FRAMES];

	auto vstr = SHORT_PRODUCT_NAME;
	vstr.resize(NUM_FRAMES, ' ');

	CLog::logInfo("Version text set to \"%s\"\n", vstr.c_str());

	CSlowDataEncoder encoder;
	encoder.setTextData(vstr);

	// Seq No and end
	for (unsigned int i = 0U; i < NUM_FRAMES; i++) {
		unsigned char buffer[DV_FRAME_LENGTH_BYTES];
		memcpy(buffer + 0U, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

		// Insert sync bytes when the sequence number is zero, slow data otherwise
		if (i == 0U) {
			memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
			encoder.sync();
		} else {
			encoder.getTextData(buffer + VOICE_FRAME_LENGTH_BYTES);
		}

		m_data[i] = new CAMBEData;
		m_data[i]->setData(buffer, DV_FRAME_LENGTH_BYTES);
		m_data[i]->setSeq(i);

		if (i == (NUM_FRAMES - 1U))
			m_data[i]->setEnd(true);
	}
}

CVersionUnit::~CVersionUnit()
{
	for (unsigned int i = 0U; i < NUM_FRAMES; i++)
		delete m_data[i];

	delete[] m_data;
}

void CVersionUnit::sendVersion()
{
	if (m_status != VS_IDLE)
		return;

	m_id = CHeaderData::createId();

	m_status = VS_WAIT;
	m_timer.start();
}

void CVersionUnit::clock(unsigned int ms)
{
	m_timer.clock(ms);

	if (m_status == VS_WAIT && m_timer.hasExpired()) {
		m_timer.stop();

		// RPT1 and RPT2 will be filled in later
		CHeaderData header;
		header.setMyCall1(m_callsign);
		header.setMyCall2("VERS");
		header.setYourCall("CQCQCQ  ");
		header.setId(m_id);

		m_handler->process(header, DIR_INCOMING, AS_VERSION);

		m_out    = 0U;
		m_status = VS_TRANSMIT;

		m_time = std::chrono::high_resolution_clock::now();

		return;
	}

	if (m_status == VS_TRANSMIT) {
		auto hrct = std::chrono::high_resolution_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(hrct - m_time);
		unsigned int needed = elapsed.count() / DSTAR_FRAME_TIME_MS;

		while (m_out < needed) {
			CAMBEData* data = m_data[m_out];
			data->setId(m_id);

			m_out++;

			m_handler->process(*data, DIR_INCOMING, AS_VERSION);

			if (m_out == NUM_FRAMES) {
				m_out    = 0U;
				m_status = VS_IDLE;
				return;
			}
		}

		return;
	}
}

void CVersionUnit::cancel()
{
	m_status = VS_IDLE;
	m_out    = 0U;

	m_timer.stop();
}

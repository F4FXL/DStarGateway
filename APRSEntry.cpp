/*
 *   Copyright (C) 2010,2011,2012,2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include <boost/algorithm/string.hpp>

#include "APRSEntry.h"

CAPRSEntry::CAPRSEntry(const std::string& callsign, const std::string& band, double frequency, double offset, double range, double latitude, double longitude, double agl) :
m_callsign(callsign),
m_band(band),
m_frequency(frequency),
m_offset(offset),
m_range(range),
m_latitude(latitude),
m_longitude(longitude),
m_agl(agl),
m_timer(1000U, 10U),
m_first(true),
m_collector(NULL),
m_linkStatus()
{
	boost::trim(m_callsign);

	m_collector = new CAPRSCollector;
}

CAPRSEntry::~CAPRSEntry()
{
	delete m_collector;
}

std::string CAPRSEntry::getCallsign() const
{
	return m_callsign;
}

std::string CAPRSEntry::getBand() const
{
	return m_band;
}

double CAPRSEntry::getFrequency() const
{
	return m_frequency;
}

double CAPRSEntry::getOffset() const
{
	return m_offset;
}

double CAPRSEntry::getRange() const
{
	return m_range;
}

double CAPRSEntry::getLatitude() const
{
	return m_latitude;
}

double CAPRSEntry::getLongitude() const
{
	return m_longitude;
}

double CAPRSEntry::getAGL() const
{
	return m_agl;
}

CAPRSCollector* CAPRSEntry::getCollector() const
{
	return m_collector;
}

CAPRSEntryStatus& CAPRSEntry::getStatus()
{
	return m_linkStatus;
}

void CAPRSEntry::reset()
{
	m_first = true;
	m_timer.stop();
	m_collector->reset();
}

void CAPRSEntry::clock(unsigned int ms)
{
	m_linkStatus.clock(ms);
	m_timer.clock(ms);
}

bool CAPRSEntry::isOK()
{
	if (m_first) {
		m_first = false;
		m_timer.start();
		return true;
	}

	if (m_timer.hasExpired()) {
		m_timer.start();
		return true;
	} else {
		m_timer.start();
		return false;
	}
}
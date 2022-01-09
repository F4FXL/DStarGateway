/*
 *   Copyright (C) 2010,2012,2018 by Jonathan Naylor G4KLX
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

#include <cassert>
#include <cstring>
#include <string>
#include <boost/algorithm/string.hpp>

#include "APRSCollector.h"
#include "DStarDefines.h"
#include "Utils.h"
#include "NMEASentenceCollector.h"
#include "GPSACollector.h"
#include "RSMS1AMessageCollector.h"
#include "SlowDataCollectorThrottle.h"

const unsigned int APRS_CSUM_LENGTH = 4U;
const unsigned int APRS_DATA_LENGTH = 300U;
const unsigned int SLOW_DATA_BLOCK_LENGTH = 6U;

const char APRS_OVERLAY = '\\';
const char APRS_SYMBOL  = 'K';

CAPRSCollector::CAPRSCollector() :
m_collectors()
{
	m_collectors.push_back(new CRSMS1AMessageCollector()); // we do not throttle messages, they have highest priority !
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CGPSACollector(), 10U));
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CNMEASentenceCollector("$GPGGA"), 10U));
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CNMEASentenceCollector("$GPGLL"), 10U));
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CNMEASentenceCollector("$GPVTG"), 10U));
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CNMEASentenceCollector("$GPRMC"), 10U));
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CNMEASentenceCollector("$GPGSA"), 10U));
	m_collectors.push_back(new CSlowDataCollectorThrottle(new CNMEASentenceCollector("$GPGSV"), 10U));
}

CAPRSCollector::~CAPRSCollector()
{
	for(auto collector : m_collectors) {
		delete collector;
	}
	m_collectors.clear();
}

void CAPRSCollector::writeHeader(const std::string& callsign)
{
	for(auto collector : m_collectors) {
		collector->setMyCall(callsign);
	}
}

bool CAPRSCollector::writeData(const unsigned char* data)
{
	bool ret = false;
	for(auto collector : m_collectors) {
		bool ret2 = collector->writeData(data);
		ret = ret || ret2;
	}
	return ret;
}

void CAPRSCollector::reset()
{
	for(auto collector : m_collectors) {
		collector->reset();
	}
}

void CAPRSCollector::sync()
{
	for(auto collector : m_collectors) {
		collector->sync();
	}
}

unsigned int CAPRSCollector::getData(unsigned char dataType, unsigned char* data, unsigned int length)
{
	for(auto collector : m_collectors) {
		if(collector->getDataType() == dataType) {
			unsigned int res = collector->getData(data, length);
			if(res > 0U)
				return res;
		}
	}
	return 0U;
}

void CAPRSCollector::getData(std::function<void(const std::string&)> dataHandler)
{
	for(auto collector : m_collectors) {
		std::string data;
		if(collector->getData(data)) {
			dataHandler(data);
			collector->reset();
		}
	}
}

void CAPRSCollector::clock(unsigned int ms)
{
	for(auto collector : m_collectors) {
		collector->clock(ms);
	}
}
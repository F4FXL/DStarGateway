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

#ifndef APRSCollector_H
#define APRSCollector_H

#include <vector>
#include <functional>

#include "SlowDataCollector.h"
#include "Defs.h"

enum APRS_STATE {
	AS_NONE,
	AS_GGA,
	AS_RMC,
	AS_MSG,
	AS_CRC
};

class CAPRSCollector {
public:
	CAPRSCollector();
	~CAPRSCollector();

	void writeHeader(const std::string& callsign);

	bool writeData(const unsigned char* data);

	void reset();

	void sync();

	unsigned int getData(unsigned char dataType, unsigned char* data, unsigned int length);
	
	void getData(std::function<void(const std::string&)> dataHandler);

	void clock(unsigned int ms);

private:
	std::vector<ISlowDataCollector *> m_collectors;
};

#endif

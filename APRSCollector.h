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

#include "Defs.h"

enum APRS_STATE {
	AS_NONE,
	AS_GGA,
	AS_RMC,
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

	unsigned int getData(unsigned char* data, unsigned int length);

private:
	APRS_STATE     m_state;
	unsigned char* m_ggaData;
	unsigned int   m_ggaLength;
	bool           m_ggaValid;
	unsigned char* m_rmcData;
	unsigned int   m_rmcLength;
	bool           m_rmcValid;
	unsigned char* m_crcData;
	unsigned int   m_crcLength;
	bool           m_crcValid;
	unsigned char* m_buffer;
	SLOWDATA_STATE m_slowData;
	std::string       m_collector;
	std::string       m_callsign;

	bool addGPSData(const unsigned char* data);

	void addGGAData();
	bool addRMCData();
	bool addCRCData();

	bool checkXOR(const unsigned char* data, unsigned int length) const;
	unsigned char calcXOR(const unsigned char* buffer, unsigned int length) const;

	bool checkCRC(const unsigned char* data, unsigned int length) const;
	unsigned int calcCRC(const unsigned char* buffer, unsigned int length) const;

	unsigned int convertNMEA1(unsigned char* data, unsigned int length);
	unsigned int convertNMEA2(unsigned char* data, unsigned int length);

	void dstarCallsignToAPRS(const std::string& dstarCallsign, char* aprsCallsign) const;

	char* mystrsep(char** sp, const char* sep) const;
};

#endif

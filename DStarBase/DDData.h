/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
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

#include <netinet/in.h>
#include "HeaderData.h"

class CDDData {
public:
	CDDData();
	CDDData(const CDDData& data);
	~CDDData();

	bool setIcomRepeaterData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort);
	bool setHBRepeaterData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort);

	unsigned int getIcomRepeaterData(unsigned char* data, unsigned int length);
	unsigned int getHBRepeaterData(unsigned char* data, unsigned int length);

	unsigned char getBand1() const;
	unsigned char getBand2() const;
	unsigned char getBand3() const;
	void setBand1(unsigned char band);
	void setBand2(unsigned char band);
	void setBand3(unsigned char band);

	unsigned char getFlag1() const;
	unsigned char getFlag2() const;
	unsigned char getFlag3() const;
	void setFlags(unsigned char flag1, unsigned char flag2, unsigned char flag3);

	std::string getMyCall1() const;
	std::string getMyCall2() const;
	std::string getYourCall() const;
	std::string getRptCall1() const;
	std::string getRptCall2() const;

	void setMyCall1(const std::string& callsign);
	void setMyCall2(const std::string& callsign);
	void setYourCall(const std::string& callsign);
	void setRptCall1(const std::string& callsign);
	void setRptCall2(const std::string& callsign);

	unsigned int getRptSeq() const;
	void setRptSeq(unsigned int seqNo);

	void setEthernetFrame(const unsigned char* frame, unsigned int length);
	unsigned int getEthernetFrame(unsigned char* frame, unsigned int length) const;

	unsigned char* getSourceAddress() const;
	unsigned char* getDestinationAddress() const;

	void setRepeaters(const std::string& rpt1, const std::string& rpt2);
	void setDestination(const in_addr& address, unsigned int port);

	in_addr      getYourAddress() const;
	unsigned int getYourPort() const;

private:
	CHeaderData    m_header;
	unsigned int   m_length;
	unsigned char* m_frame;
};


/*
 *   Copyright (C) 2010,2012,2013 by Jonathan Naylor G4KLX
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

#include "Defs.h"

class CPollData {
public:
	CPollData(const std::string& data1, const std::string& data2, DIRECTION direction, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CPollData(const std::string& data, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CPollData(const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CPollData();
	~CPollData();

	bool setDExtraData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);
	bool setDPlusData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);
	bool setDCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);
	bool setCCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);

	unsigned int getDExtraData(unsigned char* data, unsigned int length) const;
	unsigned int getDPlusData(unsigned char* data, unsigned int length) const;
	unsigned int getDCSData(unsigned char* data, unsigned int length) const;
	unsigned int getCCSData(unsigned char* data, unsigned int length) const;

	std::string  getData1() const;
	void         setData1(const std::string& data);

	std::string  getData2() const;
	void         setData2(const std::string& data);

	bool         isDongle() const;

	in_addr      getYourAddress() const;
	unsigned int getYourPort() const;
	unsigned int getMyPort() const;

	DIRECTION    getDirection() const;
	unsigned int getLength() const;

private:
	std::string  m_data1;
	std::string  m_data2;
	DIRECTION    m_direction;
	bool         m_dongle;
	unsigned int m_length;
	in_addr      m_yourAddress;
	unsigned int m_yourPort;
	unsigned int m_myPort;
};


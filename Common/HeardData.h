/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
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
#include <netinet/in.h>

#include "DStarDefines.h"
#include "HeaderData.h"

class CHeardData {
public:
	CHeardData();
	CHeardData(const CHeardData& data);
	CHeardData(const CHeaderData& data, const std::string& repeater, const std::string& reflector);
	~CHeardData();

	bool setIcomRepeaterData(const unsigned char* data, unsigned int length, const in_addr& address, unsigned int port);

	unsigned int getCCSData(unsigned char* data, unsigned int length) const;

	std::string     getRepeater() const;
	std::string     getUser() const;

	void setDestination(const in_addr& address, unsigned int port);

	struct sockaddr_storage getDestination() const;

	in_addr      getAddress() const;
	unsigned int getPort() const;

private:
	std::string  m_reflector;
	std::string  m_repeater;
	std::string  m_user;
	std::string  m_ext;
	in_addr      m_address;
	unsigned int m_port;
};

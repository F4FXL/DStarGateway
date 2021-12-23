/*
 *   Copyright (C) 2011,2012 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early
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

class CStatusData {
public:
	CStatusData(const std::string& text, unsigned int n);
	virtual ~CStatusData();

	unsigned int getHBRepeaterData(unsigned char* data, unsigned int length) const;

	void setDestination(const in_addr& address, unsigned int port);

	in_addr      getAddress() const;
	unsigned int getPort() const;

private:
	unsigned char* m_data;
	unsigned int   m_n;
	in_addr        m_address;
	unsigned int   m_port;
};

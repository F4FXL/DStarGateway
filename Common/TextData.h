/*
 *   Copyright (C) 2010,2011,2012,2013 by Jonathan Naylor G4KLX
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

class CTextData {
public:
	CTextData(LINK_STATUS status, const std::string& reflector, const std::string& text, const in_addr& address, unsigned int port, bool temporary = false);
	CTextData(const std::string& text, const in_addr& address, unsigned int port, bool temporary = true);
	virtual ~CTextData();

	unsigned int getHBRepeaterData(unsigned char* data, unsigned int length) const;

	in_addr      getAddress() const;
	unsigned int getPort() const;

private:
	LINK_STATUS    m_status;
	unsigned char* m_reflector;
	unsigned char* m_text;
	in_addr        m_address;
	unsigned int   m_port;
	bool           m_temporary;
};

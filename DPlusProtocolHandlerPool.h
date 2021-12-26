/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas A. Early
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
#include <map>

#include "DPlusProtocolHandler.h"

class CDPlusProtocolHandlerPool {
public:
	CDPlusProtocolHandlerPool(const unsigned int port, const std::string &addr = std::string(""));
	~CDPlusProtocolHandlerPool();

	CDPlusProtocolHandler *getHandler();
	CDPlusProtocolHandler *getIncomingHandler();
	void release(CDPlusProtocolHandler *handler);

	DPLUS_TYPE   read();
	CHeaderData*  readHeader();
	CAMBEData*    readAMBE();
	CPollData*    readPoll();
	CConnectData* readConnect();

	void close();

private:
	CDPlusProtocolHandler *getHandler(unsigned int port);

	std::map<unsigned int, CDPlusProtocolHandler *> m_pool;
	std::map<unsigned int, CDPlusProtocolHandler *>::iterator m_index;
	unsigned int m_basePort;
	std::string m_address;
};

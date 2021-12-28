/*
 *   Copyright (C) 2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas A. Early N7TAE
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
#include <map>
#include <mutex>

#include "DCSProtocolHandler.h"

class CDCSProtocolHandlerPool {
public:
	CDCSProtocolHandlerPool(const unsigned int port, const std::string &addr = std::string(""));
	~CDCSProtocolHandlerPool();

	CDCSProtocolHandler *getHandler();
	CDCSProtocolHandler *getIncomingHandler();
	void release(CDCSProtocolHandler *handler);

	DCS_TYPE      read();
	CAMBEData    *readData();
	CPollData    *readPoll();
	CConnectData *readConnect();

	void close();

private:
	CDCSProtocolHandler *getHandler(unsigned int port);
	std::map<int,CDCSProtocolHandler *> m_pool;
	std::map<int,CDCSProtocolHandler *>::iterator m_index;
	unsigned int m_basePort;
	std::string m_address;
};


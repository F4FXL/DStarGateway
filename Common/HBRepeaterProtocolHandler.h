/*
 *   Copyright (C) 2010-2013 by Jonathan Naylor G4KLX
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

#ifndef	HBRepeaterProtocolHandler_H
#define	HBRepeaterProtocolHandler_H

#include <netinet/in.h>
#include <string>

#include "RepeaterProtocolHandler.h"
#include "UDPReaderWriter.h"
#include "DStarDefines.h"
#include "HeaderData.h"
#include "StatusData.h"
#include "HeardData.h"
#include "AMBEData.h"
#include "TextData.h"
#include "PollData.h"
#include "DDData.h"

class CHBRepeaterProtocolHandler : public IRepeaterProtocolHandler {
public:
	CHBRepeaterProtocolHandler(const std::string& address, unsigned int port);
	virtual ~CHBRepeaterProtocolHandler();

	virtual bool open();

	virtual bool writeHeader(CHeaderData& header);
	virtual bool writeAMBE(CAMBEData& data);
	virtual bool writeDD(CDDData& data);
	virtual bool writeText(CTextData& text);
	virtual bool writeStatus(CStatusData& status);

	virtual REPEATER_TYPE read();
	virtual CPollData*    readPoll();
	virtual CHeardData*   readHeard();
	virtual CHeaderData*  readHeader();
	virtual CAMBEData*    readAMBE();
	virtual CDDData*      readDD();
	virtual CHeaderData*  readBusyHeader();
	virtual CAMBEData*    readBusyAMBE();

	virtual void close();

private:
	CUDPReaderWriter m_socket;
	REPEATER_TYPE    m_type;
	unsigned char*   m_buffer;
	unsigned int     m_length;
	in_addr          m_address;
	unsigned int     m_port;

	bool readPackets();
};

#endif

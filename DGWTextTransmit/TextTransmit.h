/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
 *   Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#ifndef	TextTransmit_H
#define	TextTransmit_H

#include <string>

#include "UDPReaderWriter.h"
#include "HeaderData.h"
#include "AMBEData.h"

bool parseCLIArgs(int argc, const char * argv[], std::string& repeater, std::string& text, std::string& file);

class CTextTransmit {
public:
	CTextTransmit(const std::string& callsign, const std::string& text);
	~CTextTransmit();

	bool run();

private:
	CUDPReaderWriter m_socket;
	std::string         m_callsign;
	std::string         m_text;

	bool sendHeader(const CHeaderData& header);
	bool sendData(const CAMBEData& data);
};

#endif

/*
 *   Copyright (C) 2010,2012 by Jonathan Naylor G4KLX
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

#include "G2ProtocolHandler.h"
#include "DStarDefines.h"
#include "HeaderData.h"
#include "AMBEData.h"
#include "Timer.h"

class CG2Handler {
public:
	static void initialise(unsigned int maxRoutes);

	static void setG2ProtocolHandler(CG2ProtocolHandler* handler);

	static void process(CHeaderData& header);
	static void process(CAMBEData& header);

	static void clock(unsigned int ms);

	static void finalise();

protected:
	CG2Handler(const in_addr& address, unsigned int id);
	~CG2Handler();

	bool clockInt(unsigned int ms);

private:
	static unsigned int        m_maxRoutes;
	static CG2Handler**        m_routes;

	static CG2ProtocolHandler* m_handler;

	in_addr           m_address;
	unsigned int      m_id;
	CTimer            m_inactivityTimer;
};

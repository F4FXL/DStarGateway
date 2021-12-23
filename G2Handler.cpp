/*
 *   Copyright (C) 2010-2014 by Jonathan Naylor G4KLX
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

#include <cassert>

#include "GroupHandler.h"
#include "G2Handler.h"
#include "Utils.h"
#include "Defs.h"

unsigned int        CG2Handler::m_maxRoutes = 0U;
CG2Handler**        CG2Handler::m_routes = NULL;

CG2ProtocolHandler* CG2Handler::m_handler = NULL;

CG2Handler::CG2Handler(const in_addr& address, unsigned int id) :
m_address(address),
m_id(id),
m_inactivityTimer(1000U, NETWORK_TIMEOUT)
{
	m_inactivityTimer.start();
}

CG2Handler::~CG2Handler()
{
}

void CG2Handler::initialise(unsigned int maxRoutes)
{
	m_maxRoutes = maxRoutes;

	if (maxRoutes == 0U)
		return;

	m_routes = new CG2Handler*[m_maxRoutes];
	for (unsigned int i = 0U; i < m_maxRoutes; i++)
		m_routes[i] = NULL;
}

void CG2Handler::setG2ProtocolHandler(CG2ProtocolHandler* handler)
{
	assert(handler != NULL);

	m_handler = handler;
}

void CG2Handler::process(CHeaderData& header)
{
	// Is this a busy reply?
	unsigned char flag1 = header.getFlag1();
	if (flag1 == 0x01) {
		// Don't check the incoming stream
		// printf("G2 busy message received\n"));
		return;
	}

	// Check to see if this is for Smart Group
	CGroupHandler* handler = CGroupHandler::findGroup(header);
	if (handler != NULL) {
		handler->process(header);
		return;
	}
}

void CG2Handler::process(CAMBEData& data)
{
	// Check to see if this is for Smart Group
	CGroupHandler* handler = CGroupHandler::findGroup(data);
	if (handler != NULL) {
		handler->process(data);
		return;
	}
}

void CG2Handler::clock(unsigned int ms)
{
	for (unsigned int i = 0U; i < m_maxRoutes; i++) {
		CG2Handler* route = m_routes[i];
		if (route != NULL) {
			bool ret = route->clockInt(ms);
			if (ret) {
				delete route;
				m_routes[i] = NULL;
			}
		}
	}
}

void CG2Handler::finalise()
{
	for (unsigned int i = 0U; i < m_maxRoutes; i++)
		delete m_routes[i];

	delete[] m_routes;
}

bool CG2Handler::clockInt(unsigned int ms)
{
	m_inactivityTimer.clock(ms);

	if (m_inactivityTimer.isRunning() && m_inactivityTimer.hasExpired()) {
		printf("Inactivity timeout for a G2 route has expired\n");
		return true;
	}

	return false;
}

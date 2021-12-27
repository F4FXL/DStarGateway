/*
 *   Copyright (C) 2010,2012 by Jonathan Naylor G4KLX
 *   copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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
#include <vector>

#include "RepeaterHandler.h"
#ifdef USE_STARNET
#include "StarNetHandler.h"
#endif
#include "RemoteHandler.h"
#include "DExtraHandler.h"
#include "DPlusHandler.h"
#include "DStarDefines.h"
#include "DCSHandler.h"
#include "Log.h"

CRemoteHandler::CRemoteHandler(const std::string& password, unsigned int port, const std::string& address) :
m_password(password),
m_handler(port, address),
m_random(0U)
{
	assert(port > 0U);
	assert(!password.empty());
}

CRemoteHandler::~CRemoteHandler()
{
}

bool CRemoteHandler::open()
{
	return m_handler.open();
}

void CRemoteHandler::process()
{
	RPH_TYPE type = m_handler.readType();
	switch (type) {
		case RPHT_LOGOUT:
			m_handler.setLoggedIn(false);
			CLog::logInfo("Remote control user has logged out");
			break;
		case RPHT_LOGIN:
			m_random = ::rand();
			m_handler.sendRandom(m_random);
			break;
		case RPHT_HASH: {
				bool valid = m_handler.readHash(m_password, m_random);
				if (valid) {
					CLog::logInfo("Remote control user has logged in");
					m_handler.setLoggedIn(true);
					m_handler.sendACK();
				} else {
					CLog::logInfo("Remote control user has failed login authentication");
					m_handler.setLoggedIn(false);
					m_handler.sendNAK("Invalid password");
				}
			}
			break;
		case RPHT_CALLSIGNS:
			sendCallsigns();
			break;
		case RPHT_REPEATER: {
				std::string callsign = m_handler.readRepeater();
				sendRepeater(callsign);
			}
			break;
#ifdef USE_STARNET
		case RPHT_STARNET: {
				std::string callsign = m_handler.readStarNetGroup();
				sendStarNetGroup(callsign);
			}
			break;
#endif
		case RPHT_LINK: {
				std::string callsign, reflector;
				RECONNECT reconnect;
				m_handler.readLink(callsign, reconnect, reflector);
				if (reflector.empty())
					CLog::logInfo("Remote control user has linked \"%s\" to \"None\" with reconnect %d", callsign.c_str(), int(reconnect));
				else
					CLog::logInfo("Remote control user has linked \"%s\" to \"%s\" with reconnect %d", callsign.c_str(), reflector.c_str(), int(reconnect));
				link(callsign, reconnect, reflector, true);
			}
			break;
		case RPHT_UNLINK: {
				std::string callsign, reflector;
				PROTOCOL protocol;
				m_handler.readUnlink(callsign, protocol, reflector);
				CLog::logInfo("Remote control user has unlinked \"%s\" from \"%s\" for protocol %d", callsign.c_str(), reflector.c_str(), int(protocol));
				unlink(callsign, protocol, reflector);
			}
			break;
		case RPHT_LINKSCR: {
				std::string callsign, reflector;
				RECONNECT reconnect;
				m_handler.readLinkScr(callsign, reconnect, reflector);
				if (reflector.empty())
					CLog::logInfo("Remote control user has linked \"%s\" to \"None\" with reconnect %d from localhost", callsign.c_str(), reconnect);
				else
					CLog::logInfo("Remote control user has linked \"%s\" to \"%s\" with reconnect %d from localhost", callsign.c_str(), reflector.c_str(), reconnect);
				link(callsign, reconnect, reflector, false);
			}
			break;
#ifdef USE_STARNET
		case RPHT_LOGOFF: {
				std::string callsign, user;
				m_handler.readLogoff(callsign, user);
				CLog::logInfo("Remote control user has logged off \"%s\" from \"%s\"", user.c_str(), callsign.c_str());
				logoff(callsign, user);
			}
			break;
#endif
		default:
			break;
	}
}

void CRemoteHandler::close()
{
	m_handler.close();
}

void CRemoteHandler::sendCallsigns()
{
	std::vector<std::string> repeaters = CRepeaterHandler::listDVRepeaters();
#if USE_STARNET
	std::vector<std::string>  starNets  = CStarNetHandler::listStarNets();
#else
	std::vector<std::string>  starNets;
#endif

	m_handler.sendCallsigns(repeaters, starNets);
}

void CRemoteHandler::sendRepeater(const std::string& callsign)
{
	CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(callsign);
	if (repeater == NULL) {
		m_handler.sendNAK("Invalid repeater callsign");
		return;
	}

	CRemoteRepeaterData* data = repeater->getInfo();
	if (data != NULL) {
		CDExtraHandler::getInfo(repeater, *data);
		CDPlusHandler::getInfo(repeater, *data);
		CDCSHandler::getInfo(repeater, *data);
#ifdef USE_CCS
		CCCSHandler::getInfo(repeater, *data);
#endif

		m_handler.sendRepeater(*data);
	}

	delete data;
}

#ifdef USE_STARNET
void CRemoteHandler::sendStarNetGroup(const std::string& callsign)
{
	CStarNetHandler* starNet = CStarNetHandler::findStarNet(callsign);
	if (starNet == NULL) {
		m_handler.sendNAK("Invalid STARnet Group callsign");
		return;
	}

	CRemoteStarNetGroup* data = starNet->getInfo();
	if (data != NULL)
		m_handler.sendStarNetGroup(*data);

	delete data;
}
#endif

void CRemoteHandler::link(const std::string& callsign, RECONNECT reconnect, const std::string& reflector, bool respond)
{
	CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(callsign);
	if (repeater == NULL) {
		m_handler.sendNAK("Invalid repeater callsign");
		return;
	}

	repeater->link(reconnect, reflector);

	if (respond)
	    m_handler.sendACK();
}

void CRemoteHandler::unlink(const std::string& callsign, PROTOCOL protocol, const std::string& reflector)
{
	CRepeaterHandler* repeater = CRepeaterHandler::findDVRepeater(callsign);
	if (repeater == NULL) {
		m_handler.sendNAK("Invalid repeater callsign");
		return;
	}

	repeater->unlink(protocol, reflector);

    m_handler.sendACK();
}

#if USE_STARNET
void CRemoteHandler::logoff(const std::string& callsign, const std::string& user)
{
	CStarNetHandler* starNet = CStarNetHandler::findStarNet(callsign);
	if (starNet == NULL) {
		m_handler.sendNAK("Invalid STARnet group callsign");
		return;
	}

	bool res = starNet->logoff(user);
	if (!res)
		m_handler.sendNAK("Invalid STARnet user callsign");
	else
		m_handler.sendACK();
}
#endif

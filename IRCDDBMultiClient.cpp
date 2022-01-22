/*

CIRCDDBClient - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2011,2012   Jonathan Naylor, G4KLX
Copyright (c) 2021 by Thomas Geoffrey Merck F4FXL / KC3FRA

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#include <stdio.h>

#include "IRCDDBMultiClient.h"
#include "Log.h"

CIRCDDBMultiClient::CIRCDDBMultiClient(const CIRCDDB_Array& clients) :
m_clients(),
m_queriesLock(),
m_responseQueueLock()
{
	for (unsigned int i = 0; i < clients.size(); i++)	{
		if (clients[i] != NULL)
			m_clients.push_back(clients[i]);
	}
	m_clients.shrink_to_fit();
}

CIRCDDBMultiClient::~CIRCDDBMultiClient()
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		delete m_clients[i];
	}

	while (m_responseQueue.size() > 0) {
		delete m_responseQueue[0];
		m_responseQueue.erase(m_responseQueue.begin());
	}

	for (CIRCDDBMultiClientQuery_HashMap::iterator it = m_userQueries.begin(); it != m_userQueries.end(); it++)
		delete it->second;
	m_userQueries.clear();

	for (CIRCDDBMultiClientQuery_HashMap::iterator it = m_repeaterQueries.begin(); it != m_repeaterQueries.end(); it++)
		delete it->second;
	m_repeaterQueries.clear();

	for (CIRCDDBMultiClientQuery_HashMap::iterator it = m_gatewayQueries.begin(); it != m_gatewayQueries.end(); it++)
		delete it->second;
	m_gatewayQueries.clear();
}

bool CIRCDDBMultiClient::open()
{
	bool result = true;

	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->open() && result;
	}

	if (!result) close();

	return result;
}

void CIRCDDBMultiClient::rptrQTH(const std::string & callsign, double latitude, double longitude, const std::string & desc1, const std::string & desc2, const std::string & infoURL)
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		m_clients[i]->rptrQTH(callsign, latitude, longitude, desc1, desc2, infoURL);
	}
}

void CIRCDDBMultiClient::rptrQRG(const std::string & callsign, double txFrequency, double duplexShift, double range, double agl)
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		m_clients[i]->rptrQRG(callsign, txFrequency, duplexShift, range, agl);
	}
}

void CIRCDDBMultiClient::kickWatchdog(const std::string & callsign, const std::string & wdInfo)
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		m_clients[i]->kickWatchdog(callsign, wdInfo);
	}
}

int CIRCDDBMultiClient::getConnectionState()
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		int state = m_clients[i]->getConnectionState();
		if (state != 7)
			return state;
	}

	return 7;
}

bool CIRCDDBMultiClient::sendHeard(const std::string & myCall, const std::string & myCallExt, const std::string & yourCall, const std::string & rpt1, const std::string & rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3)
{
	bool result = true;

	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->sendHeard(myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::sendHeardWithTXMsg(const std::string & myCall, const std::string & myCallExt, const std::string & yourCall, const std::string & rpt1, const std::string & rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, const std::string & network_destination, const std::string & tx_message)
{
	bool result = true;

	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->sendHeardWithTXMsg(myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3, network_destination, tx_message) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::sendHeardWithTXStats(const std::string & myCall, const std::string & myCallExt, const std::string & yourCall, const std::string & rpt1, const std::string & rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, int num_dv_frames, int num_dv_silent_frames, int num_bit_errors)
{
	bool result = true;

	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->sendHeardWithTXStats(myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3, num_dv_frames, num_dv_silent_frames, num_bit_errors) && result;
	}

	return result;
}

void CIRCDDBMultiClient::sendDStarGatewayInfo(const std::string subcommand, const std::vector<std::string> parms)
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		m_clients[i]->sendDStarGatewayInfo(subcommand, parms);
	}
}

bool CIRCDDBMultiClient::findGateway(const std::string & gatewayCallsign)
{
	pushQuery(IDRT_GATEWAY, gatewayCallsign, new CIRCDDBMultiClientQuery("", "", gatewayCallsign, "", "", "", IDRT_GATEWAY));
	bool result = true;
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->findGateway(gatewayCallsign) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::findRepeater(const std::string & repeaterCallsign)
{
	pushQuery(IDRT_REPEATER, repeaterCallsign, new CIRCDDBMultiClientQuery("", repeaterCallsign, "", "", "", "", IDRT_REPEATER));
	bool result = true;
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->findRepeater(repeaterCallsign) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::findUser(const std::string & userCallsign)
{
	pushQuery(IDRT_USER, userCallsign, new CIRCDDBMultiClientQuery(userCallsign, "", "", "", "", "", IDRT_USER));
	bool result = true;
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->findUser(userCallsign) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::notifyRepeaterG2NatTraversal(const std::string& repeater)
{
	// NAT traversal message does not expect a response over IRC
	bool result = true;
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->notifyRepeaterG2NatTraversal(repeater) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::notifyRepeaterDextraNatTraversal(const std::string& repeater, unsigned int myPort)
{
	// NAT traversal message does not expect a response over IRC
	bool result = true;
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->notifyRepeaterDextraNatTraversal(repeater, myPort) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::notifyRepeaterDPlusNatTraversal(const std::string& repeater, unsigned int myPort)
{
	// NAT traversal message does not expect a response over IRC
	bool result = true;
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		result = m_clients[i]->notifyRepeaterDPlusNatTraversal(repeater, myPort) && result;
	}

	return result;
}

bool CIRCDDBMultiClient::receiveNATTraversalG2(std::string& address)
{
	CIRCDDBMultiClientQuery * item = checkAndGetNextResponse(IDRT_NATTRAVERSAL_G2, "CIRCDDBMultiClient::receiveNATTraversalG2: unexpected response type");
	if (item == NULL)
		return false;

	address = item->getAddress();

	return true;
}

bool CIRCDDBMultiClient::receiveNATTraversalDextra(std::string& address, std::string& remotePort)
{
	CIRCDDBMultiClientQuery * item = checkAndGetNextResponse(IDRT_NATTRAVERSAL_DEXTRA, "CIRCDDBMultiClient::receiveNATTraversalDextra: unexpected response type");
	if (item == NULL)
		return false;

	address = item->getAddress();
	remotePort = item->getRemotePort();

	return true;
}

bool CIRCDDBMultiClient::receiveNATTraversalDPlus(std::string& address, std::string& remotePort)
{
	CIRCDDBMultiClientQuery * item = checkAndGetNextResponse(IDRT_NATTRAVERSAL_DPLUS, "CIRCDDBMultiClient::receiveNATTraversalDextra: unexpected response type");
	if (item == NULL)
		return false;

	address = item->getAddress();
	remotePort = item->getRemotePort();

	return true;
}

IRCDDB_RESPONSE_TYPE CIRCDDBMultiClient::getMessageType()
{
	//process the inner clients at each call
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		std::string user = "", repeater = "", gateway = "", address = "", timestamp = "", key = "", port ="";

		IRCDDB_RESPONSE_TYPE type = m_clients[i]->getMessageType();

		switch (type) {
			case IDRT_USER: {
				if (!m_clients[i]->receiveUser(user, repeater, gateway, address, timestamp))
					type = IDRT_NONE;
				key = user;
				break;
			}
			case IDRT_GATEWAY: {
				if (!m_clients[i]->receiveGateway(gateway, address))
					type = IDRT_NONE;
				key = gateway;
				break;
			}
			case IDRT_REPEATER: {
				if (!m_clients[i]->receiveRepeater(repeater, gateway, address))
					type = IDRT_NONE;
				key = repeater;
				break;
			}
			case IDRT_NATTRAVERSAL_G2: {
				if (!m_clients[i]->receiveNATTraversalG2(address))
					type = IDRT_NATTRAVERSAL_G2;
				key = "NAT_TRAVERSAL_G2";
				break;
			}
			case IDRT_NATTRAVERSAL_DEXTRA: {
				if (!m_clients[i]->receiveNATTraversalDextra(address, port))
					type = IDRT_NATTRAVERSAL_DEXTRA;
				key = "NAT_TRAVERSAL_DEXTRA";
				break;
			}
			case IDRT_NATTRAVERSAL_DPLUS: {
				if (!m_clients[i]->receiveNATTraversalDPlus(address, port))
					type = IDRT_NATTRAVERSAL_DEXTRA;
				key = "NAT_TRAVERSAL_DPLUS";
				break;
			}
			case IDRT_NONE: {
			default:
				break;
			}
		}

		if (type != IDRT_NONE)
		{
			m_queriesLock.lock();

			bool canAddToQueue = false;
			bool wasQuery = false;
			CIRCDDBMultiClientQuery * item = popQuery(type, key);

			if (item != NULL) {//is this a response to a query we've sent ?
				item->Update(user, repeater, gateway, address, timestamp, port);//update item (if needed)
				canAddToQueue = (item->incrementResponseCount() >= m_clients.size()); //did all the clients respond or did we have an answer ?
				wasQuery = true;
			}
			else {
				item = new CIRCDDBMultiClientQuery(user, repeater, gateway, address, timestamp, port, type);
				canAddToQueue = true;
			}

			if (canAddToQueue) {
				m_responseQueueLock.lock();
				m_responseQueue.push_back(item);
				m_responseQueueLock.unlock();
			}
			else if (wasQuery)
				pushQuery(type, key, item);

			m_queriesLock.unlock();
		}
	}

    IRCDDB_RESPONSE_TYPE result = IDRT_NONE;

	m_responseQueueLock.lock();
	if (m_responseQueue.size() != 0) result = m_responseQueue[0]->getType();
	m_responseQueueLock.unlock();

	return result;
}

bool CIRCDDBMultiClient::receiveRepeater(std::string & repeaterCallsign, std::string & gatewayCallsign, std::string & address)
{
	CIRCDDBMultiClientQuery * item = checkAndGetNextResponse(IDRT_REPEATER, "CIRCDDBMultiClient::receiveRepeater: unexpected response type");
	if (item == NULL)
		return false;

	repeaterCallsign = item->getRepeater();
	gatewayCallsign = item->getGateway();
	address = item->getAddress();
	delete item;
	return true;
}

bool CIRCDDBMultiClient::receiveGateway(std::string & gatewayCallsign, std::string & address)
{
	CIRCDDBMultiClientQuery * item = checkAndGetNextResponse(IDRT_GATEWAY, "CIRCDDBMultiClient::receiveGateway: unexpected response type");
	if (item == NULL)
		return false;

	gatewayCallsign = item->getGateway();
	address = item->getAddress();
	delete item;
	return true;
}

bool CIRCDDBMultiClient::receiveUser(std::string & userCallsign, std::string & repeaterCallsign, std::string & gatewayCallsign, std::string & address)
{
	std::string dummy;
	return receiveUser(userCallsign, repeaterCallsign, gatewayCallsign, address, dummy);
}

bool CIRCDDBMultiClient::receiveUser(std::string & userCallsign, std::string & repeaterCallsign, std::string & gatewayCallsign, std::string & address, std::string & timeStamp)
{
	CIRCDDBMultiClientQuery * item = checkAndGetNextResponse(IDRT_USER, "CIRCDDBMultiClient::receiveUser: unexpected response type");
	if (item == NULL) {
		//CLog::logInfo("CIRCDDBMultiClient::receiveUser NO USER IN QUEUE"));
		return false;
	}

	//CLog::logInfo("CIRCDDBMultiClient::receiveUser : %s", item->toString());

	userCallsign = item->getUser();
	repeaterCallsign = item->getRepeater();
	gatewayCallsign = item->getGateway();
	address = item->getAddress();
	timeStamp = item->getTimestamp();
	delete item;
	return true;
}

void CIRCDDBMultiClient::close()
{
	for (unsigned int i = 0; i < m_clients.size(); i++) {
		m_clients[i]->close();
	}
}

CIRCDDBMultiClientQuery * CIRCDDBMultiClient::checkAndGetNextResponse(IRCDDB_RESPONSE_TYPE expectedType, std::string errorMessage)
{
	CIRCDDBMultiClientQuery * item = NULL;
	m_responseQueueLock.lock();

	if (m_responseQueue.size() == 0 || m_responseQueue[0]->getType() != expectedType) {
		CLog::logInfo(errorMessage.c_str());
	}
	else {
		item = m_responseQueue[0];
		m_responseQueue.erase(m_responseQueue.begin());
	}
	m_responseQueueLock.unlock();
	return item;
}

void CIRCDDBMultiClient::pushQuery(IRCDDB_RESPONSE_TYPE type, const std::string& key, CIRCDDBMultiClientQuery * query)
{
	CIRCDDBMultiClientQuery_HashMap * queries = getQueriesHashMap(type);
	m_queriesLock.lock();

	if (queries != NULL && (*queries)[key] == NULL)
		(*queries)[key] = query;
	else
		delete query;

	m_queriesLock.unlock();
}


CIRCDDBMultiClientQuery * CIRCDDBMultiClient::popQuery(IRCDDB_RESPONSE_TYPE type, const std::string & key)
{
	CIRCDDBMultiClientQuery_HashMap * queries = getQueriesHashMap(type);
	m_queriesLock.lock();

	CIRCDDBMultiClientQuery * item = NULL;
	if (queries != NULL && queries->count(key) != 0) {
		item = queries->at(key);
		queries->erase(key);
	}

	m_queriesLock.unlock();
	return item;
}

CIRCDDBMultiClientQuery_HashMap * CIRCDDBMultiClient::getQueriesHashMap(IRCDDB_RESPONSE_TYPE type)
{
	switch (type)
	{
	case IDRT_USER:
		return &m_userQueries;
	case IDRT_GATEWAY:
		return &m_gatewayQueries;
	case IDRT_REPEATER:
		return &m_repeaterQueries;
	case IDRT_NONE:
	default:
		return NULL;
	}
}

/*

CIRCDDB - ircDDB client library in C++

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


#pragma once

#include "IRCDDB.h"
#include <string>
#include <vector>
#include <map>
#include <sstream> 
#include <mutex>

//Small data container to keep track of queries with sent to the inner clients
class CIRCDDBMultiClientQuery
{
public:
	CIRCDDBMultiClientQuery(const std::string& user,
				const std::string& repeater,
				const std::string& gateway,
				const std::string& address,
				const std::string& timestamp,
				IRCDDB_RESPONSE_TYPE type) :
		m_user(user),
		m_repeater(repeater),
		m_gateway(gateway),
		m_address(address),
		m_timestamp(timestamp),
		m_type(type),
		m_responseCount(0)
	{

	}

	std::string getUser() const
	{
		return m_user;
	}

	std::string getRepeater() const
	{
		return m_repeater;
	}

	std::string getGateway() const
	{
		return m_gateway;
	}

	std::string getAddress() const
	{
		return m_address;
	}

	std::string getTimestamp() const
	{
		return m_timestamp;
	}

	unsigned int getResponseCount()
	{
		return m_responseCount;
	}

	unsigned int incrementResponseCount()
	{
		++m_responseCount;
		//wxLogMessage("Resp Count : %s %d"), toString(), m_responseCount);
		return m_responseCount;
	}

	/*
		Updates the entry, but only if the timestamp is newer. if an address was already specified it is kept.
	*/
	void Update(const std::string& user, const std::string& repeater, const std::string& gateway, const std::string& address, const std::string& timestamp)
	{
		//wxLogMessage("Before : %s"), toString());
		if (timestamp.empty() || timestamp.compare(m_timestamp) >= 0) {
			m_user = user;
			m_repeater = repeater;
			m_gateway = gateway;
			m_timestamp = timestamp;

			if(m_address.empty() && !address.empty())
				m_address = address;
		}
		//wxLogMessage("After : %s"), toString());
	}

	IRCDDB_RESPONSE_TYPE getType()
	{
		return m_type;
	}

	std::string toString()
	{
		std::stringstream strStream;
		strStream << m_user << " " << m_repeater << " " << m_gateway << " " << m_address << " " << m_timestamp;
		return strStream.str();
	}

private:
	std::string m_user;
	std::string m_repeater;
	std::string m_gateway;
	std::string m_address;
	std::string m_timestamp;
	IRCDDB_RESPONSE_TYPE m_type;
	unsigned int m_responseCount;
};

typedef std::map<std::string, CIRCDDBMultiClientQuery*> CIRCDDBMultiClientQuery_HashMap;
typedef std::vector<CIRCDDBMultiClientQuery*> CIRCDDBMultiClientQuery_Array;

class CIRCDDBMultiClient : public CIRCDDB
{
public:
	CIRCDDBMultiClient(const CIRCDDB_Array& clients);
	~CIRCDDBMultiClient();

	// Inherited via CIRCDDB
	virtual bool open();
	virtual void rptrQTH(const std::string & callsign, double latitude, double longitude, const std::string & desc1, const std::string & desc2, const std::string & infoURL);
	virtual void rptrQRG(const std::string & callsign, double txFrequency, double duplexShift, double range, double agl);
	virtual void kickWatchdog(const std::string & callsign, const std::string & wdInfo);
	virtual int getConnectionState() ;
	virtual bool sendHeard(const std::string & myCall, const std::string & myCallExt, const std::string & yourCall, const std::string & rpt1, const std::string & rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3);
	virtual bool sendHeardWithTXMsg(const std::string & myCall, const std::string & myCallExt, const std::string & yourCall, const std::string & rpt1, const std::string & rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, const std::string & network_destination, const std::string & tx_message);
	virtual bool sendHeardWithTXStats(const std::string & myCall, const std::string & myCallExt, const std::string & yourCall, const std::string & rpt1, const std::string & rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, int num_dv_frames, int num_dv_silent_frames, int num_bit_errors);
	virtual bool findGateway(const std::string & gatewayCallsign);
	virtual bool findRepeater(const std::string & repeaterCallsign);
	virtual bool findUser(const std::string & userCallsign);
	virtual bool notifyRepeaterNatTraversal(const std::string& repeater);
	virtual IRCDDB_RESPONSE_TYPE getMessageType();
	virtual bool receiveRepeater(std::string & repeaterCallsign, std::string & gatewayCallsign, std::string & address);
	virtual bool receiveGateway(std::string & gatewayCallsign, std::string & address);
	virtual bool receiveUser(std::string & userCallsign, std::string & repeaterCallsign, std::string & gatewayCallsign, std::string & address);
	virtual bool receiveUser(std::string & userCallsign, std::string & repeaterCallsign, std::string & gatewayCallsign, std::string & address, std::string & timeStamp);
	virtual void sendDStarGatewayInfo(const std::string subcommand, const std::vector<std::string> parms);
	virtual void close();

	//

private :
	CIRCDDB_Array m_clients;
	std::recursive_mutex m_queriesLock;
	std::recursive_mutex m_responseQueueLock;

	CIRCDDBMultiClientQuery_HashMap m_userQueries;
	CIRCDDBMultiClientQuery_HashMap m_repeaterQueries;
	CIRCDDBMultiClientQuery_HashMap m_gatewayQueries;
	CIRCDDBMultiClientQuery_Array m_responseQueue;

	CIRCDDBMultiClientQuery * checkAndGetNextResponse(IRCDDB_RESPONSE_TYPE expectedType, std::string errorMessage);
	void pushQuery(IRCDDB_RESPONSE_TYPE type, const std::string& key,  CIRCDDBMultiClientQuery * query);
	CIRCDDBMultiClientQuery * popQuery(IRCDDB_RESPONSE_TYPE type, const std::string& key);
	CIRCDDBMultiClientQuery_HashMap * getQueriesHashMap(IRCDDB_RESPONSE_TYPE type);
};


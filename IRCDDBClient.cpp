/*
CIRCDDBClient - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2011,2012   Jonathan Naylor, G4KLX
Copyright (c) 2017 by Thomas A. Early

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

#include "IRCDDBClient.h"
#include "IRCClient.h"
#include "IRCDDBApp.h"
#include "Utils.h"
#include "Log.h"

struct CIRCDDBClientPrivate
{
	IRCClient *client;
	IRCDDBApp *app;
};

CIRCDDBClient::CIRCDDBClient(const std::string& hostName, unsigned int port, const std::string& callsign, const std::string& password, const std::string& versionInfo, const std::string& localAddr, bool isQuadNet ) :
d(new CIRCDDBClientPrivate),
m_isQuadNet(isQuadNet)
{
	std::string update_channel("#dstar");
	d->app = new IRCDDBApp(update_channel);
	d->client = new IRCClient(d->app, update_channel, hostName, port, callsign, password, versionInfo, localAddr);
}

CIRCDDBClient::~CIRCDDBClient()
{
	delete d->client;
	delete d->app;
	delete d;
}


	// A false return implies a network error, or unable to log in
bool CIRCDDBClient::open()
{
	CLog::logInfo("start client and app\n");
	d->client->startWork();
	d->app->startWork();
	return true;
}


int CIRCDDBClient::getConnectionState()
{
	return d->app->getConnectionState();
}


void CIRCDDBClient::rptrQTH(const std::string& callsign, double latitude, double longitude, const std::string& desc1, const std::string& desc2, const std::string& infoURL)
{
	d->app->rptrQTH(callsign, latitude, longitude, desc1, desc2, infoURL);
}


void CIRCDDBClient::rptrQRG(const std::string& callsign, double txFrequency, double duplexShift, double range, double agl)
{
	d->app->rptrQRG(callsign, txFrequency, duplexShift, range, agl);
}


void CIRCDDBClient::kickWatchdog(const std::string& callsign, const std::string& wdInfo)
{
	d->app->kickWatchdog(callsign, wdInfo);
}



// Send heard data, a false return implies a network error
bool CIRCDDBClient::sendHeard( const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1,
									const std::string& rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3 )
{
	if (myCall.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeard:myCall='%s' len != 8\n", myCall.c_str());
		return false;
	}

	if (myCallExt.size() != 4) {
		CLog::logDebug("CIRCDDBClient::sendHeard:myCallExt='%s' len != 4\n", myCallExt.c_str());
		return false;
	}

	if (yourCall.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeard:yourCall='%s' len != 8\n", yourCall.c_str());
		return false;
	}

	if (rpt1.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeard:rpt1='%s' len != 8\n", rpt1.c_str());
		return false;
	}

	if (rpt2.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeard:rpt2='%s' len != 8\n", rpt2.c_str());
		return false;
	}

	return d->app->sendHeard(myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3, std::string("        "), std::string(""), std::string(""));
}

void CIRCDDBClient::sendDStarGatewayInfo(const std::string subcommand, const std::vector<std::string> parms)
{
	CLog::logDebug("CIRCDDBClient::sendDStarGatewayInfo subcommand %s parms", subcommand.c_str());
	for(unsigned int i=0; i < parms.size();i++)
		CLog::logInfo(" %s", parms[i].c_str());
	CLog::logInfo("\n");

	if(m_isQuadNet) {
		d->app->sendDStarGatewayInfo(subcommand, parms);
	}
}

// Send heard data, a false return implies a network error
bool CIRCDDBClient::sendHeardWithTXMsg(const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1,
	const std::string& rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, const std::string& network_destination, const std::string& tx_message)
{
	if (myCall.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXMsg:myCall='%s' len != 8\n", myCall.c_str());
		return false;
	}

	if (myCallExt.size() != 4) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXMsg:myCallExt='%s' len != 4\n", myCallExt.c_str());
		return false;
	}

	if (yourCall.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXMsg:yourCall='%s' len != 8\n", yourCall.c_str());
		return false;
	}

	if (rpt1.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXMsg:rpt1='%s' len != 8\n", rpt1.c_str());
		return false;
	}

	if (rpt2.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXMsg:rpt2='%s' len != 8\n", rpt2.c_str());
		return false;
	}

	std::string dest(network_destination);
	if (0 == dest.size())
		dest = std::string("        ");

	if (8 != dest.size()) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXMsg:network_destination='%s' len != 8\n", dest.c_str());
		return false;
	}

	std::string msg;
	if (20 == tx_message.size()) {
		for (unsigned int i=0; i < tx_message.size(); i++) {
			char ch = tx_message.at(i);
			if ((ch > 32) && (ch < 127))
				msg.push_back(ch);
			else
				msg.push_back('_');
		}
	}
	return d->app->sendHeard(myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3, dest, msg, std::string(""));
}



bool CIRCDDBClient::sendHeardWithTXStats( const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1,
	const std::string& rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, int num_dv_frames, int num_dv_silent_frames, int num_bit_errors)
{
	if ((num_dv_frames <= 0) || (num_dv_frames > 65535)) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:num_dv_frames=%d not in range 1-65535\n", num_dv_frames);
		return false;
	}
	
	if (num_dv_silent_frames > num_dv_frames) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:num_dv_silent_frames=%d > num_dv_frames=%d\n", num_dv_silent_frames, num_dv_frames);
		return false;
	}
	
	if (num_bit_errors > (4*num_dv_frames)) { // max 4 bit errors per frame
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:num_bit_errors > (4*num_dv_frames), %d > 4*%d\n", num_bit_errors, num_dv_frames);
		return false;
	}
	
	if (myCall.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:myCall='%s' len != 8\n", myCall.c_str());
		return false;
	}
	
	if (myCallExt.size() != 4) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:myCallExt='%s' len != 4\n", myCallExt.c_str());
		return false;
	}
	
	if (yourCall.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:yourCall='%s' len != 8\n", yourCall.c_str());
		return false;
	}
	
	if (rpt1.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:rpt1='%s' len != 8\n", rpt1.c_str());
		return false;
	}
	
	if (rpt2.size() != 8) {
		CLog::logDebug("CIRCDDBClient::sendHeardWithTXStats:rpt2='%s' len != 8\n", rpt2.c_str());
		return false;
	}

	char str[10];
	snprintf(str, 10, "%04x", num_dv_frames);
	std::string stats(str);

	if (num_dv_silent_frames >= 0) {
		snprintf(str, 10, "%02x", (num_dv_silent_frames * 100) / num_dv_frames);
		stats.append(str);

		if (num_bit_errors >= 0) {
			snprintf(str, 10, "%02x", (num_bit_errors * 125) / (num_dv_frames * 3));
			stats.append(str);
		}
	}
	stats.resize(20, '_');

	return d->app->sendHeard(myCall, myCallExt, yourCall, rpt1, rpt2, flag1, flag2, flag3, std::string("        "), std::string(""), stats);
}

// Send query for a gateway/reflector, a false return implies a network error
bool CIRCDDBClient::findGateway(const std::string& gatewayCallsign)
{
	if (8 != gatewayCallsign.size()) {
		CLog::logDebug("CIRCDDBClient::findGateway:gatewayCallsign='%s' len != 8\n", gatewayCallsign.c_str());
		return false;
	}
	std::string gw(gatewayCallsign);
	CUtils::ToUpper(gw);
	return d->app->findGateway(gw);
}


bool CIRCDDBClient::findRepeater(const std::string& repeaterCallsign)
{
	if (8 != repeaterCallsign.size()) {
		CLog::logDebug("CIRCDDBClient::findRepeater:repeaterCallsign='%s' len != 8\n", repeaterCallsign.c_str());
		return false;
	}
	std::string rptr(repeaterCallsign);
	CUtils::ToUpper(rptr);
	return d->app->findRepeater(rptr);
}

// Send query for a user, a false return implies a network error
bool CIRCDDBClient::findUser(const std::string& userCallsign)
{
	if (8 != userCallsign.size()) {
		CLog::logDebug("CIRCDDBClient::findUser:userCall='%s' len != 8\n", userCallsign.c_str());
		return false;
	}
	CLog::logTrace("IRC Find user %s", userCallsign.c_str());
	std::string usr(userCallsign);
	CUtils::ToUpper(usr);
	return d->app->findUser(usr);
}

// The following functions are for processing received messages

// Get the waiting message type
IRCDDB_RESPONSE_TYPE CIRCDDBClient::getMessageType()
{
	return d->app->getReplyMessageType();
}

// Get a gateway message, as a result of IDRT_REPEATER returned from getMessageType()
// A false return implies a network error
bool CIRCDDBClient::receiveRepeater(std::string& repeaterCallsign, std::string& gatewayCallsign, std::string& address)
{
	IRCDDB_RESPONSE_TYPE rt = d->app->getReplyMessageType();

	if (rt != IDRT_REPEATER) {
		CLog::logDebug("CIRCDDBClient::receiveRepeater: unexpected response type=%d\n", rt);
		return false;
	}

	IRCMessage *m = d->app->getReplyMessage();
	if (m == NULL) {
		CLog::logDebug("CIRCDDBClient::receiveRepeater: no message\n");
		return false;
	}

	if (m->getCommand().compare("IDRT_REPEATER")) {
		CLog::logDebug("CIRCDDBClient::receiveRepeater: wrong message type, expected 'IDRT_REPEATER, got '%s'\n", m->getCommand().c_str());
		delete m;
		return false;
	}

	if (3 != m->getParamCount()) {
		CLog::logDebug("CIRCDDBClient::receiveRepeater: unexpected number of message parameters, expected 3, got %d\n", m->getParamCount());
		delete m;
		return false;
	}

	repeaterCallsign = m->getParam(0);
	gatewayCallsign = m->getParam(1);
	address = m->getParam(2);
	delete m;
	return true;
}

// Get a gateway message, as a result of IDRT_GATEWAY returned from getMessageType()
// A false return implies a network error
bool CIRCDDBClient::receiveGateway(std::string& gatewayCallsign, std::string& address)
{
	IRCDDB_RESPONSE_TYPE rt = d->app->getReplyMessageType();

	if (rt != IDRT_GATEWAY) {
		CLog::logDebug("CIRCDDBClient::receiveGateway: unexpected response type=%d\n", rt);
		return false;
	}

	IRCMessage *m = d->app->getReplyMessage();

	if (m == NULL) {
		CLog::logDebug("CIRCDDBClient::receiveGateway: no message\n");
		return false;
	}

	if (m->getCommand().compare("IDRT_GATEWAY")) {
		CLog::logDebug("CIRCDDBClient::receiveGateway: wrong message type, expected 'IDRT_GATEWAY' got '%s'\n", m->getCommand().c_str());
		delete m;
	return false;
	}

	if (2 != m->getParamCount()) {
		CLog::logDebug("CIRCDDBClient::receiveGateway: unexpected number of message parameters, expected 2, got %d\n", m->getParamCount());
		delete m;
	return false;
	}

	gatewayCallsign = m->getParam(0);
	address = m->getParam(1);
	delete m;
	return true;
}

// Get a user message, as a result of IDRT_USER returned from getMessageType()
// A false return implies a network error
bool CIRCDDBClient::receiveUser(std::string& userCallsign, std::string& repeaterCallsign, std::string& gatewayCallsign, std::string& address)
{
	std::string dummy("");
	return receiveUser(userCallsign, repeaterCallsign, gatewayCallsign, address, dummy);
}

bool CIRCDDBClient::receiveUser(std::string& userCallsign, std::string& repeaterCallsign, std::string& gatewayCallsign, std::string& address, std::string& timeStamp)
{
	IRCDDB_RESPONSE_TYPE rt = d->app->getReplyMessageType();

	if (rt != IDRT_USER) {
		CLog::logDebug("CIRCDDBClient::receiveUser: unexpected response type=%d\n", rt);
		return false;
	}

	IRCMessage * m = d->app->getReplyMessage();

	if (m == NULL) {
		CLog::logDebug("CIRCDDBClient::receiveUser: no message\n");
		return false;
	}

	if (m->getCommand().compare("IDRT_USER")) {
		CLog::logDebug("CIRCDDBClient::receiveUser: wrong message type, expected 'IDRT_USER', got '%s'\n", m->getCommand().c_str());
		delete m;
		return false;
	}

	if (5 != m->getParamCount()) {
		CLog::logDebug("CIRCDDBClient::receiveUser: unexpected number of message parameters, expected 5, got %d\n", m->getParamCount());
		delete m;
		return false;
	}

	userCallsign = m->getParam(0);
	repeaterCallsign = m->getParam(1);
	gatewayCallsign = m->getParam(2);
	address = m->getParam(3);
	timeStamp = m->getParam(4);

	CLog::logTrace("IRC Receive User %s %s %s %s %s", userCallsign.c_str(), repeaterCallsign.c_str(), gatewayCallsign.c_str(), address.c_str(), timeStamp.c_str());

	delete m;
	return true;
}

void CIRCDDBClient::close()		// Implictely kills any threads in the IRC code
{
	d->client -> stopWork();
	d->app -> stopWork();
}


void CIRCDDBClient::queryUsers()
{
}

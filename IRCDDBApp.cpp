/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2012        Jonathan Naylor, G4KLX
Copyright (c) 2017 by Thomas A. Early N7TAE
Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA

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

#include <netdb.h>
#include <map>
#include <mutex>
#include <regex>
#include <cstdio>
#include <chrono>
#include <thread>
#include <boost/algorithm/string.hpp>

#include "IRCDDBApp.h"
#include "Utils.h"
#include "Log.h"

class IRCDDBAppUserObject
{
public:
	std::string m_nick;
	std::string m_name;
	std::string m_host;
	bool m_op;
	unsigned int m_usn;

	IRCDDBAppUserObject()
	{
//		IRCDDBAppUserObject(std::string(""), std::string(""), std::string(""));
	}

	IRCDDBAppUserObject(const std::string& n, const std::string& nm, const std::string& h)
	{
		m_nick = n;
		m_name = nm;
		m_host = h;
		m_op = false;
		m_usn = 0;
	}
};

class IRCDDBAppRptrObject
{
public:
	std::string m_arearp_cs;
	time_t m_lastChanged;
	std::string m_zonerp_cs;

	IRCDDBAppRptrObject ()
	{
	}

	IRCDDBAppRptrObject (time_t &dt, std::string& repeaterCallsign, std::string& gatewayCallsign, time_t &maxTime)
	{
		m_arearp_cs = repeaterCallsign;
		m_lastChanged = dt;
		m_zonerp_cs = gatewayCallsign;

		if (dt > maxTime)
			maxTime = dt;
	}
};

class IRCDDBAppPrivate
{
public:
	IRCDDBAppPrivate()
	: m_tablePattern("^[0-9]$")
	, m_datePattern("^20[0-9][0-9]-((1[0-2])|(0[1-9]))-((3[01])|([12][0-9])|(0[1-9]))$")
	, m_timePattern("^((2[0-3])|([01][0-9])):[0-5][0-9]:[0-5][0-9]$")
	, m_dbPattern("^[0-9A-Z_]{8}$")
	, m_fromPattern("\\(from: (.*)\\)")
	{
	}

	int m_state;
	int m_timer;
	int m_infoTimer;
	int m_wdTimer;

	IRCMessageQueue *m_sendQ;
	IRCMessageQueue m_replyQ;

	std::string m_currentServer;
	std::string m_myNick;
	std::string m_updateChannel;
	std::string m_channelTopic;
	std::string m_bestServer;

	std::regex m_tablePattern;
	std::regex m_datePattern;
	std::regex m_timePattern;
	std::regex m_dbPattern;
	std::regex m_fromPattern;

	bool m_initReady;
	bool m_terminateThread;

	std::map<std::string, IRCDDBAppUserObject> m_userMap;
	std::mutex m_userMapMutex;

	std::map<std::string, IRCDDBAppRptrObject> m_rptrMap;
	std::mutex m_rptrMapMutex;

	std::map<std::string, std::string> m_moduleQRG;
	std::mutex m_moduleQRGMutex;

	std::map<std::string, std::string> m_moduleQTH;
	std::map<std::string, std::string> m_moduleURL;
	std::mutex m_moduleQTHURLMutex;

	std::map<std::string, std::string> m_moduleWD;
	std::mutex m_moduleWDMutex;
};

IRCDDBApp::IRCDDBApp(const std::string& u_chan)
	: m_d(new IRCDDBAppPrivate)
	, m_maxTime((time_t)time(0) - (time_t)(60 * 24 * 60 * 60)) // look 60 days in the past
{
	m_d->m_sendQ = NULL;
	m_d->m_initReady = false;

	userListReset();

	m_d->m_state = 0;
	m_d->m_timer = 0;
	m_d->m_myNick = std::string("none");

	m_d->m_updateChannel = u_chan;

	m_d->m_terminateThread = false;
}

IRCDDBApp::~IRCDDBApp()
{
	delete m_d->m_sendQ;
	delete m_d;
}

void IRCDDBApp::rptrQTH(const std::string& callsign, double latitude, double longitude, const std::string& desc1, const std::string& desc2, const std::string& infoURL)
{
	char pstr[32];
	snprintf(pstr, 32, "%+09.5f %+010.5f", latitude, longitude);
	std::string pos(pstr);

	std::string cs(callsign);
	std::string d1(desc1);
	std::string d2(desc2);

	d1.resize(20, '_');
	d2.resize(20, '_');

	std::regex nonValid("[^a-zA-Z0-9 +&(),./'-]");
	std::smatch sm;
	while (std::regex_search(d1, sm, nonValid))
		d1.erase(sm.position(0), sm.length());
	while (std::regex_search(d2, sm, nonValid))
		d2.erase(sm.position(0), sm.length());

	CUtils::ReplaceChar(pos, ',', '.');
	CUtils::ReplaceChar(d1, ' ', '_');
	CUtils::ReplaceChar(d2, ' ', '_');
	CUtils::ReplaceChar(cs, ' ', '_');

	std::lock_guard lochQTHURL(m_d->m_moduleQTHURLMutex);

	m_d->m_moduleQTH[cs] = cs + std::string(" ") + pos + std::string(" ") + d1 + std::string(" ") + d2;

	CLog::logInfo("QTH: %s\n", m_d->m_moduleQTH[cs].c_str());

	std::string url = infoURL;

	std::regex urlNonValid("[^[:graph:]]");
	while (std::regex_search(url, sm, urlNonValid))
		url.erase(sm.position(0), sm.length());

	if (url.size()) {
		m_d->m_moduleURL[cs] = cs + std::string(" ") + url;
		CLog::logInfo("URL: %s\n", m_d->m_moduleURL[cs].c_str());
	}

	m_d->m_infoTimer = 5; // send info in 5 seconds
}

void IRCDDBApp::rptrQRG(const std::string& callsign, double txFrequency, double duplexShift, double range, double agl)
{
	std::string cs = callsign;
	CUtils::ReplaceChar(cs, ' ', '_');

	char fstr[64];
	snprintf(fstr, 64, "%011.5f %+010.5f %06.2f %06.1f", txFrequency, duplexShift, range / 1609.344, agl);
	std::string f(fstr);
	CUtils::ReplaceChar(f, ',', '.');

	std::lock_guard lockModuleQRG(m_d->m_moduleQRGMutex);
	m_d->m_moduleQRG[cs] = cs + std::string(" ") + f;
	CLog::logInfo("QRG: %s\n", m_d->m_moduleQRG[cs].c_str());

	m_d->m_infoTimer = 5; // send info in 5 seconds
}

void IRCDDBApp::kickWatchdog(const std::string& callsign, const std::string& s)
{
	std::string text = s;

	std::regex nonValid("[^[:graph:]]");
	std::smatch sm;
	while (std::regex_search(text, sm, nonValid))
		text.erase(sm.position(0), sm.length());

	if (text.size()) {
		std::string cs = callsign;
		CUtils::ReplaceChar(cs, ' ', '_');

		std::lock_guard lockModuleWD(m_d->m_moduleWDMutex);
		m_d->m_moduleWD[cs] = cs + std::string(" ") + text;
		m_d->m_wdTimer = 60;
	}
}

int IRCDDBApp::getConnectionState()
{
	return m_d->m_state;
}

IRCDDB_RESPONSE_TYPE IRCDDBApp::getReplyMessageType()
{
	IRCMessage *m = m_d->m_replyQ.peekFirst();
	if (m == NULL)
		return IDRT_NONE;

	std::string msgType = m->getCommand();

	if (0 == msgType.compare("IDRT_USER"))
		return IDRT_USER;

	if (0 == msgType.compare("IDRT_REPEATER"))
		return IDRT_REPEATER;

	if (0 == msgType.compare("IDRT_GATEWAY"))
		return IDRT_GATEWAY;

	if(msgType.compare("NATTRAVERSAL_G2") == 0)
		return IDRT_NATTRAVERSAL_G2;

	if(msgType.compare("NATTRAVERSAL_DEXTRA") == 0)
		return IDRT_NATTRAVERSAL_DEXTRA;

	if(msgType.compare("NATTRAVERSAL_DPLUS") == 0)
		return IDRT_NATTRAVERSAL_DPLUS;

	CLog::logWarning("IRCDDBApp::getMessageType: unknown msg type: %s\n", msgType.c_str());

	return IDRT_NONE;
}

IRCMessage *IRCDDBApp::getReplyMessage()
{
	return m_d->m_replyQ.getMessage();
}

void IRCDDBApp::startWork()
{
	m_d->m_terminateThread = false;
	m_future = std::async(std::launch::async, &IRCDDBApp::Entry, this);
}

void IRCDDBApp::stopWork()
{
    m_d->m_terminateThread = true;
	m_future.get();
}

unsigned int IRCDDBApp::calculateUsn(const std::string& nick)
{
	std::string::size_type pos = nick.find_last_of('-');
	std::string lnick = std::string::npos==pos ? nick : nick.substr(0, pos);
	lnick.push_back('-');
	unsigned int maxUsn = 0;
	for (int i = 1; i <= 4; i++) {
		std::string ircUser = lnick + std::to_string(i);

		if (m_d->m_userMap.count(ircUser) == 1) {
			IRCDDBAppUserObject obj = m_d->m_userMap[ircUser];
			if (obj.m_usn > maxUsn)
				maxUsn = obj.m_usn;
		}
	}
	return maxUsn + 1;
}

void IRCDDBApp::userJoin(const std::string& nick, const std::string& name, const std::string& host)
{
	std::lock_guard lockUserMap(m_d->m_userMapMutex);

	std::string lnick = nick;
	CUtils::ToLower(lnick);

	IRCDDBAppUserObject u(lnick, name, host);
	u.m_usn = calculateUsn(lnick);

	m_d->m_userMap[lnick] = u;

	/*if (m_d->m_initReady)*/ {
		std::string::size_type hyphenPos = nick.find('-');

		if ((hyphenPos >= 4) && (hyphenPos <= 6)) {
			std::string gatewayCallsign = nick.substr(0, hyphenPos);
			CUtils::ToUpper(gatewayCallsign);
			gatewayCallsign.resize(7, ' ');
			gatewayCallsign.push_back('G');

			IRCMessage *m2 = new IRCMessage("IDRT_GATEWAY");
			m2->addParam(gatewayCallsign);
			m2->addParam(host);
			m_d->m_replyQ.putMessage(m2);
		}
	}
}

void IRCDDBApp::userLeave(const std::string& nick)
{
	std::string lnick = nick;
	CUtils::ToLower(lnick);

	std::lock_guard lockUserMap(m_d->m_userMapMutex);
	m_d->m_userMap.erase(lnick);

	if (m_d->m_currentServer.size()) {
		if (m_d->m_userMap.count(m_d->m_myNick) != 1) {
			CLog::logInfo("IRCDDBApp::userLeave: could not find own nick\n");
			return;
		}

		IRCDDBAppUserObject me = m_d->m_userMap[m_d->m_myNick];

		if (me.m_op == false) {
			// if I am not op, then look for new server

			if (0 == m_d->m_currentServer.compare(lnick)) {
				// m_currentServer = null;
				m_d->m_state = 2;  // choose new server
				m_d->m_timer = 200;
				m_d->m_initReady = false;
			}
		}
	}
}

void IRCDDBApp::userListReset()
{
  std::lock_guard lockUserMap(m_d->m_userMapMutex);
  m_d->m_userMap.clear();
}

void IRCDDBApp::setCurrentNick(const std::string& nick)
{
	m_d->m_myNick = nick;
	CLog::logInfo("IRCDDBApp::setCurrentNick %s\n", nick.c_str());
}

void IRCDDBApp::setBestServer(const std::string& ircUser)
{
	m_d->m_bestServer = ircUser;
	CLog::logInfo("IRCDDBApp::setBestServer %s\n", ircUser.c_str());
}

void IRCDDBApp::setTopic(const std::string& topic)
{
	m_d->m_channelTopic = topic;
}

bool IRCDDBApp::findServerUser()
{
	bool found = false;
	std::lock_guard lockUserMap(m_d->m_userMapMutex);

	std::map<std::string, IRCDDBAppUserObject>::iterator it;

	for (it = m_d->m_userMap.begin(); it != m_d->m_userMap.end(); ++it) {
		IRCDDBAppUserObject u = it->second;

		if (0==u.m_nick.compare(0, 2, "s-") && u.m_op && m_d->m_myNick.compare(u.m_nick) && 0==u.m_nick.compare(m_d->m_bestServer)) {
			m_d->m_currentServer = u.m_nick;
			found = true;
			break;
		}
	}

	if (found) {
		return true;
	}

	if (8 == m_d->m_bestServer.size()) {
		for (it = m_d->m_userMap.begin(); it != m_d->m_userMap.end(); ++it) {
			IRCDDBAppUserObject u = it->second;

			if (0==u.m_nick.compare(m_d->m_bestServer.substr(0,7)) && u.m_op && m_d->m_myNick.compare(u.m_nick) ) {
				m_d->m_currentServer = u.m_nick;
				found = true;
				break;
			}
		}
	}

	if (found) {
		return true;
	}

	for (it = m_d->m_userMap.begin(); it != m_d->m_userMap.end(); ++it) {
		IRCDDBAppUserObject u = it->second;
		if (0==u.m_nick.compare(0, 2, "s-") && u.m_op && m_d->m_myNick.compare(u.m_nick)) {
			m_d->m_currentServer = u.m_nick;
			found = true;
			break;
		}
	}
	return found;
}

void IRCDDBApp::userChanOp(const std::string& nick, bool op)
{
	std::lock_guard lockUserMap(m_d->m_userMapMutex);

	std::string lnick = nick;
	CUtils::ToLower(lnick);

	if (m_d->m_userMap.count(lnick) == 1)
		m_d->m_userMap[lnick].m_op = op;
}

static const int numberOfTables = 2;

std::string IRCDDBApp::getIPAddressFromCall(std::string& zonerp_cs)
{
	unsigned int max_usn = 0;
	std::string ipAddr;
	std::string gw = zonerp_cs;
	CUtils::ReplaceChar(gw, '_', ' ');
	CUtils::ToLower(gw);
	CUtils::Trim(gw);

	std::lock_guard lockUserMap(m_d->m_userMapMutex);
	for (int j=1; j <= 4; j++) {
		std::string ircUser = gw + std::string("-") + std::to_string(j);

		if (m_d->m_userMap.count(ircUser) == 1) {
			IRCDDBAppUserObject o = m_d->m_userMap[ircUser];

			if (o.m_usn >= max_usn) {
				max_usn = o.m_usn;
				ipAddr = o.m_host.c_str();
			}
		}
	}
	return ipAddr;
}

std::string IRCDDBApp::getIPAddressFromNick(std::string& ircUser)
{
	std::string ipAddress;

	if (m_d->m_userMap.count(ircUser) == 1) {
		ipAddress.assign(m_d->m_userMap[ircUser].m_host);
	}

	return ipAddress;
}

bool IRCDDBApp::findGateway(const std::string& gwCall)
{
	std::string s = gwCall.substr(0,6);

	IRCMessage *m2 = new IRCMessage("IDRT_GATEWAY");
	m2->addParam(gwCall);
	m2->addParam(getIPAddressFromCall(s));
	m_d->m_replyQ.putMessage(m2);

	return true;
}

static void findReflector(const std::string& rptrCall, IRCDDBAppPrivate *d)
{
	std::string zonerp_cs;
	std::string ipAddr;

	const unsigned int MAXIPV4ADDR = 5;
	struct sockaddr_in addr[MAXIPV4ADDR];
	unsigned int numAddr = 0;

	char host_name[80];

	std::string host = rptrCall.substr(0,6) + std::string(".reflector.ircddb.net");

	CUtils::safeStringCopy(host_name, host.c_str(), sizeof host_name);

	if (0 == CUtils::getAllIPV4Addresses(host_name, 0, &numAddr, addr, MAXIPV4ADDR)) {
		if (numAddr > 0) {
			unsigned char *a = (unsigned char *)&addr[0].sin_addr;
			char istr[16];
			snprintf(istr, 16, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]);
			std::string ipAddr(istr);
			zonerp_cs = rptrCall;
			zonerp_cs[7] = 'G';
		}
	}

	IRCMessage *m2 = new IRCMessage("IDRT_REPEATER");
	m2->addParam(rptrCall);
	m2->addParam(zonerp_cs);
	m2->addParam(ipAddr);
	d->m_replyQ.putMessage(m2);
}

bool IRCDDBApp::findRepeater(const std::string& rptrCall)
{
	if (0==rptrCall.compare(0, 3, "XRF") || 0==rptrCall.compare(0, 3, "REF") || 0==rptrCall.compare(0, 3, "DCS") || 0==rptrCall.compare(0, 3, "XLX") ) {
		findReflector(rptrCall, m_d);
		return true;
	}

	std::string arearp_cs(rptrCall);
	CUtils::ReplaceChar(arearp_cs, ' ',  '_');

	std::string s("NONE");
	std::string zonerp_cs;
	std::lock_guard lockRptrMap(m_d->m_rptrMapMutex);

	if (1 == m_d->m_rptrMap.count(arearp_cs)) {
		IRCDDBAppRptrObject o = m_d->m_rptrMap[arearp_cs];
		zonerp_cs = o.m_zonerp_cs;
		CUtils::ReplaceChar(zonerp_cs, '_', ' ');
		zonerp_cs.resize(7, ' ');
		zonerp_cs.push_back('G');
		s = o.m_zonerp_cs;
	}

	IRCMessage * m2 = new IRCMessage("IDRT_REPEATER");
	m2->addParam(rptrCall);
	m2->addParam(zonerp_cs);
	m2->addParam(getIPAddressFromCall(s));
	m_d->m_replyQ.putMessage(m2);

	return true;
}

void IRCDDBApp::sendDStarGatewayInfo(const std::string &subcommand, const std::vector<std::string> &pars)
{
	IRCMessageQueue *q = getSendQ();
	std::string srv(m_d->m_currentServer);
	if (srv.size() && m_d->m_state>=6 && q) {
		std::string command("DStarGateway ");
		command.append(subcommand);
		for (auto it=pars.begin(); it!=pars.end(); it++) {
			command.push_back(' ');
			command.append(*it);
		}
		IRCMessage *m = new IRCMessage(srv, command);
		q->putMessage(m);
	}
}

bool IRCDDBApp::sendHeard(const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1, const std::string& rpt2, unsigned char flag1,
													unsigned char flag2, unsigned char flag3, const std::string& destination, const std::string& tx_msg, const std::string& tx_stats)
{
	std::string my(myCall);
	std::string myext(myCallExt);
	std::string ur(yourCall);
	std::string r1(rpt1);
	std::string r2(rpt2);
	std::string dest(destination);
	std::regex nonValid("[^A-Z0-9/_]");
	std::smatch sm;
	char underScore = '_';
	while (std::regex_search(my, sm, nonValid))
		my[sm.position(0)] = underScore;
	while (std::regex_search(myext, sm, nonValid))
		myext[sm.position(0)] = underScore;
	while (std::regex_search(ur, sm, nonValid))
		ur[sm.position(0)] = underScore;
	while (std::regex_search(r1, sm, nonValid))
		r1[sm.position(0)] = underScore;
	while (std::regex_search(r2, sm, nonValid))
		r2[sm.position(0)] = underScore;
	while (std::regex_search(dest, sm, nonValid))
		dest[sm.position(0)] = underScore;

	bool statsMsg = (tx_stats.size() > 0);

	std::string srv(m_d->m_currentServer);
	IRCMessageQueue *q = getSendQ();

	if (srv.size() && m_d->m_state>=6 && q) {
		std::string cmd("UPDATE ");

		cmd += CUtils::getCurrentTime();
		cmd += std::string(" ") + my + std::string(" ") + r1 + std::string(" ");
		if (!statsMsg)
			cmd.append("0 ");
		cmd += r2 + std::string(" ") + ur + std::string(" ");

		char flags[10];
		snprintf(flags, 10, "%02X %02X %02X", flag1, flag2, flag3);
		cmd.append(flags);
		
		cmd += std::string(" ") + myext;

		if (statsMsg)
			cmd += std::string(" # ") + tx_stats;
		else {
			cmd += std::string(" 00 ") + dest;
			if (20 == tx_msg.size())
				cmd += std::string(" ") + tx_msg;
		}
		IRCMessage *m = new IRCMessage(srv, cmd);
		q->putMessage(m);
		return true;
	}
	return false;
}

bool IRCDDBApp::findUser(const std::string& usrCall)
{
	std::string srv(m_d->m_currentServer);
	IRCMessageQueue *q = getSendQ();

	if (srv.size()>0 && m_d->m_state>=6 && q) {
		std::string usr(usrCall);
		CUtils::ReplaceChar(usr, ' ', '_');
		IRCMessage * m =new IRCMessage(srv, std::string("FIND ") + usr);
		q->putMessage(m);
	} else {
		IRCMessage *m2 = new IRCMessage("IDRT_USER");
		m2->addParam(usrCall);
		for (int i=0; i<4; i++)
			m2->addParam(std::string(""));
		m_d->m_replyQ.putMessage(m2);
	}
	return true;
}

bool IRCDDBApp::notifyRepeaterG2NatTraversal(const std::string& repeater)
{
	std::string nick;

	if(!getNickForRepeater(repeater, nick))
		return true; //return true because this return value is handled as a network error, whoch is actually uncleve

	IRCMessage * ircMessage = new IRCMessage(nick, "NATTRAVERSAL_G2");
	m_d->m_sendQ->putMessage(ircMessage);

	return true;
}

bool IRCDDBApp::notifyRepeaterDextraNatTraversal(const std::string& repeater, unsigned int myLocalPort)
{
	std::string nick;

	if(!getNickForRepeater(repeater, nick))
		return true; //return true because this return value is handled as a network error, whoch is actually uncleve

	IRCMessage * ircMessage = new IRCMessage(nick, "NATTRAVERSAL_DEXTRA");
	ircMessage->addParam(std::to_string(myLocalPort));
	m_d->m_sendQ->putMessage(ircMessage);

	return true;
}

bool IRCDDBApp::notifyRepeaterDPlusNatTraversal(const std::string& repeater, unsigned int myLocalPort)
{
	std::string nick;

	if(!getNickForRepeater(repeater, nick))
		return true; //return true because this return value is handled as a network error, whoch is actually unclever

	IRCMessage * ircMessage = new IRCMessage(nick, "NATTRAVERSAL_DPLUS");
	ircMessage->addParam(std::to_string(myLocalPort));
	m_d->m_sendQ->putMessage(ircMessage);

	return true;
}

bool IRCDDBApp::getNickForRepeater(const std::string& repeater, std::string& nick) const
{
	std::lock_guard lockUserMap(m_d->m_userMapMutex);

	auto firstSpacePos = repeater.find_first_of(' ');
	if(firstSpacePos == std::string::npos)
		return false;

	auto lrepeater = repeater.substr(0, firstSpacePos);
	CUtils::ToLower(lrepeater);
	
	for(unsigned int i = 1; i <= 4U; i++) {
		nick = lrepeater + "-" + std::to_string(i);
		if(m_d->m_userMap.count(nick) == 1) {
			return true;
		}
	}

	nick.clear();
	CLog::logDebug("Unable to find IRC nick for repeater %s", repeater.c_str());
	return false;
}

void IRCDDBApp::msgChannel(IRCMessage *m)
{
	if (0==m->getPrefixNick().compare(0, 2, "s-") && m->m_numParams>=2)  // server msg
		doUpdate(m->m_params[1]);
}

void IRCDDBApp::doNotFound(std::string& msg, std::string& retval)
{
	int tableID = 0;
	std::vector<std::string> tkz = CUtils::stringTokenizer(msg);

	if (tkz.empty())
		return;  // no text in message

	std::string tk = tkz.front();
	tkz.erase(tkz.begin());
	
	if (std::regex_match(tk, m_d->m_tablePattern)) {
		tableID = std::stoi(tk);

		if (tableID<0 || tableID>=numberOfTables) {
			CLog::logInfo("invalid table ID %d\n", tableID);
			return;
		}

		if (tkz.empty())
			return;  // received nothing but the tableID

		tk = tkz.front();
		tk.erase(tk.begin());
	}

	if (0 == tableID) {
		if (! std::regex_match(tk, m_d->m_dbPattern))
			return; // no valid key
		retval = tk;
	}
}

void IRCDDBApp::doUpdate(std::string& msg)
{
	int tableID = 0;
	std::vector<std::string> tkz = CUtils::stringTokenizer(msg);
	if (tkz.empty())
		return;  // no text in message

	std::string tk = tkz.front();
	tkz.erase(tkz.begin());

	if (std::regex_match(tk, m_d->m_tablePattern)) {
		tableID = std::stoi(tk);
		if ((tableID < 0) || (tableID >= numberOfTables)) {
			CLog::logInfo("invalid table ID %d\n", tableID);
			return;
		}

		if (tkz.empty())
			return;  // received nothing but the tableID

		tk = tkz.front();
		tkz.erase(tkz.begin());
	}

	if (std::regex_match(tk, m_d->m_datePattern)) {
		if (tkz.empty())
			return;  // nothing after date string

		std::string timeToken = tkz.front();	// time token
		tkz.erase(tkz.begin());
		if (! std::regex_match(timeToken, m_d->m_timePattern))
			return; // no time string after date string

		time_t dt = CUtils::parseTime(tk + std::string(" ") + timeToken);

		if (0==tableID || 1==tableID) {
			if (tkz.empty())
				return;  // nothing after time string

			std::string key = tkz.front();
			tkz.erase(tkz.begin());

			if (! std::regex_match(key, m_d->m_dbPattern))
				return; // no valid key

			if (tkz.empty())
				return;  // nothing after time string

			std::string value = tkz.front();
			tkz.erase(tkz.begin());

			if (! std::regex_match(value, m_d->m_dbPattern))
				return; // no valid key

			if (tableID == 1) {
				std::lock_guard lockRptrMap(m_d->m_rptrMapMutex);
				IRCDDBAppRptrObject newRptr(dt, key, value, m_maxTime);
				m_d->m_rptrMap[key] = newRptr;

				if (m_d->m_initReady) {
					std::string arearp_cs(key);
					std::string zonerp_cs(value);
					CUtils::ReplaceChar(arearp_cs, '_', ' ');
					CUtils::ReplaceChar(zonerp_cs, '_', ' ');
					zonerp_cs.resize(7, ' ');
					zonerp_cs.push_back('G');

					IRCMessage *m2 = new IRCMessage("IDRT_REPEATER");
					m2->addParam(arearp_cs);
					m2->addParam(zonerp_cs);
					m2->addParam(getIPAddressFromCall(value));
					m_d->m_replyQ.putMessage(m2);
				}
			} else if (0==tableID && m_d->m_initReady) {
				std::lock_guard lockRptrMap(m_d->m_rptrMapMutex);
				std::string userCallsign(key);
				std::string arearp_cs(value);
				std::string zonerp_cs;
				std::string ip_addr;
				CUtils::ReplaceChar(userCallsign, '_', ' ');
				CUtils::ReplaceChar(arearp_cs, '_', ' ');

				std::smatch sm1;
				std::string nick;
				if(std::regex_search(msg, sm1, m_d->m_fromPattern))
					nick = sm1[1];

				if (1 == m_d->m_rptrMap.count(value)) {
					// CLog::logTrace("doUptate RPTR already present");
					IRCDDBAppRptrObject o = m_d->m_rptrMap[value];
					zonerp_cs = o.m_zonerp_cs;
					CUtils::ReplaceChar(zonerp_cs, '_', ' ');
					zonerp_cs.resize(7, ' ');
					ip_addr = nick.empty() ? getIPAddressFromCall(zonerp_cs) : getIPAddressFromNick(nick);
					zonerp_cs.push_back('G');
				}
				else {
					// CLog::logTrace("doUptate RPTR not present");
					zonerp_cs = arearp_cs.substr(0, arearp_cs.length() - 1U);
					ip_addr = nick.empty() ? getIPAddressFromCall(zonerp_cs) : getIPAddressFromNick(nick);
					zonerp_cs.push_back('G');

					if(!ip_addr.empty()) {
						auto tmp = boost::replace_all_copy(zonerp_cs, " ", "_");
						IRCDDBAppRptrObject newRptr(dt, value, tmp, m_maxTime);
						m_d->m_rptrMap[value] = newRptr;
					}
				}

				IRCMessage *m2 = new IRCMessage("IDRT_USER");
				m2->addParam(userCallsign);
				m2->addParam(arearp_cs);
				m2->addParam(zonerp_cs);
				m2->addParam(ip_addr);
				m2->addParam(tk + std::string(" ") + timeToken);
				m_d->m_replyQ.putMessage(m2);
			}
		}
	}
}

static std::string getTableIDString(int tableID, bool spaceBeforeNumber)
{
	if (0 == tableID)
		return std::string("");
	else if (tableID>0 && tableID<numberOfTables) {
		if (spaceBeforeNumber)
			return std::string(" ") + std::to_string(tableID);
		else
			return std::to_string(tableID) + std::string(" ");
	}
	else
		return " TABLE_ID_OUT_OF_RANGE ";
}

void IRCDDBApp::msgQuery(IRCMessage *m)
{
	if (0 == m->getPrefixNick().compare(0, 2, "s-") && m->m_numParams >=2 ) {	// server msg
		std::string msg(m->m_params[1]);
		std::vector<std::string> tkz = CUtils::stringTokenizer(msg);

		if (tkz.empty())
			return;  // no text in message

		std::string cmd = tkz.front();
		tkz.erase(tkz.begin());

		if (0 == cmd.compare("UPDATE")) {
			std::string restOfLine;
			while (! tkz.empty()) {
				restOfLine += tkz.front();
				tkz.erase(tkz.begin());
				if (! tkz.empty())
					restOfLine.push_back(' ');
			}
			doUpdate(restOfLine);
		} else if (0 == cmd.compare("LIST_END")) {
			if (5 == m_d->m_state) // if in sendlist processing state
				m_d->m_state = 3;  // get next table
		} else if (0 == cmd.compare("LIST_MORE")) {
			if (5 == m_d->m_state) // if in sendlist processing state
				m_d->m_state = 4;  // send next SENDLIST
		} else if (0 == cmd.compare("NOT_FOUND")) {
			std::string callsign;
			std::string restOfLine;
			while (! tkz.empty()) {
				restOfLine += tkz.front();
				tkz.erase(tkz.begin());
				if (! tkz.empty())
					restOfLine.push_back(' ');
			}
			doNotFound(restOfLine, callsign);

			if (callsign.size() > 0) {
				CUtils::ReplaceChar(callsign, '_', ' ');
				IRCMessage *m2 = new IRCMessage("IDRT_USER");
				m2->addParam(callsign);
				for (int i=0; i<4; i++)
					m2->addParam(std::string(""));
				m_d->m_replyQ.putMessage(m2);
			}
		}
	}
	else if(m->m_params[0] == m_d->m_myNick) {
		if(m->m_params.size() >= 2U && m->m_params[1] == "NATTRAVERSAL_G2") {
			IRCMessage * m2 = new IRCMessage(m->m_params[1]);
			m2->addParam(m->getPrefixHost());
			m_d->m_replyQ.putMessage(m2);
		}
		else if(m->m_params.size() >= 2U && boost::starts_with(m->m_params[1], "NATTRAVERSAL_DEXTRA")) {
			IRCMessage * m2 = new IRCMessage(m->m_params[1].substr(0, (std::string("NATTRAVERSAL_DEXTRA")).length()));
			m2->addParam(m->getPrefixHost());
			std::string remotePort = boost::trim_copy(boost::replace_all_copy(m->m_params[1], "NATTRAVERSAL_DEXTRA", ""));
			m2->addParam(remotePort);
			m_d->m_replyQ.putMessage(m2);
		}
		else if(m->m_params.size() >= 2U && boost::starts_with(m->m_params[1], "NATTRAVERSAL_DPLUS")) {
			IRCMessage * m2 = new IRCMessage(m->m_params[1].substr(0, (std::string("NATTRAVERSAL_DPLUS")).length()));
			m2->addParam(m->getPrefixHost());
			std::string remotePort = boost::trim_copy(boost::replace_all_copy(m->m_params[1], "NATTRAVERSAL_DPLUS", ""));
			m2->addParam(remotePort);
			m_d->m_replyQ.putMessage(m2);
		}
	}
}

void IRCDDBApp::setSendQ(IRCMessageQueue *s)
{
	m_d->m_sendQ = s;
}

IRCMessageQueue *IRCDDBApp::getSendQ()
{
	return m_d->m_sendQ;
}

std::string IRCDDBApp::getLastEntryTime(int tableID)
{
	if (1 == tableID) {
		struct tm *ptm = gmtime(&m_maxTime);
		char tstr[80];
		strftime(tstr, 80, "%Y-%m-%d %H:%M:%S", ptm);
		std::string max = tstr;
		return max;
	}
	return "DBERROR";
}

static bool needsDatabaseUpdate(int tableID)
{
	return (1 == tableID);
}

void IRCDDBApp::Entry()
{
	int sendlistTableID = 0;
	while (!m_d->m_terminateThread) {
		if (m_d->m_timer > 0)
			m_d->m_timer--;
		switch(m_d->m_state) {
			case 0:	// wait for network to start
				if (getSendQ())
					m_d->m_state = 1;
				break;

			case 1:	// connect to db
				m_d->m_state = 2;
				m_d->m_timer = 200;
				break;

			case 2:	// choose server
				CLog::logInfo("IRCDDBApp: state=2 choose new 's-'-user\n");
				if (NULL == getSendQ())
					m_d->m_state = 10;
				else {
					if (findServerUser()) {
						sendlistTableID = numberOfTables;
						m_d->m_state = 3; // next: send "SENDLIST"
					} else if (0 == m_d->m_timer) {
						m_d->m_state = 10;
						IRCMessage *m = new IRCMessage("QUIT");
						m->addParam("no op user with 's-' found.");
						IRCMessageQueue *q = getSendQ();
						if (q)
							q->putMessage(m);
					}
				}
				break;

			case 3:
				if (NULL == getSendQ())
					m_d->m_state = 10; // disconnect DB
				else {
					sendlistTableID--;
					if (sendlistTableID < 0)
						m_d->m_state = 6; // end of sendlist
					else {
						CLog::logInfo("IRCDDBApp: state=3 tableID=%d\n", sendlistTableID);
						m_d->m_state = 4; // send "SENDLIST"
						m_d->m_timer = 900; // 15 minutes max for update
					}
				}
				break;

			case 4:
				if (NULL == getSendQ())
					m_d->m_state = 10; // disconnect DB
				else {
					if (needsDatabaseUpdate(sendlistTableID)) {
						IRCMessage *m = new IRCMessage(m_d->m_currentServer, std::string("SENDLIST") + getTableIDString(sendlistTableID, true) + std::string(" ") + getLastEntryTime(sendlistTableID));
						IRCMessageQueue *q = getSendQ();
						if (q)
							q->putMessage(m);
						m_d->m_state = 5; // wait for answers
					} else
						m_d->m_state = 3; // don't send SENDLIST for this table, go to next table
				}
				break;

			case 5: // sendlist processing
				if (NULL == getSendQ())
					m_d->m_state = 10; // disconnect DB
				else if (0 == m_d->m_timer) {
					m_d->m_state = 10; // disconnect DB
					IRCMessage *m = new IRCMessage("QUIT");
					m->addParam("timeout SENDLIST");
					IRCMessageQueue *q = getSendQ();
					if (q)
						q->putMessage(m);
				}
				break;

			case 6:
				if (NULL == getSendQ())
					m_d->m_state = 10; // disconnect DB
				else {
					CLog::logInfo( "IRCDDBApp: state=6 initialization completed\n");
					m_d->m_infoTimer = 2;
					m_d->m_initReady = true;
					m_d->m_state = 7;
				}
				break;

			case 7: // standby state after initialization
				if (NULL == getSendQ())
					m_d->m_state = 10; // disconnect DB

				if (m_d->m_infoTimer > 0) {
					m_d->m_infoTimer--;

					if (0 == m_d->m_infoTimer) {
						{	// Scope for mutext locking
							std::lock_guard lochQTHURL(m_d->m_moduleQTHURLMutex);
							for (auto it = m_d->m_moduleQTH.begin(); it != m_d->m_moduleQTH.end(); ++it) {
								std::string value = it->second;
								IRCMessage *m = new IRCMessage(m_d->m_currentServer, std::string("IRCDDB RPTRQTH: ") + value);
								IRCMessageQueue *q = getSendQ();
								if (q != NULL)
									q->putMessage(m);
							}
							m_d->m_moduleQTH.clear();

							for (auto it = m_d->m_moduleURL.begin(); it != m_d->m_moduleURL.end(); ++it) {
								std::string value = it->second;
								IRCMessage *m = new IRCMessage(m_d->m_currentServer, std::string("IRCDDB RPTRURL: ") + value);
								IRCMessageQueue *q = getSendQ();
								if (q != NULL)
									q->putMessage(m);
							}
							m_d->m_moduleURL.clear();
						}

						std::lock_guard lockModuleQRG(m_d->m_moduleQRGMutex);
						for (auto it = m_d->m_moduleQRG.begin(); it != m_d->m_moduleQRG.end(); ++it) {
							std::string value = it->second;
							IRCMessage* m = new IRCMessage(m_d->m_currentServer, std::string("IRCDDB RPTRQRG: ") + value);
							IRCMessageQueue* q = getSendQ();
							if (q != NULL)
								q->putMessage(m);
						}
						m_d->m_moduleQRG.clear();
					}
				}

				if (m_d->m_wdTimer > 0) {
					m_d->m_wdTimer--;

					if (0 == m_d->m_wdTimer) {
						std::lock_guard lockModuleWD(m_d->m_moduleWDMutex);

						for (auto it = m_d->m_moduleWD.begin(); it != m_d->m_moduleWD.end(); ++it) {
							std::string value = it->second;
							IRCMessage *m = new IRCMessage(m_d->m_currentServer, std::string("IRCDDB RPTRSW: ") + value);
							IRCMessageQueue *q = getSendQ();
							if (q)
								q->putMessage(m);
						}
						m_d->m_moduleWD.clear();
					}
				}
				break;

			case 10:
				// disconnect db
				m_d->m_state = 0;
				m_d->m_timer = 0;
				m_d->m_initReady = false;
				break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	} // while
	return;
}

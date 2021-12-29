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

#include "IRCDDBApp.h"
#include "Utils.h"
#include "Log.h"

class IRCDDBAppUserObject
{
public:
	std::string nick;
	std::string name;
	std::string host;
	bool op;
	unsigned int usn;

	IRCDDBAppUserObject()
	{
//		IRCDDBAppUserObject(std::string(""), std::string(""), std::string(""));
	}

	IRCDDBAppUserObject(const std::string& n, const std::string& nm, const std::string& h)
	{
		nick = n;
		name = nm;
		host = h;
		op = false;
		usn = 0;
	}
};

class IRCDDBAppRptrObject
{
public:
	std::string arearp_cs;
	time_t lastChanged;
	std::string zonerp_cs;

	IRCDDBAppRptrObject ()
	{
	}

	IRCDDBAppRptrObject (time_t &dt, std::string& repeaterCallsign, std::string& gatewayCallsign, time_t &maxTime)
	{
		arearp_cs = repeaterCallsign;
		lastChanged = dt;
		zonerp_cs = gatewayCallsign;

		if (dt > maxTime)
			maxTime = dt;
	}
};

class IRCDDBAppPrivate
{
public:
	IRCDDBAppPrivate()
	: tablePattern("^[0-9]$")
	, datePattern("^20[0-9][0-9]-((1[0-2])|(0[1-9]))-((3[01])|([12][0-9])|(0[1-9]))$")
	, timePattern("^((2[0-3])|([01][0-9])):[0-5][0-9]:[0-5][0-9]$")
	, dbPattern("^[0-9A-Z_]{8}$")
	{
	}

	int state;
	int timer;
	int infoTimer;
	int wdTimer;

	IRCMessageQueue *sendQ;
	IRCMessageQueue replyQ;

	std::string currentServer;
	std::string myNick;
	std::string updateChannel;
	std::string channelTopic;
	std::string bestServer;

	std::regex tablePattern;
	std::regex datePattern;
	std::regex timePattern;
	std::regex dbPattern;

	bool initReady;
	bool terminateThread;

	std::map<std::string, IRCDDBAppUserObject> user;
	std::mutex userMapMutex;

	std::map<std::string, IRCDDBAppRptrObject> rptrMap;
	std::mutex rptrMapMutex;

	std::map<std::string, std::string> moduleQRG;
	std::mutex moduleQRGMutex;

	std::map<std::string, std::string> moduleQTH;
	std::map<std::string, std::string> moduleURL;
	std::mutex moduleQTHURLMutex;

	std::map<std::string, std::string> moduleWD;
	std::mutex moduleWDMutex;
};

IRCDDBApp::IRCDDBApp(const std::string& u_chan)
	: d(new IRCDDBAppPrivate)
	, m_maxTime((time_t)950000000)	//februray 2000
{
	d->sendQ = NULL;
	d->initReady = false;

	userListReset();

	d->state = 0;
	d->timer = 0;
	d->myNick = std::string("none");

	d->updateChannel = u_chan;

	d->terminateThread = false;
}

IRCDDBApp::~IRCDDBApp()
{
	delete d->sendQ;
	delete d;
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

	std::lock_guard lochQTHURL(d->moduleQTHURLMutex);

	d->moduleQTH[cs] = cs + std::string(" ") + pos + std::string(" ") + d1 + std::string(" ") + d2;

	CLog::logInfo("QTH: %s\n", d->moduleQTH[cs].c_str());

	std::string url = infoURL;

	std::regex urlNonValid("[^[:graph:]]");
	while (std::regex_search(url, sm, urlNonValid))
		url.erase(sm.position(0), sm.length());

	if (url.size()) {
		d->moduleURL[cs] = cs + std::string(" ") + url;
		CLog::logInfo("URL: %s\n", d->moduleURL[cs].c_str());
	}

	d->infoTimer = 5; // send info in 5 seconds
}

void IRCDDBApp::rptrQRG(const std::string& callsign, double txFrequency, double duplexShift, double range, double agl)
{
	std::string cs = callsign;
	CUtils::ReplaceChar(cs, ' ', '_');

	char fstr[64];
	snprintf(fstr, 64, "%011.5f %+010.5f %06.2f %06.1f", txFrequency, duplexShift, range / 1609.344, agl);
	std::string f(fstr);
	CUtils::ReplaceChar(f, ',', '.');

	std::lock_guard lockModuleQRG(d->moduleQRGMutex);
	d->moduleQRG[cs] = cs + std::string(" ") + f;
	CLog::logInfo("QRG: %s\n", d->moduleQRG[cs].c_str());

	d->infoTimer = 5; // send info in 5 seconds
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

		std::lock_guard lockModuleWD(d->moduleWDMutex);
		d->moduleWD[cs] = cs + std::string(" ") + text;
		d->wdTimer = 60;
	}
}

int IRCDDBApp::getConnectionState()
{
	return d->state;
}

IRCDDB_RESPONSE_TYPE IRCDDBApp::getReplyMessageType()
{
	IRCMessage *m = d->replyQ.peekFirst();
	if (m == NULL)
		return IDRT_NONE;

	std::string msgType = m->getCommand();

	if (0 == msgType.compare("IDRT_USER"))
		return IDRT_USER;

	if (0 == msgType.compare("IDRT_REPEATER"))
		return IDRT_REPEATER;

	if (0 == msgType.compare("IDRT_GATEWAY"))
		return IDRT_GATEWAY;

	CLog::logInfo("IRCDDBApp::getMessageType: unknown msg type: %s\n", msgType.c_str());

	return IDRT_NONE;
}

IRCMessage *IRCDDBApp::getReplyMessage()
{
	return d->replyQ.getMessage();
}

void IRCDDBApp::startWork()
{
	d->terminateThread = false;
	m_future = std::async(std::launch::async, &IRCDDBApp::Entry, this);
}

void IRCDDBApp::stopWork()
{
    d->terminateThread = true;
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

		if (d->user.count(ircUser) == 1) {
			IRCDDBAppUserObject obj = d->user[ircUser];
			if (obj.usn > maxUsn)
				maxUsn = obj.usn;
		}
	}
	return maxUsn + 1;
}

void IRCDDBApp::userJoin(const std::string& nick, const std::string& name, const std::string& host)
{
	std::lock_guard lockUserMap(d->userMapMutex);

	std::string lnick = nick;
	CUtils::ToLower(lnick);

	IRCDDBAppUserObject u(lnick, name, host);
	u.usn = calculateUsn(lnick);

	d->user[lnick] = u;

	if (d->initReady) {
		std::string::size_type hyphenPos = nick.find('-');

		if ((hyphenPos >= 4) && (hyphenPos <= 6)) {
			std::string gatewayCallsign = nick.substr(0, hyphenPos);
			CUtils::ToUpper(gatewayCallsign);
			gatewayCallsign.resize(7, ' ');
			gatewayCallsign.push_back('G');

			IRCMessage *m2 = new IRCMessage("IDRT_GATEWAY");
			m2->addParam(gatewayCallsign);
			m2->addParam(host);
			d->replyQ.putMessage(m2);
		}
	}
}

void IRCDDBApp::userLeave(const std::string& nick)
{
	std::string lnick = nick;
	CUtils::ToLower(lnick);

	std::lock_guard lockUserMap(d->userMapMutex);
	d->user.erase(lnick);

	if (d->currentServer.size()) {
		if (d->user.count(d->myNick) != 1) {
			CLog::logInfo("IRCDDBApp::userLeave: could not find own nick\n");
			return;
		}

		IRCDDBAppUserObject me = d->user[d->myNick];

		if (me.op == false) {
			// if I am not op, then look for new server

			if (0 == d->currentServer.compare(lnick)) {
				// currentServer = null;
				d->state = 2;  // choose new server
				d->timer = 200;
				d->initReady = false;
			}
		}
	}
}

void IRCDDBApp::userListReset()
{
  std::lock_guard lockUserMap(d->userMapMutex);
  d->user.clear();
}

void IRCDDBApp::setCurrentNick(const std::string& nick)
{
	d->myNick = nick;
	CLog::logInfo("IRCDDBApp::setCurrentNick %s\n", nick.c_str());
}

void IRCDDBApp::setBestServer(const std::string& ircUser)
{
	d->bestServer = ircUser;
	CLog::logInfo("IRCDDBApp::setBestServer %s\n", ircUser.c_str());
}

void IRCDDBApp::setTopic(const std::string& topic)
{
	d->channelTopic = topic;
}

bool IRCDDBApp::findServerUser()
{
	bool found = false;
	std::lock_guard lockUserMap(d->userMapMutex);

	std::map<std::string, IRCDDBAppUserObject>::iterator it;

	for (it = d->user.begin(); it != d->user.end(); ++it) {
		IRCDDBAppUserObject u = it->second;

		if (0==u.nick.compare(0, 2, "s-") && u.op && d->myNick.compare(u.nick) && 0==u.nick.compare(d->bestServer)) {
			d->currentServer = u.nick;
			found = true;
			break;
		}
	}

	if (found) {
		return true;
	}

	if (8 == d->bestServer.size()) {
		for (it = d->user.begin(); it != d->user.end(); ++it) {
			IRCDDBAppUserObject u = it->second;

			if (0==u.nick.compare(d->bestServer.substr(0,7)) && u.op && d->myNick.compare(u.nick) ) {
				d->currentServer = u.nick;
				found = true;
				break;
			}
		}
	}

	if (found) {
		return true;
	}

	for (it = d->user.begin(); it != d->user.end(); ++it) {
		IRCDDBAppUserObject u = it->second;
		if (0==u.nick.compare(0, 2, "s-") && u.op && d->myNick.compare(u.nick)) {
			d->currentServer = u.nick;
			found = true;
			break;
		}
	}
	return found;
}

void IRCDDBApp::userChanOp(const std::string& nick, bool op)
{
	std::lock_guard lockUserMap(d->userMapMutex);

	std::string lnick = nick;
	CUtils::ToLower(lnick);

	if (d->user.count(lnick) == 1)
		d->user[lnick].op = op;
}

static const int numberOfTables = 2;

std::string IRCDDBApp::getIPAddress(std::string& zonerp_cs)
{
	unsigned int max_usn = 0;
	std::string ipAddr;
	std::string gw = zonerp_cs;
	CUtils::ReplaceChar(gw, '_', ' ');
	CUtils::ToLower(gw);
	CUtils::Trim(gw);

	std::lock_guard lockUserMap(d->userMapMutex);
	for (int j=1; j <= 4; j++) {
		std::string ircUser = gw + std::string("-") + std::to_string(j);

		if (d->user.count(ircUser) == 1) {
			IRCDDBAppUserObject o = d->user[ircUser];

			if (o.usn >= max_usn) {
				max_usn = o.usn;
				ipAddr = o.host.c_str();
			}
		}
	}
	return ipAddr;
}

bool IRCDDBApp::findGateway(const std::string& gwCall)
{
	std::string s = gwCall.substr(0,6);

	IRCMessage *m2 = new IRCMessage("IDRT_GATEWAY");
	m2->addParam(gwCall);
	m2->addParam(getIPAddress(s));
	d->replyQ.putMessage(m2);

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
	d->replyQ.putMessage(m2);
}

bool IRCDDBApp::findRepeater(const std::string& rptrCall)
{
	if (0==rptrCall.compare(0, 3, "XRF") || 0==rptrCall.compare(0, 3, "REF")) {
		findReflector(rptrCall, d);
		return true;
	}

	std::string arearp_cs(rptrCall);
	CUtils::ReplaceChar(arearp_cs, ' ',  '_');

	std::string s("NONE");
	std::string zonerp_cs;
	std::lock_guard lockRptrMap(d->rptrMapMutex);

	if (1 == d->rptrMap.count(arearp_cs)) {
		IRCDDBAppRptrObject o = d->rptrMap[arearp_cs];
		zonerp_cs = o.zonerp_cs;
		CUtils::ReplaceChar(zonerp_cs, '_', ' ');
		zonerp_cs.resize(7, ' ');
		zonerp_cs.push_back('G');
		s = o.zonerp_cs;
	}

	IRCMessage * m2 = new IRCMessage("IDRT_REPEATER");
	m2->addParam(rptrCall);
	m2->addParam(zonerp_cs);
	m2->addParam(getIPAddress(s));
	d->replyQ.putMessage(m2);

	return true;
}

void IRCDDBApp::sendDStarGatewayInfo(const std::string &subcommand, const std::vector<std::string> &pars)
{
	IRCMessageQueue *q = getSendQ();
	std::string srv(d->currentServer);
	if (srv.size() && d->state>=6 && q) {
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

	std::string srv(d->currentServer);
	IRCMessageQueue *q = getSendQ();

	if (srv.size() && d->state>=6 && q) {
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
	std::string srv(d->currentServer);
	IRCMessageQueue *q = getSendQ();

	if (srv.size()>0 && d->state>=6 && q) {
		std::string usr(usrCall);
		CUtils::ReplaceChar(usr, ' ', '_');
		IRCMessage * m =new IRCMessage(srv, std::string("FIND ") + usr);
		q->putMessage(m);
	} else {
		IRCMessage *m2 = new IRCMessage("IDRT_USER");
		m2->addParam(usrCall);
		for (int i=0; i<4; i++)
			m2->addParam(std::string(""));
		d->replyQ.putMessage(m2);
	}
	return true;
}

void IRCDDBApp::msgChannel(IRCMessage *m)
{
	if (0==m->getPrefixNick().compare(0, 2, "s-") && m->numParams>=2)  // server msg
		doUpdate(m->params[1]);
}

void IRCDDBApp::doNotFound(std::string& msg, std::string& retval)
{
	int tableID = 0;
	std::vector<std::string> tkz = CUtils::stringTokenizer(msg);

	if (tkz.empty())
		return;  // no text in message

	std::string tk = tkz.front();
	tkz.erase(tkz.begin());
	
	if (std::regex_match(tk, d->tablePattern)) {
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
		if (! std::regex_match(tk, d->dbPattern))
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

	if (std::regex_match(tk, d->tablePattern)) {
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

	if (std::regex_match(tk, d->datePattern)) {
		if (tkz.empty())
			return;  // nothing after date string

		std::string timeToken = tkz.front();	// time token
		tkz.erase(tkz.begin());
		if (! std::regex_match(timeToken, d->timePattern))
			return; // no time string after date string

		time_t dt = CUtils::parseTime(tk + std::string(" ") + timeToken);

		if (0==tableID || 1==tableID) {
			if (tkz.empty())
				return;  // nothing after time string

			std::string key = tkz.front();
			tkz.erase(tkz.begin());

			if (! std::regex_match(key, d->dbPattern))
				return; // no valid key

			if (tkz.empty())
				return;  // nothing after time string

			std::string value = tkz.front();
			tkz.erase(tkz.begin());

			if (! std::regex_match(value, d->dbPattern))
				return; // no valid key

			if (tableID == 1) {
				std::lock_guard lockRptrMap(d->rptrMapMutex);
				IRCDDBAppRptrObject newRptr(dt, key, value, m_maxTime);
				d->rptrMap[key] = newRptr;

				if (d->initReady) {
					std::string arearp_cs(key);
					std::string zonerp_cs(value);
					CUtils::ReplaceChar(arearp_cs, '_', ' ');
					CUtils::ReplaceChar(zonerp_cs, '_', ' ');
					zonerp_cs.resize(7, ' ');
					zonerp_cs.push_back('G');

					IRCMessage *m2 = new IRCMessage("IDRT_REPEATER");
					m2->addParam(arearp_cs);
					m2->addParam(zonerp_cs);
					m2->addParam(getIPAddress(value));
					d->replyQ.putMessage(m2);
				}
			} else if (0==tableID && d->initReady) {
				std::lock_guard lockRptrMap(d->rptrMapMutex);
				std::string userCallsign(key);
				std::string arearp_cs(value);
				std::string zonerp_cs;
				std::string ip_addr;
				CUtils::ReplaceChar(userCallsign, '_', ' ');
				CUtils::ReplaceChar(arearp_cs, '_', ' ');

				if (1 == d->rptrMap.count(value)) {
					IRCDDBAppRptrObject o = d->rptrMap[value];
					zonerp_cs = o.zonerp_cs;
					CUtils::ReplaceChar(zonerp_cs, '_', ' ');
					zonerp_cs.resize(7, ' ');
					zonerp_cs.push_back('G');
					ip_addr = getIPAddress(o.zonerp_cs);
				}

				IRCMessage *m2 = new IRCMessage("IDRT_USER");
				m2->addParam(userCallsign);
				m2->addParam(arearp_cs);
				m2->addParam(zonerp_cs);
				m2->addParam(ip_addr);
				m2->addParam(tk + std::string(" ") + timeToken);
				d->replyQ.putMessage(m2);
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
	if (0==m->getPrefixNick().compare(0, 2, "s-") && m->numParams>=2) {	// server msg
		std::string msg(m->params[1]);
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
			if (5 == d->state) // if in sendlist processing state
				d->state = 3;  // get next table
		} else if (0 == cmd.compare("LIST_MORE")) {
			if (5 == d->state) // if in sendlist processing state
				d->state = 4;  // send next SENDLIST
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
				d->replyQ.putMessage(m2);
			}
		}
	}
}

void IRCDDBApp::setSendQ(IRCMessageQueue *s)
{
	d->sendQ = s;
}

IRCMessageQueue *IRCDDBApp::getSendQ()
{
	return d->sendQ;
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
	while (!d->terminateThread) {
		if (d->timer > 0)
			d->timer--;
		switch(d->state) {
			case 0:	// wait for network to start
				if (getSendQ())
					d->state = 1;
				break;

			case 1:	// connect to db
				d->state = 2;
				d->timer = 200;
				break;

			case 2:	// choose server
				CLog::logInfo("IRCDDBApp: state=2 choose new 's-'-user\n");
				if (NULL == getSendQ())
					d->state = 10;
				else {
					if (findServerUser()) {
						sendlistTableID = numberOfTables;
						d->state = 3; // next: send "SENDLIST"
					} else if (0 == d->timer) {
						d->state = 10;
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
					d->state = 10; // disconnect DB
				else {
					sendlistTableID--;
					if (sendlistTableID < 0)
						d->state = 6; // end of sendlist
					else {
						CLog::logInfo("IRCDDBApp: state=3 tableID=%d\n", sendlistTableID);
						d->state = 4; // send "SENDLIST"
						d->timer = 900; // 15 minutes max for update
					}
				}
				break;

			case 4:
				if (NULL == getSendQ())
					d->state = 10; // disconnect DB
				else {
					if (needsDatabaseUpdate(sendlistTableID)) {
						IRCMessage *m = new IRCMessage(d->currentServer, std::string("SENDLIST") + getTableIDString(sendlistTableID, true) + std::string(" ") + getLastEntryTime(sendlistTableID));
						IRCMessageQueue *q = getSendQ();
						if (q)
							q->putMessage(m);
						d->state = 5; // wait for answers
					} else
						d->state = 3; // don't send SENDLIST for this table, go to next table
				}
				break;

			case 5: // sendlist processing
				if (NULL == getSendQ())
					d->state = 10; // disconnect DB
				else if (0 == d->timer) {
					d->state = 10; // disconnect DB
					IRCMessage *m = new IRCMessage("QUIT");
					m->addParam("timeout SENDLIST");
					IRCMessageQueue *q = getSendQ();
					if (q)
						q->putMessage(m);
				}
				break;

			case 6:
				if (NULL == getSendQ())
					d->state = 10; // disconnect DB
				else {
					CLog::logInfo( "IRCDDBApp: state=6 initialization completed\n");
					d->infoTimer = 2;
					d->initReady = true;
					d->state = 7;
				}
				break;

			case 7: // standby state after initialization
				if (NULL == getSendQ())
					d->state = 10; // disconnect DB

				if (d->infoTimer > 0) {
					d->infoTimer--;

					if (0 == d->infoTimer) {
						{	// Scope for mutext locking
							std::lock_guard lochQTHURL(d->moduleQTHURLMutex);
							for (auto it = d->moduleQTH.begin(); it != d->moduleQTH.end(); ++it) {
								std::string value = it->second;
								IRCMessage *m = new IRCMessage(d->currentServer, std::string("IRCDDB RPTRQTH: ") + value);
								IRCMessageQueue *q = getSendQ();
								if (q != NULL)
									q->putMessage(m);
							}
							d->moduleQTH.clear();

							for (auto it = d->moduleURL.begin(); it != d->moduleURL.end(); ++it) {
								std::string value = it->second;
								IRCMessage *m = new IRCMessage(d->currentServer, std::string("IRCDDB RPTRURL: ") + value);
								IRCMessageQueue *q = getSendQ();
								if (q != NULL)
									q->putMessage(m);
							}
							d->moduleURL.clear();
						}

						std::lock_guard lockModuleQRG(d->moduleQRGMutex);
						for (auto it = d->moduleQRG.begin(); it != d->moduleQRG.end(); ++it) {
							std::string value = it->second;
							IRCMessage* m = new IRCMessage(d->currentServer, std::string("IRCDDB RPTRQRG: ") + value);
							IRCMessageQueue* q = getSendQ();
							if (q != NULL)
								q->putMessage(m);
						}
						d->moduleQRG.clear();
					}
				}

				if (d->wdTimer > 0) {
					d->wdTimer--;

					if (0 == d->wdTimer) {
						std::lock_guard lockModuleWD(d->moduleWDMutex);

						for (auto it = d->moduleWD.begin(); it != d->moduleWD.end(); ++it) {
							std::string value = it->second;
							IRCMessage *m = new IRCMessage(d->currentServer, std::string("IRCDDB RPTRSW: ") + value);
							IRCMessageQueue *q = getSendQ();
							if (q)
								q->putMessage(m);
						}
						d->moduleWD.clear();
					}
				}
				break;

			case 10:
				// disconnect db
				d->state = 0;
				d->timer = 0;
				d->initReady = false;
				break;
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	} // while
	return;
}

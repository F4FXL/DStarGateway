/*
 *   Copyright (C) 2011,2012,2013,2018 by Jonathan Naylor G4KLX
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

#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>
#include <linux/sockios.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <unistd.h>

#include "RepeaterHandler.h"
#include "DDHandler.h"
#include "Defs.h"
#include "Log.h"
#include "StringUtils.h"

const unsigned int ETHERNET_ADDRESS_LENGTH = 6U;
const unsigned int ETHERNET_MTU            = 1500U;
const unsigned int BUFFER_LENGTH           = 2000U;

const unsigned int MIN_HEARD_TIME_SECS     = 120U;

const int MINIMUM_DD_FRAME_LENGTH = 60;

const unsigned char ETHERNET_BROADCAST_ADDRESS[] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};
// Multicast address '01:00:5E:00:00:01' - IP: '224.0.0.1' (send to all)
const unsigned char TOALL_MULTICAST_ADDRESS[] = {0x01U, 0x00U, 0x5EU, 0x00U, 0x00U, 0x01U};
// Multicast address '01:00:5E:00:00:23' - IP: '224.0.0.35' (DX-Cluster)
const unsigned char DX_MULTICAST_ADDRESS[] = {0x01U, 0x00U, 0x5EU, 0x00U, 0x00U, 0x23U};

CIRCDDB*       CDDHandler::m_irc          = NULL;
CHeaderLogger* CDDHandler::m_headerLogger = NULL;
int            CDDHandler::m_fd           = -1;
unsigned int   CDDHandler::m_maxRoutes    = 0U;
CEthernet**    CDDHandler::m_list         = NULL;
unsigned char* CDDHandler::m_buffer       = NULL;
bool           CDDHandler::m_logEnabled   = false;
std::string       CDDHandler::m_logDir       = "";
std::string       CDDHandler::m_name         = "";
CTimer         CDDHandler::m_timer        = CTimer(1000U, MIN_HEARD_TIME_SECS);

CEthernet::CEthernet(const unsigned char* address, const std::string& callsign) :
m_address(NULL),
m_callsign(callsign)
{
	assert(address != NULL);
	assert(!callsign.empty());

	m_address = new unsigned char[ETHERNET_ADDRESS_LENGTH];
	::memcpy(m_address, address, ETHERNET_ADDRESS_LENGTH);
}

CEthernet::~CEthernet()
{
	delete[] m_address;
}

unsigned char* CEthernet::getAddress() const
{
	return m_address;
}

std::string CEthernet::getCallsign() const
{
	return m_callsign;
}

void CDDHandler::initialise(unsigned int maxRoutes, const std::string& name)
{
	assert(maxRoutes > 0U);

	m_maxRoutes = maxRoutes;
	m_name      = name;

	m_buffer = new unsigned char[BUFFER_LENGTH];

	m_list = new CEthernet*[maxRoutes];
	for (unsigned int i = 0U; i < maxRoutes; i++)
		m_list[i] = NULL;

	// Add a dummy entry for broadcasts
	m_list[0] = new CEthernet(ETHERNET_BROADCAST_ADDRESS, "        ");
	// Add a dummy entry for "to all" multicast
	m_list[1] = new CEthernet(TOALL_MULTICAST_ADDRESS, "CQCQCQ  ");
	// Add a dummy entry for "DX-Cluster" multicast
	m_list[2] = new CEthernet(DX_MULTICAST_ADDRESS, "CQCQCQ  ");

#if defined(__linux__)
	m_fd = ::open("/dev/net/tun", O_RDWR);
	if (m_fd < 0) {
		wxLogError("Cannot open /dev/net/tun");
		return;
	}

	struct ifreq ifr1;
	::memset(&ifr1, 0x00, sizeof(struct ifreq));

	ifr1.ifr_flags = IFF_TAP | IFF_NO_PI;
	::strcpy(ifr1.ifr_name, "tap%d");

	if (::ioctl(m_fd, TUNSETIFF, (void *)&ifr1) < 0) {
		wxLogError("TUNSETIFF ioctl failed, closing the tap device");
		::close(m_fd);
		m_fd = -1;
		return;
	}

	std::string device = std::string(ifr1.ifr_name);
	wxLogMessage("DD mode Tap interface created on %s", device.c_str());

	int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		wxLogError("Unable to open the config socket, closing the tap device");
		::close(m_fd);
		m_fd = -1;
		return;
	}

	struct ifreq ifr2;
	::memset(&ifr2, 0x00, sizeof(struct ifreq));
	::strcpy(ifr2.ifr_name, ifr1.ifr_name);

	ifr2.ifr_flags = IFF_UP | IFF_BROADCAST | IFF_MULTICAST;
	if (::ioctl(fd, SIOCSIFFLAGS, (void *)&ifr2) < 0) {
		wxLogError("SIOCSIFFLAGS ioctl failed, closing the tap device");
		::close(m_fd);
		m_fd = -1;
		return;
	}

	::close(fd);
#endif
}

void CDDHandler::setLogging(bool enabled, const std::string& dir)
{
	m_logDir     = dir;
	m_logEnabled = enabled;
}

void CDDHandler::setHeaderLogger(CHeaderLogger* logger)
{
	m_headerLogger = logger;
}

void CDDHandler::setIRC(CIRCDDB* irc)
{
	assert(irc != NULL);

	m_irc = irc;
}

void CDDHandler::process(CDDData& data)
{
	// If we're not initialised, return immediately
	if (m_maxRoutes == 0U)
		return;

	unsigned char flag1 = data.getFlag1();
	unsigned char flag2 = data.getFlag2();
	unsigned char flag3 = data.getFlag3();
	std::string myCall1    = data.getMyCall1();
	std::string myCall2    = data.getMyCall2();
	std::string yourCall   = data.getYourCall();
	std::string rptCall1   = data.getRptCall1();
	std::string rptCall2   = data.getRptCall2();

	if (!m_timer.isRunning() || m_timer.hasExpired()) {
		// Write to Header.log if it's enabled
		if (m_headerLogger != NULL)
			m_headerLogger->write("Repeater", data);

		if (m_irc != NULL) {
			m_irc->sendHeardWithTXMsg(myCall1, myCall2, yourCall, rptCall1, rptCall2, flag1, flag2, flag3, "", "Digital Data        ");
			m_irc->sendHeardWithTXStats(myCall1, myCall2, yourCall, rptCall1, rptCall2, flag1, flag2, flag3, 1, 0, -1);
		}

		m_timer.start();
	}

	// Can we continue?
	if (m_fd < 0)
		return;

	unsigned char* address = data.getSourceAddress();

	bool found = false;
	for (unsigned int i = 0U; i < m_maxRoutes; i++) {
		if (m_list[i] != NULL) {
			unsigned char* addr = m_list[i]->getAddress();

			if (::memcmp(addr, address, ETHERNET_ADDRESS_LENGTH) == 0) {
				found = true;
				break;
			}
		}
	}

	if (!found) {
		wxLogMessage("Adding DD user %s with ethernet address %02X:%02X:%02X:%02X:%02X:%02X", myCall1.c_str(),
			address[0], address[1], address[2], address[3], address[4], address[5]);

		CEthernet* ethernet = new CEthernet(address, myCall1);

		found = false;
		for (unsigned int i = 0U; i < m_maxRoutes; i++) {
			if (m_list[i] == NULL) {
				m_list[i] = ethernet;
				found = true;
				if (m_logEnabled)
					writeStatus(*ethernet);
				break;
			}
		}

		if (!found) {
			wxLogError("No space to add new DD ethernet address");
			delete ethernet;
			return;
		}
	}

#if defined(__linux__)
	unsigned int length = data.getEthernetFrame(m_buffer, BUFFER_LENGTH);

	ssize_t len = ::write(m_fd, (char*)m_buffer, length);
	if (len != ssize_t(length))
		wxLogError("Error returned from write()");
#endif
}

CDDData* CDDHandler::read()
{
	// If we're not initialised, return immediately
	if (m_maxRoutes == 0U)
		return NULL;

#if defined(__WINDOWS__)
	return NULL;
#else
	// Check that the read() won't block
	fd_set readFds;
	FD_ZERO(&readFds);
#if defined(__WINDOWS__)
	FD_SET((unsigned int)m_fd, &readFds);
#else
	FD_SET(m_fd, &readFds);
#endif

	// Return immediately
	timeval tv;
	tv.tv_sec  = 0L;
	tv.tv_usec = 0L;

	int ret = ::select(m_fd + 1, &readFds, NULL, NULL, &tv);
	if (ret < 0) {
		wxLogError("Error returned from select()");
		return NULL;
	}

#if defined(__WINDOWS__)
	if (!FD_ISSET((unsigned int)m_fd, &readFds))
		return NULL;
#else
	if (!FD_ISSET(m_fd, &readFds))
		return NULL;
#endif

	// Ensure that the minimum length is padded with 0x00s
	::memset(m_buffer, 0x00U, MINIMUM_DD_FRAME_LENGTH);

	ssize_t len = ::read(m_fd, (char*)m_buffer, BUFFER_LENGTH);
	if (len <= 0) {
		wxLogError("Error returned from read()");
		return NULL;
	}

	// There seems to be a minimum size with DD mode, so pad with zeroes if it's not reached
	if (len < MINIMUM_DD_FRAME_LENGTH)
		len = MINIMUM_DD_FRAME_LENGTH;

	// Point to the destination ethernet address
	unsigned char* address = m_buffer + 0U;

	// Do destination address to callsign lookup
	CEthernet* ethernet = NULL;
	for (unsigned int i = 0U; i < m_maxRoutes; i++) {
		if (m_list[i] != NULL) {
			unsigned char* addr = m_list[i]->getAddress();

			if (::memcmp(addr, address, ETHERNET_ADDRESS_LENGTH) == 0) {
				ethernet = m_list[i];
				break;
			}
		}
	}

	if (ethernet == NULL) {
		wxLogWarning("Cannot find the ethernet address of %02X:%02X:%02X:%02X:%02X:%02X in the ethernet list", address[0], address[1], address[2], address[3], address[4], address[5]);
		return NULL;
	}

	CRepeaterHandler* handler = CRepeaterHandler::findDDRepeater();
	if (handler == NULL) {
		wxLogWarning("Incoming DD data to unknown repeater");
		return NULL;
	}

	// wxLogMessage("Mapping ethernet address %02X:%02X:%02X:%02X:%02X:%02X to user %s",
	//				address[0], address[1], address[2], address[3], address[4], address[5],
	//				ethernet->getCallsign().c_str());

	CDDData* data = new CDDData;
	data->setEthernetFrame(m_buffer, len);
	data->setYourCall(ethernet->getCallsign());

	handler->process(*data);

	return data;
#endif
}

void CDDHandler::clock(unsigned int ms)
{
	m_timer.clock(ms);
}

void CDDHandler::finalise()
{
#if !defined(WIN32)
	if (m_fd >= 0) {
		::close(m_fd);
		m_fd = -1;
	}
#endif

	delete[] m_buffer;

	for (unsigned int i = 0U; i < m_maxRoutes; i++)
		delete m_list[i];
	delete[] m_list;
}

void CDDHandler::writeStatus(const CEthernet& ethernet)
{
	std::string fullName = DDMODE_BASE_NAME;

	if (!m_name.empty()) {
		fullName += "_";
		fullName += m_name;
	}

	fullName = m_logDir + "/" + fullName + ".log";

	std::ofstream file;
	file.open(fullName, std::ios::app);
	if (!file.is_open()) {
		wxLogError("Unable to open %s for writing", fullName.c_str());
		return;
	}

	std::string callsign = ethernet.getCallsign();
	unsigned char* address = ethernet.getAddress();

	time_t timeNow = ::time(NULL);
	struct tm* tm = ::gmtime(&timeNow);

	std::string text = string_format("%04d-%02d-%02d %02d:%02d:%02d: %02X:%02X:%02X:%02X:%02X:%02X %s\n",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec,
		address[0], address[1], address[2], address[3], address[4], address[5],	callsign.c_str());

	file << text;

	file.close();
}

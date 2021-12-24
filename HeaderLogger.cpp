/*
 *   Copyright (C) 2010,2011,2012,2014 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include "HeaderLogger.h"
#include "Defs.h"
#include "Log.h"
#include "StringUtils.h"

#if !defined(__WXMSW__)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif


CHeaderLogger::CHeaderLogger(const std::string& dir, const std::string& name) :
m_dir(dir),
m_name(name),
m_file()
{
}

CHeaderLogger::~CHeaderLogger()
{
}

bool CHeaderLogger::open()
{
	std::string fullName = HEADERS_BASE_NAME;

	if (!m_name.empty()) {
		fullName += "_";
		fullName += m_name;
	}

	fullName = m_dir + "/" + fullName + ".log";

	m_file.open(fullName, std::ios::app);
	if (!m_file.is_open()) {
		wxLogError("Cannot open %s file for appending", fullName.c_str());
		return false;
	}

	return true;
}

void CHeaderLogger::write(const std::string& type, const CHeaderData& header)
{
	assert(!type.empty());

	time_t timeNow = ::time(NULL);
	struct tm* tm = ::gmtime(&timeNow);

	char* t = ::inet_ntoa(header.getYourAddress());
	std::string address(t);

	std::string text;
	string_format("%04d-%02d-%02d %02d:%02d:%02d: %s header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X (%s:%u)\n",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, type,
		header.getMyCall1().c_str(), header.getMyCall2().c_str(), header.getYourCall().c_str(),
		header.getRptCall1().c_str(), header.getRptCall2().c_str(), header.getFlag1(), header.getFlag2(),
		header.getFlag3(), address.c_str(), header.getYourPort());

	m_file << text;
	m_file.flush();
}

void CHeaderLogger::write(const std::string& type, const CDDData& data)
{
	assert(!type.empty());

	time_t timeNow = ::time(NULL);
	struct tm* tm = ::gmtime(&timeNow);

	std::string text;
	string_format("%04d-%02d-%02d %02d:%02d:%02d: %s header - My: %s/%s  Your: %s  Rpt1: %s  Rpt2: %s  Flags: %02X %02X %02X\n",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, type,
		data.getMyCall1().c_str(), data.getMyCall2().c_str(), data.getYourCall().c_str(),
		data.getRptCall1().c_str(), data.getRptCall2().c_str(), data.getFlag1(), data.getFlag2(),
		data.getFlag3());

	m_file << text;
	m_file.flush();
}

void CHeaderLogger::close()
{
	m_file.close();
}

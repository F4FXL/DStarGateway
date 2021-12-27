/*
 *   Copyright (C) 2010-2013 by Jonathan Naylor G4KLX
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

#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

#include "DStarDefines.h"
#include "HostFile.h"
#include "Log.h"


CHostFile::CHostFile(const std::string& fileName, bool logging) :
m_names(),
m_addresses(),
m_locks()
{
	std::string delimiters = " \t\r\n";
	std::ifstream file;
	file.open(fileName);
	if (!file.is_open())
		return;

	if (logging)
		CLog::logInfo("Reading %s", fileName.c_str());

	while(!file.eof()) {
		std::string line;
		std::getline(file, line);

		if (line.length() > 0 && line[0] != '#') {
			char * lineDup = strdup(line.c_str());

			char * t = std::strtok(lineDup, delimiters.c_str());
			std::string name(t);
			t = std::strtok(NULL, delimiters.c_str());
			std::string address(t);
			t = std::strtok(NULL, delimiters.c_str());
			std::string lock(t);

			free(lineDup);

			name.resize(LONG_CALLSIGN_LENGTH, ' ');

			if (!name.empty() && !address.empty()) {
				m_names.push_back(name);
				m_addresses.push_back(address);
				m_locks.push_back(!lock.empty());
			}
		}
	}

	file.close();
}

CHostFile::~CHostFile()
{
}

unsigned int CHostFile::getCount() const
{
	return m_names.size();
}

std::string CHostFile::getName(unsigned int n) const
{
	return m_names[n];
}

std::string CHostFile::getAddress(unsigned int n) const
{
	return m_addresses[n];
}

std::string CHostFile::getAddress(const std::string& host) const
{
	std::string name = host;
	name.resize(LONG_CALLSIGN_LENGTH, ' ');

	for(unsigned int i = 0; i < m_names.size(); i++) {
		if(m_names[i] == name) {
			return m_addresses[i];
		}
	}

	return "";
}

bool CHostFile::getLock(unsigned int n) const
{
	return m_locks[n];
}

std::vector<std::string> CHostFile::getNames() const
{
	return m_names;
}

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

#ifndef HostFile_H
#define HostFile_H

#include <vector>
#include <string>

class CHostFile {
public:
	CHostFile(const std::string& fileName, bool logging);
	~CHostFile();

	unsigned int getCount() const;
	std::string     getName(unsigned int n) const;
	std::string     getAddress(unsigned int n) const;
	std::string     getAddress(const std::string& host) const;
	bool         getLock(unsigned int n) const;

	std::vector<std::string> getNames() const;

private:
	std::vector<std::string> m_names;
	std::vector<std::string> m_addresses;	
	std::vector<bool> m_locks;
};

#endif

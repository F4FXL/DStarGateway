/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
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

#ifndef	RemoteControlCallsignData_H
#define	RemoteControlCallsignData_H

#include <string>
#include <vector>

class CRemoteControlCallsignData {
public:
	CRemoteControlCallsignData();
	~CRemoteControlCallsignData();

	void addRepeater(const std::string& callsign);
	void addStarNet(const std::string& callsign);

	unsigned int getRepeaterCount() const;
	unsigned int getStarNetCount() const;

	std::string getRepeater(unsigned int n) const;
	std::string getStarNet(unsigned int n) const;

private:
	std::vector<std::string> m_repeaters;
	std::vector<std::string> m_starNets;
};

#endif

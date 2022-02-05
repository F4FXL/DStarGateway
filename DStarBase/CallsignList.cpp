/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

#include <cstdio>
#include "CallsignList.h"
#include "DStarDefines.h"
#include "Utils.h"

CCallsignList::CCallsignList(const std::string& filename) :
m_filename(filename),
m_callsigns()
{
}


CCallsignList::~CCallsignList()
{
	m_callsigns.clear();
}

bool CCallsignList::load()
{
	FILE *file = fopen(m_filename.c_str(), "r");
	if (NULL == file)
		return false;

	char cstr[32];

	while (fgets(cstr, 32, file)) {
		std::string callsign(cstr);
		CUtils::ToUpper(callsign);
		callsign.resize(LONG_CALLSIGN_LENGTH, ' ');

		m_callsigns.insert(callsign);
	}

	fclose(file);

	return true;
}

unsigned int CCallsignList::getCount() const
{
	return m_callsigns.size();
}

bool CCallsignList::isInList(const std::string& callsign) const
{
	return m_callsigns.find(callsign) != m_callsigns.end();
}

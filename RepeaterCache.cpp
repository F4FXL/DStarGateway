/*
 *   Copyright (C) 2010 by Jonathan Naylor G4KLX
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

#include "RepeaterCache.h"

CRepeaterCache::CRepeaterCache()
{
}

CRepeaterCache::~CRepeaterCache()
{
	for (std::unordered_map<std::string, CRepeaterRecord *>::iterator it = m_cache.begin(); it != m_cache.end(); ++it)
		delete it->second;
}

CRepeaterRecord* CRepeaterCache::find(const std::string& repeater)
{
	return m_cache[repeater];
}

void CRepeaterCache::update(const std::string& repeater, const std::string& gateway)
{
	CRepeaterRecord* rec = m_cache[repeater];

	if (rec == NULL)
		// A brand new record is needed
		m_cache[repeater] = new CRepeaterRecord(repeater, gateway);
	else
		// Update an existing record
		rec->setGateway(gateway);
}

unsigned int CRepeaterCache::getCount() const
{
	return m_cache.size();
}

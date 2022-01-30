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

#pragma once

#include <string>
#include <unordered_map>

class CUserRecord {
public:
	CUserRecord(const std::string& user, const std::string& repeater, const std::string& timestamp) :
	m_user(user),
	m_repeater(repeater),
	m_timestamp(timestamp)
	{
	}

	std::string getUser() const
	{
		return m_user;
	}

	std::string getRepeater() const
	{
		return m_repeater;
	}

	std::string getTimeStamp() const
	{
		return m_timestamp;
	}

	void setRepeater(const std::string& repeater)
	{
		m_repeater = repeater;
	}

	void setTimestamp(const std::string& timestamp)
	{
		m_timestamp = timestamp;
	}

private:
	std::string m_user;
	std::string m_repeater;
	std::string m_timestamp;
};

class CUserCache {
public:
	CUserCache();
	~CUserCache();

	CUserRecord* find(const std::string& user);

	void update(const std::string& user, const std::string& repeater, const std::string& timestamp);

	unsigned int getCount() const;

private:
	std::unordered_map<std::string, CUserRecord *> m_cache;
};

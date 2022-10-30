/*
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
#include <vector>

#include "AMBEData.h"

class CIndexRecord {
public:
	CIndexRecord(const std::string& name, unsigned int start, unsigned int length) :
	m_name(name),
	m_start(start),
	m_length(length)
	{
	}

	std::string getName() const
	{
		return m_name;
	}

	unsigned int getStart() const
	{
		return m_start;
	}

	unsigned int getLength() const
	{
		return m_length;
	}

private:
	std::string     m_name;
	unsigned int m_start;
	unsigned int m_length;
};

class CAMBEFileReader
{
public:
    CAMBEFileReader(const std::string& indexFile, const std::string& ambeFile);
    ~CAMBEFileReader();
    bool read();
    bool lookup(const std::string &id, std::vector<CAMBEData *>& data);

private:
    bool readAmbe();
    bool readIndex();

    std::string      m_indexFile;
    std::string      m_ambeFile;
    unsigned char*   m_ambe;
    unsigned int     m_ambeLength;
    std::unordered_map<std::string, CIndexRecord*> m_index;
};
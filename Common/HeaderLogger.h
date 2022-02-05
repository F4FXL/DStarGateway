/*
 *   Copyright (C) 2010-2013 by Jonathan Naylor G4KLX
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

#ifndef	HeaderLogger_H
#define	HeaderLogger_H

#include <iostream>
#include <fstream>
#include <string>

#include "HeaderData.h"
#include "DDData.h"

class CHeaderLogger {
public:
	CHeaderLogger(const std::string& dir, const std::string& name = "");
	~CHeaderLogger();

	bool open();

	void write(const std::string& type, const CHeaderData& header);
	void write(const std::string& type, const CDDData& header);

	void close();

private:
	std::string m_dir;
	std::string m_name;
	std::ofstream m_file;
};

#endif

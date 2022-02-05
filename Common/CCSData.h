/*
 *   Copyright (C) 2013 by Jonathan Naylor G4KLX
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

#include <netinet/in.h>
#include <string>

enum CC_TYPE {
	CT_TERMINATE,
	CT_DTMFNOTFOUND,
	CT_DTMFFOUND,
	CT_INFO
};

class CCCSData {
public:
	CCCSData(const std::string& local, double latitude, double longitude, double frequency, double offset, const std::string& description1, const std::string& description2, const std::string& url, CC_TYPE type);
	CCCSData(const std::string& local, const std::string& remote, CC_TYPE type);
	CCCSData();
	~CCCSData();

	bool setCCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);

	unsigned int getCCSData(unsigned char* data, unsigned int length) const;

	void setDestination(const in_addr& address, unsigned int port);

	std::string  getLocal() const;
	std::string  getRemote() const;
	CC_TYPE      getType() const;

	in_addr      getYourAddress() const;
	unsigned int getYourPort() const;
	unsigned int getMyPort() const;

private:
	std::string  m_local;
	std::string  m_remote;
	double       m_latitude;
	double       m_longitude;
	double       m_frequency;
	double       m_offset;
	std::string  m_description1;
	std::string  m_description2;
	std::string  m_url;
	CC_TYPE      m_type;
	in_addr      m_yourAddress;
	unsigned int m_yourPort;
	unsigned int m_myPort;
};

/*
 *   Copyright (C) 2010,2012,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
 *   Copyright (c) 2021-2023 by Geoffrey Merck F4FXL / KC3FRA
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
#include <netinet/in.h>

#include "Defs.h"

enum CD_TYPE {
	CT_LINK1,
	CT_LINK2,
	CT_UNLINK,
	CT_ACK,
	CT_NAK
};

class CConnectData {
public:
	CConnectData(GATEWAY_TYPE gatewayType, const std::string& repeater, const std::string& reflector, CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort = 0U);
	CConnectData(const std::string& repeater, const std::string& reflector, CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort = 0U);
	CConnectData(const std::string& repeater, const sockaddr_storage& yourAddressAndPort, unsigned int myPort = 0U);
	CConnectData(const std::string& repeater, CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort = 0U);
	CConnectData(CD_TYPE type, const sockaddr_storage& yourAddressAndPort, unsigned int myPort = 0U);

	CConnectData(GATEWAY_TYPE gatewayType, const std::string& repeater, const std::string& reflector, CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CConnectData(const std::string& repeater, const std::string& reflector, CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CConnectData(const std::string& repeater, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CConnectData(const std::string& repeater, CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CConnectData(CD_TYPE type, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort = 0U);
	CConnectData();
	~CConnectData();

	bool setDExtraData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);
	bool setDPlusData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);
	bool setDCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);
	bool setCCSData(const unsigned char* data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort, unsigned int myPort);

	unsigned int getDExtraData(unsigned char* data, unsigned int length) const;
	unsigned int getDPlusData(unsigned char* data, unsigned int length) const;
	unsigned int getDCSData(unsigned char* data, unsigned int length) const;
	unsigned int getCCSData(unsigned char* data, unsigned int length) const;

	std::string   getRepeater() const;
	std::string   getReflector() const;
	CD_TYPE       getType() const;

	sockaddr_storage getYourAddressAndPort() const;
	unsigned int  getMyPort() const;

	void setLocator(const std::string& locator);

private:
	GATEWAY_TYPE  m_gatewayType;
	std::string   m_repeater;
	std::string   m_reflector;
	CD_TYPE       m_type;
	std::string   m_locator;
	sockaddr_storage m_yourAddressAndPort;
	unsigned int  m_myPort;
};


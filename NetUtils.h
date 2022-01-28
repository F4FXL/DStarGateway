/*
 *   Copyright (c) 2022 by Geoffrey Merck F4FXL / KC3FRA
 *   Copyright (C) 2009-2011,2013,2015,2016,2020 by Jonathan Naylor G4KLX
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
#include <netdb.h>
#include <sys/socket.h>

#define TOIPV6(s) ((struct sockaddr_in6*)&s)
#define TOIPV4(s) (((struct sockaddr_in*)&s))
#define GETPORT(s) (s.ss_family == AF_INET6 ? TOIPV6(s)->sin6_port : TOIPV4(s)->sin_port)
#define SETPORT(s, p) (if(s.ss_family == AF_INET6)TOIPV6(s)->sin6_port = p;else TOIPV4(s)->sin_port = p;)

enum IPMATCHTYPE {
	IMT_ADDRESS_AND_PORT,
	IMT_ADDRESS_ONLY
};

class CNetUtils
{
public:
    static bool lookupV6(const std::string& hostname, sockaddr_storage& addr);
    static bool lookupV4(const std::string& hostname, sockaddr_storage& addr);
    static bool lookup(const std::string& hostname, sockaddr_storage& addr);
    static bool lookup(const std::string& hostname, sockaddr_storage& addr, struct addrinfo& hints);
    static bool match(const sockaddr_storage& addr1, const sockaddr_storage& addr2, IPMATCHTYPE type);
    static void setPort(struct sockaddr_storage& addr, in_port_t port);
};
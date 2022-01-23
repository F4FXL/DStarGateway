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

#include <sys/types.h>
#include <cstring>

#include "NetUtils.h"

#define TOIPV6(s) ((struct sockaddr_in6*)&s)
#define TOIPV4(s) (((struct sockaddr_in*)&s))

bool CNetUtils::lookupV4(const std::string& hostname, sockaddr_storage& addr)
{
    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;

    return lookup(hostname, addr, hints);
}

bool CNetUtils::lookupV6(const std::string& hostname, sockaddr_storage& addr)
{
    struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;

    return lookup(hostname, addr, hints);
}

bool CNetUtils::lookup(const std::string& hostname, sockaddr_storage& addr)
{
   struct addrinfo hints;
    ::memset(&hints, 0, sizeof(hints));
    return lookup(hostname, addr, hints);
}

bool CNetUtils::lookup(const std::string& hostname, sockaddr_storage& addr, struct addrinfo& hints)
{
    struct addrinfo *res;

    int err = getaddrinfo(hostname.c_str(), nullptr, &hints, &res);
    if(err != 0) {
        ::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        lookup("255.255.255.255", addr, hints);
        return false;
    }

    ::memcpy(&addr, res->ai_addr, res->ai_addrlen);

    ::freeaddrinfo(res);

    return true;
}

bool CNetUtils::match(const sockaddr_storage& addr1, const sockaddr_storage& addr2, IPMATCHTYPE type)
{
	if (addr1.ss_family != addr2.ss_family)
		return false;

	if (type == IMT_ADDRESS_AND_PORT) {
		switch (addr1.ss_family) {
		case AF_INET:
			struct sockaddr_in *in_1, *in_2;
			in_1 = (struct sockaddr_in*)&addr1;
			in_2 = (struct sockaddr_in*)&addr2;
			return (in_1->sin_addr.s_addr == in_2->sin_addr.s_addr) && (in_1->sin_port == in_2->sin_port);
		case AF_INET6:
			struct sockaddr_in6 *in6_1, *in6_2;
			in6_1 = (struct sockaddr_in6*)&addr1;
			in6_2 = (struct sockaddr_in6*)&addr2;
			return IN6_ARE_ADDR_EQUAL(&in6_1->sin6_addr, &in6_2->sin6_addr) && (in6_1->sin6_port == in6_2->sin6_port);
		default:
			return false;
		}
	} else if (type == IMT_ADDRESS_ONLY) {
		switch (addr1.ss_family) {
		case AF_INET:
			struct sockaddr_in *in_1, *in_2;
			in_1 = (struct sockaddr_in*)&addr1;
			in_2 = (struct sockaddr_in*)&addr2;
			return in_1->sin_addr.s_addr == in_2->sin_addr.s_addr;
		case AF_INET6:
			struct sockaddr_in6 *in6_1, *in6_2;
			in6_1 = (struct sockaddr_in6*)&addr1;
			in6_2 = (struct sockaddr_in6*)&addr2;
			return IN6_ARE_ADDR_EQUAL(&in6_1->sin6_addr, &in6_2->sin6_addr);
		default:
			return false;
		}
	} else {
		return false;
	}
}

void CNetUtils::setPort(struct sockaddr_storage& addr, in_port_t port)
{
	switch (addr.ss_family)
	{
	case AF_INET:
		TOIPV4(addr)->sin_port = port;
		break;
	case AF_INET6:
		TOIPV6(addr)->sin6_port = port;
	default:
		break;
	}
}

/*
 *   Copyright (c) 2022 by Geoffrey Merck F4FXL / KC3FRA
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
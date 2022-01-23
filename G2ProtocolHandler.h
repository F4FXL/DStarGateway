/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017-2018 by Thomas A. Early N7TAE
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

#include <unordered_map>
#include <sys/socket.h>
#include <boost/container_hash/hash.hpp>

#include "UDPReaderWriter.h"
#include "DStarDefines.h"
#include "HeaderData.h"
#include "AMBEData.h"
#include "NetUtils.h"

enum G2_TYPE {
	GT_NONE,
	GT_HEADER,
	GT_AMBE
};

// struct sockaddr_storage_map {
//     struct comp {
//         bool operator() (const struct sockaddr_storage& a, const struct sockaddr_storage& b) const {
//             return CNetUtils::match(a, b, IMT_ADDRESS_AND_PORT);
//         }
//     };
//     struct hash {
//         std::size_t operator() (const sockaddr_storage& a) const {
// 			switch(a.ss_family)
// 			{
// 				case AF_INET: {
// 					auto ptr4 = ((struct sockaddr_in *)&a);
// 					size_t res = AF_INET;
// 					boost::hash_combine(res, ptr4->sin_addr.s_addr);
// 					boost::hash_combine(res, ptr4->sin_port);
// 					return res;
// 				}
// 				case AF_INET6: {
// 					auto ptr6 = ((struct sockaddr_in6 *)&a);
// 					size_t res = AF_INET6;
// 					boost::hash_combine(res, ptr6->sin6_port);
// 					boost::hash_combine(res, ptr6->sin6_addr);
// 					return res;
// 				}
// 				default:
// 					return 0U;
// 			}
//         }
//     };
// };

class CG2ProtocolHandler {
public:
	CG2ProtocolHandler(unsigned int port, const std::string& addr = std::string(""));
	~CG2ProtocolHandler();

	bool open();

	bool writeHeader(const CHeaderData& header);
	bool writeAMBE(const CAMBEData& data);

	G2_TYPE read();
	CHeaderData* readHeader();
	CAMBEData*   readAMBE();

	void close();
	void traverseNat(const std::string& address);

private:
	std::unordered_map<uint32_t, unsigned int> m_portmap;

	CUDPReaderWriter m_socket;
	G2_TYPE          m_type;
	unsigned char*   m_buffer;
	unsigned int     m_length;
	in_addr          m_address;
	unsigned int     m_port;

	bool readPackets();
};

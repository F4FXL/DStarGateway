/*
 *	Copyright (C) 2009,2013 by Jonathan Naylor, G4KLX
 *  Copyright (c) 2017 by Thomas A. Early N7TAE
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 */

#pragma once

#include <sys/socket.h>
#include <string>
#include <vector>

enum TRISTATE {
	STATE_FALSE,
	STATE_TRUE,
	STATE_UNKNOWN
};

class CUtils {
public:
	static void						dump(const char* title, const bool* data, unsigned int length);
	static void						dumpRev(const char* title, const bool* data, unsigned int length);
	static void						dump(const char* title, const unsigned char* data, unsigned int length);
	static unsigned char			bitsToByte(const bool* bits);
	static unsigned char			bitsToByteRev(const bool* bits);
	static void						byteToBits(unsigned char byte, bool* bits);
	static void						byteToBitsRev(unsigned char byte, bool* bits);
	static std::string				latLonToLoc(double latitude, double longitude);
	static void						clean(std::string& str, const std::string& allowed);
	static std::string				ToUpper(std::string &str);
	static std::string				ToLower(std::string &str);
	static std::string				Trim(std::string &str);
	static int						getAllIPV4Addresses(const char *name, unsigned short port, unsigned int *num, struct sockaddr_in *addr, unsigned int max_addr);
	static void						safeStringCopy(char * dest, const char * src, unsigned int buf_size);
	static std::vector<std::string>	stringTokenizer(const std::string &s);
	static std::string				getCurrentTime(void);
	static void						ReplaceChar(std::string &str, char from, char to);
	static time_t					parseTime(const std::string str);
};

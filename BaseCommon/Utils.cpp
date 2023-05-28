/*
 *	Copyright (C) 2009,2013 Jonathan Naylor, G4KLX
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
 
#include <sys/types.h>
#include <netdb.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <string>
#include <cctype>
#include <iterator>
#include <cstring>
#include <sstream>
#include <iostream>
#include <fstream>

#include "Utils.h"
#include "Log.h"

void CUtils::dump(const char* title, const bool* data, unsigned int length)
{
	assert(title != NULL);
	assert(data != NULL);

	CLog::logInfo("%s\n", title);

	unsigned int offset = 0U;

	while (offset < length) {
		std::string output;

		unsigned char buffer[16];
		unsigned int bytes = 0U;
		for (unsigned int bits = 0U; bits < 128U && (offset + bits) < length; bits += 8U)
			buffer[bytes++] = bitsToByte(data + offset + bits);

		for (unsigned i = 0U; i < bytes; i++) {
			char temp[4];
			sprintf(temp, "%02X ", buffer[i]);
			output += temp;
		}

		for (unsigned int i = bytes; i < 16U; i++)
			output += "   ";

		output += "   *";

		for (unsigned i = 0U; i < bytes; i++) {
			unsigned char c = buffer[i];

			if (::isprint(c))
				output += char(c);
			else
				output += ".";
		}

		output += "*'";

		CLog::logDebug("%04X:  %s\n", offset / 8U, output.c_str());

		offset += 128U;
	}
}

void CUtils::dumpRev(const char* title, const bool* data, unsigned int length)
{
	assert(title != NULL);
	assert(data != NULL);

	CLog::logInfo("%s\n", title);

	unsigned int offset = 0U;

	while (offset < length) {
		std::string output;

		unsigned char buffer[16];
		unsigned int bytes = 0U;
		for (unsigned int bits = 0U; bits < 128U && (offset + bits) < length; bits += 8U)
			buffer[bytes++] = bitsToByteRev(data + offset + bits);

		for (unsigned i = 0U; i < bytes; i++) {
			char temp[4];
			sprintf(temp, "%02X ", buffer[i]);
			output += temp;
		}

		for (unsigned int i = bytes; i < 16U; i++)
			output += "   ";

		output += "   *";

		for (unsigned i = 0U; i < bytes; i++) {
			unsigned char c = buffer[i];

			if (::isprint(c))
				output += char(c);
			else
				output += ".";
		}

		output += "*";

		CLog::logDebug("%04X:  %s\n", offset / 8U, output.c_str());

		offset += 128U;
	}
}

void CUtils::dump(const char* title, const unsigned char* data, unsigned int length)
{
	assert(title != NULL);
	assert(data != NULL);

	CLog::logInfo("%s\n", title);

	unsigned int offset = 0U;

	while (length > 0U) {
		std::string output;

		unsigned int bytes = (length > 16U) ? 16U : length;

		for (unsigned i = 0U; i < bytes; i++) {
			char temp[4];
			sprintf(temp, "%02X ", data[offset + i]);
			output += temp;
		}

		for (unsigned int i = bytes; i < 16U; i++)
			output += "   ";

		output += "   *";

		for (unsigned i = 0U; i < bytes; i++) {
			unsigned char c = data[offset + i];

			if (::isprint(c))
				output += char(c);
			else
				output += ".";
		}

		output += "*";

		CLog::logDebug("%04X:  %s\n", offset, output.c_str());

		offset += 16U;

		if (length >= 16U)
			length -= 16U;
		else
			length = 0U;
	}
}

unsigned char CUtils::bitsToByte(const bool* bits)
{
	assert(bits != NULL);

	unsigned char val = 0x00;

	for (unsigned int i = 0U; i < 8U; i++) {
		val <<= 1;

		if (bits[i])
			val |= 0x01;
	}

	return val;
}

unsigned char CUtils::bitsToByteRev(const bool* bits)
{
	assert(bits != NULL);

	unsigned char val = 0x00;

	for (unsigned int i = 0U; i < 8U; i++) {
		val >>= 1;

		if (bits[i])
			val |= 0x80;
	}

	return val;
}

void CUtils::byteToBits(unsigned char byte, bool* data)
{
	assert(data != NULL);

	unsigned char mask = 0x80U;
	for (unsigned int i = 0U; i < 8U; i++, mask >>= 1)
		data[i] = byte & mask ? true : false;
}

void CUtils::byteToBitsRev(unsigned char byte, bool* data)
{
	assert(data != NULL);

	unsigned char mask = 0x01U;
	for (unsigned int i = 0U; i < 8U; i++, mask <<= 1)
		data[i] = byte & mask ? true : false;
}

std::string CUtils::latLonToLoc(double latitude, double longitude)
{
	if (latitude < -90.0 || latitude > 90.0)
		return std::string();

	if (longitude < -360.0 || longitude > 360.0)
		return std::string();

	latitude += 90.0;

	if (longitude > 180.0)
		longitude -= 360.0;

	if (longitude < -180.0)
		longitude += 360.0;

	longitude += 180.0;

	char locator[6U];

	double lon = floor(longitude / 20.0);
	double lat = floor(latitude / 10.0);

	locator[0U] = 'A' + (unsigned int)lon;
	locator[1U] = 'A' + (unsigned int)lat;

	longitude -= lon * 20.0;
	latitude  -= lat * 10.0;

	lon = ::floor(longitude / 2.0);
	lat = ::floor(latitude / 1.0);

	locator[2U] = '0' + (unsigned int)lon;
	locator[3U] = '0' + (unsigned int)lat;

	longitude -= lon * 2.0;
	latitude  -= lat * 1.0;

	lon = ::floor(longitude / (2.0 / 24.0));
	lat = ::floor(latitude / (1.0 / 24.0));

	locator[4U] = 'A' + (unsigned int)lon;
	locator[5U] = 'A' + (unsigned int)lat;

	return std::string(locator);
}

void CUtils::clean(std::string &str, const std::string& allowed)
{
	for (unsigned int i = 0U; i < str.size(); i++) {
		int n = allowed.find(str[i]);
		if (n < 0)
			str[i] = ' ';
	}
}

std::string CUtils::ToUpper(std::string &str)
{
	for (auto it=str.begin(); it!=str.end(); it++) {
		if (islower(*it))
			*it = toupper(*it);
	}
	return str;
}

std::string CUtils::ToLower(std::string &str)
{
	for (auto it=str.begin(); it!=str.end(); it++) {
		if (isupper(*it))
			*it = tolower(*it);
	}
	return str;
}

std::string CUtils::Trim(std::string &str)
{
	while (str.size() && std::isspace(str[0]))
		str.erase(str.begin());
	while (str.size() && std::isspace(str[str.size()-1]))
		str.erase(--str.end());
	return str;
}

void CUtils::ReplaceChar(std::string &str, char from, char to)
{
	for (auto it=str.begin(); it!=str.end(); it++) {
		if (from == *it)
			*it = to;
	}
}

int CUtils::getAllIPV4Addresses(const char *name, unsigned short port, unsigned int *num, struct sockaddr_in *addr, unsigned int max_addr)
{

	struct addrinfo hints;
	struct addrinfo * res;

	memset(&hints, 0x00, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int r = getaddrinfo(name, NULL, &hints, &res);

	if (r == 0) {
		struct addrinfo * rp;
		unsigned int numAddr = 0;

		for (rp = res; rp != NULL; rp = rp->ai_next) {
			if (rp->ai_family == AF_INET)
				numAddr ++;
		}

		if (numAddr > 0) {
			if (numAddr > max_addr)
				numAddr = max_addr;

			int *shuffle = new int[numAddr];

			unsigned int i;

			for (i=0; i < numAddr; i++)
				shuffle[i] = i;

			for (i=0; i < (numAddr - 1); i++) {
				if (rand() & 1) {
					int tmp;
					tmp = shuffle[i];
					shuffle[i] = shuffle[i+1];
					shuffle[i+1] = tmp;
				}
			}

			for (i=(numAddr - 1); i > 0; i--) {
				if (rand() & 1) {
					int tmp;
					tmp = shuffle[i];
					shuffle[i] = shuffle[i-1];
					shuffle[i-1] = tmp;
				}
			}

			for (rp = res, i=0 ; (rp != NULL) && (i < numAddr); rp = rp->ai_next) {
				if (rp->ai_family == AF_INET) {
					memcpy(addr+shuffle[i], rp->ai_addr, sizeof (struct sockaddr_in));
					addr[shuffle[i]].sin_port = htons(port);
					i++;
				}
			}

			delete[] shuffle;
		}

		*num = numAddr;
		freeaddrinfo(res);
		return 0;
	} else {
		std::string e(gai_strerror(r));
		CLog::logInfo("getaddrinfo: %s\n", e.c_str());
		return 1;
	}
}

void CUtils::safeStringCopy(char *dest, const char *src, unsigned int buf_size)
{
	unsigned int i = 0;

	while (i < (buf_size - 1)  &&  (src[i] != 0)) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = 0;
}

std::string CUtils::getCurrentTime(void)
{
	time_t now = time(NULL);
	struct tm* tm;
	struct tm tm_buf;
	char buffer[25];

	tm = gmtime_r(&now, &tm_buf);
	strftime(buffer, 25, "%Y-%m-%d %H:%M:%S", tm);
	return std::string(buffer);
}

std::vector<std::string> CUtils::stringTokenizer(const std::string &s)
{
	std::stringstream ss(s);
	std::istream_iterator<std::string> it(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> result(it, end);
	return result;
}

time_t CUtils::parseTime(const std::string str)
{
	struct tm stm;
	strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &stm);
	return mktime(&stm);
}

void CUtils::truncateFile(const std::string& fileName)
{
	std::fstream file;
	file.open(fileName, std::fstream::trunc);
	if(file.is_open())
		file.close();
}

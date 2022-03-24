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

#include <sys/stat.h>
#include <cstdio>
#include <cstring>

#include "AMBEFileReader.h"
#include "DStarDefines.h"
#include "Log.h"

const unsigned int SILENCE_LENGTH = 10U;

CAMBEFileReader::CAMBEFileReader(const std::string& indexFile, const std::string& ambeFile) :
m_indexFile(indexFile),
m_ambeFile(ambeFile),
m_ambe(nullptr),
m_ambeLength(0U),
m_index()
{

}

CAMBEFileReader::~CAMBEFileReader()
{
	if(m_ambe != nullptr) {
		delete[] m_ambe;
	}
}

bool CAMBEFileReader::read()
{
    bool ret = readAmbe() && readIndex();
    return ret;
}

bool CAMBEFileReader::readAmbe()
{
    struct stat sbuf;
    if (stat(m_ambeFile.c_str(), &sbuf)) {
        CLog::logWarning("File %s not readable\n", m_ambeFile.c_str());
        return false;
    }

	unsigned int fsize = sbuf.st_size;

	FILE *file = fopen(m_ambeFile.c_str(), "rb");
	if (NULL == file) {
		CLog::logError("Cannot open %s for reading\n", m_ambeFile.c_str());
		return false;
	}

	CLog::logInfo("Reading %s\n", m_ambeFile.c_str());

	unsigned char buffer[VOICE_FRAME_LENGTH_BYTES];

	size_t n = fread(buffer, sizeof(unsigned char), 4, file);
	if (n != 4) {
		CLog::logError("Unable to read the header from %s\n", m_ambeFile.c_str());
		fclose(file);
		return false;
	}

	if (memcmp(buffer, "AMBE", 4)) {
		CLog::logError("Invalid header from %s\n", m_ambeFile.c_str());
		fclose(file);
		return false;
	}

	// Length of the file minus the header
	unsigned int length = fsize - 4U;

	// Hold the file data plus silence at the end
	m_ambe = new unsigned char[length + SILENCE_LENGTH * VOICE_FRAME_LENGTH_BYTES];
	m_ambeLength = length / VOICE_FRAME_LENGTH_BYTES;

	// Add silence to the beginning of the buffer
	unsigned char* p = m_ambe;
	for (unsigned int i = 0U; i < SILENCE_LENGTH; i++, p += VOICE_FRAME_LENGTH_BYTES)
		memcpy(p, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

	n = fread(p, 1, length, file);
	if (n != length) {
		CLog::logError("Unable to read the AMBE data from %s\n", m_ambeFile.c_str());
		fclose(file);
		delete[] m_ambe;
        m_ambeLength = 0U;
		m_ambe = nullptr;
		return false;
	}

	fclose(file);

	return true;
}

bool CAMBEFileReader::readIndex()
{
	struct stat sbuf;

    if (stat(m_indexFile.c_str(), &sbuf)) {
        CLog::logError("File %s not readable\n", m_indexFile.c_str());
        return false;
    }

	FILE *file = fopen(m_indexFile.c_str(), "r");
	if (file == nullptr) {
		CLog::logError("Cannot open %s for reading\n", m_indexFile.c_str());
		return false;
	}

	// Add a silence entry at the beginning
	m_index[" "] = new CIndexRecord(" ", 0, SILENCE_LENGTH);

	CLog::logInfo("Reading %s\n", m_indexFile.c_str());

	char line[128];
	while (fgets(line, 128, file)) {
		if (strlen(line) && '#'!=line[0]) {
			const std::string space(" \t\r\n");
			std::string name(strtok(line, space.c_str()));
			std::string strt(strtok(NULL, space.c_str()));
			std::string leng(strtok(NULL, space.c_str()));

			if (name.size() && strt.size() && leng.size()) {
				unsigned long start  = std::stoul(strt);
				unsigned long length = std::stoul(leng);

				if (start >= m_ambeLength || (start + length) >= m_ambeLength)
					CLog::logInfo("The start or end for *%s* is out of range, start: %lu, end: %lu\n", name.c_str(), start, start + length);
				else
					m_index[name] = new CIndexRecord(name, start + SILENCE_LENGTH, length);
			}
		}
	}

	fclose(file);

	return true;
}

bool CAMBEFileReader::lookup(const std::string &id, std::vector<CAMBEData *>& data)
{
	if(m_index.count(id) == 0U) {
		CLog::logError("Cannot find the AMBE index for *%s*", id.c_str());
		return false;
	}

	CIndexRecord* info = m_index[id];
	unsigned int  start = info->getStart();
	unsigned int length = info->getLength();

	for (unsigned int i = 0U; i < length; i++) {
		unsigned char* dataIn = m_ambe + (start + i) * VOICE_FRAME_LENGTH_BYTES;
		unsigned char buffer[DV_FRAME_LENGTH_BYTES];
		::memcpy(buffer + 0U, dataIn, VOICE_FRAME_LENGTH_BYTES);

		CAMBEData* dataOut = new CAMBEData;
		dataOut->setData(buffer, DV_FRAME_LENGTH_BYTES);
		data.push_back(dataOut);
	}

	return true;
}
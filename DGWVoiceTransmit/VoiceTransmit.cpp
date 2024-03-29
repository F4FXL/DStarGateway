/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
 *	 Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <cassert>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>

#include "ProgramArgs.h"
#include "DStarDefines.h"
#include "VoiceTransmit.h"
#include "SlowDataEncoder.h"
#include "APRSUtils.h"
#include "StringUtils.h"

int main(int argc, const char * argv[])
{
	std::string repeater, text, dprs;
	std::vector<std::string> filenames;

	if (!parseCLIArgs(argc, argv, repeater, filenames, text, dprs)) {
		::fprintf(stderr, "dgwvoicetransmit: invalid command line usage: dgwvoicetransmit [-text text] [-dprs dprs] <repeater> <file1> <file2> ..., exiting\n");
		return 1;
	}

	CHeaderData::initialise();

	CVoiceStore store(filenames);
	bool opened = store.open();
	if (!opened) {
		::fprintf(stderr, "dgwvoicetransmit: unable to open one of the files, exiting\n");
		return 1;
	}

	CVoiceTransmit tt(repeater, &store, text, dprs);
	bool ret = tt.run();

	store.close();


	return ret ? 0 : 1;
}

bool parseCLIArgs(int argc, const char * argv[], std::string& repeater, std::vector<std::string>& files, std::string& text, std::string& dprs)
{
	if(argc < 3)
		return false;

	std::unordered_map<std::string, std::string> namedArgs;
	std::vector<std::string> positionalArgs;

	CProgramArgs::eatArguments(argc, argv, namedArgs, positionalArgs);

	if(positionalArgs.size() < 2U)
		return false;

	repeater.assign(boost::replace_all_copy(boost::to_upper_copy(positionalArgs[0]), "_", " "));
	files.assign(positionalArgs.begin() + 1, positionalArgs.end());

	if(namedArgs.count("text") > 0U) {
		text.assign(namedArgs["text"]);
	}
	else {
		text.assign("");
	}

	if(namedArgs.count("dprs") > 0U) {
		std::string dprsRepeater(repeater);
		CAPRSUtils::dstarCallsignToAPRS(dprsRepeater);
		std::string dprsnoCrc = CStringUtils::string_format("%s>DPRS:%s\r", dprsRepeater.c_str(), namedArgs["dprs"].c_str());
		dprs = CStringUtils::string_format("$$CRC%04X,%s", CAPRSUtils::calcGPSAIcomCRC(dprsnoCrc), dprsnoCrc.c_str());
	}
	else {
		dprs.assign("");
	}

	return true;
}

CVoiceTransmit::CVoiceTransmit(const std::string& callsign, CVoiceStore* store, const std::string& text, const std::string& dprs) :
m_socket("", 0U),
m_callsign(callsign),
m_text(text),
m_dprs(dprs),
m_store(store)
{
	assert(store != NULL);
}

CVoiceTransmit::~CVoiceTransmit()
{
}

bool CVoiceTransmit::run()
{
	CSlowDataEncoder * slowData = nullptr;
	bool opened = m_socket.open();
	if (!opened)
		return false;

	in_addr address = CUDPReaderWriter::lookup("127.0.0.1");

	unsigned int id = CHeaderData::createId();

	std::string callsignG = m_callsign.substr(0U, LONG_CALLSIGN_LENGTH - 1U);
	callsignG.push_back('G');

	CHeaderData* header = m_store->getHeader();
	if (header == NULL) {
		m_socket.close();
		return false;
	}

	header->setId(id);
	header->setRptCall1(callsignG);
	header->setRptCall2(m_callsign);
	header->setDestination(address, G2_DV_PORT);

	bool overrideSlowData = !m_text.empty();

	if(overrideSlowData) {
		slowData = new CSlowDataEncoder();
		slowData->setHeaderData(*header);
		if(!m_text.empty()) slowData->setTextData(m_text);
		if(!m_dprs.empty()) slowData->setGPSData(m_dprs);
	}

	sendHeader(header);

	delete header;

	auto start = std::chrono::high_resolution_clock::now();

	unsigned int out   = 0U;
	unsigned int seqNo = 0U;
	bool loop = true;

	while (loop) {
		unsigned int needed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		needed /= DSTAR_FRAME_TIME_MS;
		unsigned char buffer[DV_FRAME_LENGTH_BYTES];

		while (out < needed) {
			CAMBEData* ambe = m_store->getAMBE();

			if (ambe == NULL) {
				CAMBEData data;
				data.setData(END_PATTERN_BYTES, DV_FRAME_LENGTH_BYTES);
				data.setDestination(address, G2_DV_PORT);
				data.setId(id);
				data.setSeq(seqNo);
				data.setEnd(true);

				sendData(&data);

				m_socket.close();

				loop = false;
				break;
			}

			ambe->getData(buffer, DV_FRAME_LENGTH_BYTES);
			// Insert sync bytes when the sequence number is zero, slow data otherwise
			if (seqNo == 0U) {
				::memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
			} else if (overrideSlowData) {
				slowData->getInterleavedData(buffer + VOICE_FRAME_LENGTH_BYTES);
			}
			ambe->setData(buffer, DV_FRAME_LENGTH_BYTES);
		
			ambe->setSeq(seqNo);
			ambe->setDestination(address, G2_DV_PORT);
			ambe->setEnd(false);
			ambe->setId(id);

			sendData(ambe);
			delete ambe;

			out++;
			seqNo++;
			if(seqNo >= 21U) seqNo = 0U;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10U));
	}

	if(slowData != nullptr) delete slowData;

	return true;
}

bool CVoiceTransmit::sendHeader(CHeaderData* header)
{
	assert(header != NULL);

	unsigned char buffer[60U];
	unsigned int length = header->getG2Data(buffer, 60U, true);

	for (unsigned int i = 0U; i < 2U; i++) {
		bool res = m_socket.write(buffer, length, header->getYourAddress(), header->getYourPort());
		if (!res)
			return false;
	}

	return true;
}

bool CVoiceTransmit::sendData(CAMBEData* data)
{
	assert(data != NULL);

	unsigned char buffer[40U];
	unsigned int length = data->getG2Data(buffer, 40U);

	return m_socket.write(buffer, length, data->getYourAddress(), data->getYourPort());
}

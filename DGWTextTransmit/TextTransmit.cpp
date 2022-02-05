/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
 *   Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <unordered_map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <thread>
#include <chrono>

#include "SlowDataEncoder.h"
#include "DStarDefines.h"
#include "TextTransmit.h"
#include "ProgramArgs.h"


int main(int argc, const char* argv[])
{
	std::string repeater, text, filename;

	if (!parseCLIArgs(argc, argv, repeater, text, filename)) {
		::fprintf(stderr, "dgwtexttransmit: invalid command line usage: dgwtexttransmit <repeater> -text <text>|-file <filename>, exiting\n");
		return 1;
	}

	CHeaderData::initialise();

	if (!filename.empty()) {
		std::ifstream file;
		file.open(filename);
		if (!file.is_open()) {
			::fprintf(stderr, "dgwtexttransmit: unable to open the file, exiting\n");
			return 1;
		}

		std::getline(file, text);

		file.close();
	}

	text.resize(20U, ' ');

	CTextTransmit tt(repeater, text);
	bool ret = tt.run();

	return ret ? 0 : 1;
}

bool parseCLIArgs(int argc, const char * argv[], std::string& repeater, std::string& text, std::string& file)
{
	repeater.clear();
	text.clear();
	file.clear();

	if(argc < 4)
		return false;

	std::unordered_map<std::string, std::string> namedArgs;
	std::vector<std::string> positionalArgs;

	CProgramArgs::eatArguments(argc, argv, namedArgs, positionalArgs);

	if(namedArgs.count("text") == 0 && namedArgs.count("file") == 0)
		return false;

	if(positionalArgs.size() != 1)
		return false;

	repeater = boost::to_upper_copy(positionalArgs[0]);
	boost::replace_all(repeater, "_", " ");
	repeater.resize(LONG_CALLSIGN_LENGTH, ' ');

	if(namedArgs.count("text") == 1) {
		text.assign(namedArgs["text"]);
	}
	else if(namedArgs.count("file") == 1){
		file.assign(namedArgs[file]);
	}

	return true;
}

CTextTransmit::CTextTransmit(const std::string& callsign, const std::string& text) :
m_socket("", 0U),
m_callsign(callsign),
m_text(text)
{
}

CTextTransmit::~CTextTransmit()
{
}

bool CTextTransmit::run()
{
	bool opened = m_socket.open();
	if (!opened)
		return false;

	in_addr address = CUDPReaderWriter::lookup("127.0.0.1");

	unsigned int id = CHeaderData::createId();

	std::string callsignG = m_callsign.substr(0U, LONG_CALLSIGN_LENGTH - 1U);
	callsignG.push_back('G');

	CHeaderData header;
	header.setId(id);
	header.setMyCall1(m_callsign);
	header.setMyCall2("INFO");
	header.setRptCall1(callsignG);
	header.setRptCall2(m_callsign);
	header.setYourCall("CQCQCQ  ");
	header.setDestination(address, G2_DV_PORT);

	sendHeader(header);

	CSlowDataEncoder encoder;
	encoder.setHeaderData(header);
	encoder.setTextData(m_text);

	CAMBEData data;
	data.setDestination(address, G2_DV_PORT);
	data.setId(id);

	auto start = std::chrono::high_resolution_clock::now();

	unsigned int out = 0U;

	for (;;) {
		unsigned int needed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
		needed /= DSTAR_FRAME_TIME_MS;

		while (out < needed) {
			data.setSeq(out);

			unsigned char buffer[DV_FRAME_LENGTH_BYTES];
			::memcpy(buffer + 0U, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

			// Insert sync bytes when the sequence number is zero, slow data otherwise
			if (out == 0U) {
				::memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
				encoder.sync();
			} else {
				encoder.getTextData(buffer + VOICE_FRAME_LENGTH_BYTES);
			}

			data.setData(buffer, DV_FRAME_LENGTH_BYTES);

			sendData(data);
			out++;

			if (out == 21U) {
				data.setData(END_PATTERN_BYTES, DV_FRAME_LENGTH_BYTES);
				data.setSeq(0U);
				data.setEnd(true);

				sendData(data);

				m_socket.close();

				return true;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10U));
	}
}

bool CTextTransmit::sendHeader(const CHeaderData& header)
{
	unsigned char buffer[60U];
	unsigned int length = header.getG2Data(buffer, 60U, true);

	for (unsigned int i = 0U; i < 2U; i++) {
		bool res = m_socket.write(buffer, length, header.getYourAddress(), header.getYourPort());
		if (!res)
			return false;
	}

	return true;
}

bool CTextTransmit::sendData(const CAMBEData& data)
{
	unsigned char buffer[40U];
	unsigned int length = data.getG2Data(buffer, 40U);

	return m_socket.write(buffer, length, data.getYourAddress(), data.getYourPort());
}

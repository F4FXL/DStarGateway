/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
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

#include <cassert>
#include <cstring>

#include "DStarDefines.h"
#include "DDData.h"
#include "Utils.h"

unsigned int ETHERNET_ADDRESS_LENGTH = 6U;

unsigned int BUFFER_LENGTH = 2500U;

CDDData::CDDData() :
m_header(),
m_length(0U),
m_frame(NULL)
{
	m_frame = new unsigned char[BUFFER_LENGTH];
}

CDDData::CDDData(const CDDData& data) :
m_header(data.m_header),
m_length(data.m_length),
m_frame(NULL)
{
	m_frame = new unsigned char[BUFFER_LENGTH];
	::memcpy(m_frame, data.m_frame, data.m_length);
}

CDDData::~CDDData()
{
	delete[] m_frame;
}

bool CDDData::setIcomRepeaterData(const unsigned char *data, unsigned int length, const in_addr& yourAddress, unsigned int yourPort)
{
	assert(data != NULL);
	assert(length >= 29U);

	bool ret = m_header.setIcomRepeaterData(data, length, true, yourAddress, yourPort);
	if (!ret)
		return false;

	m_length = data[59] * 256U + data[58];

	if (m_length > BUFFER_LENGTH)
		m_length = BUFFER_LENGTH;

	::memcpy(m_frame, data + 60U, m_length);

	return true;
}

bool CDDData::setHBRepeaterData(const unsigned char *data, unsigned int length, const in_addr& /*yourAddress*/, unsigned int /*yourPort*/)
{
	assert(data != NULL);
	assert(length >= 60U);

	m_header.setData(data, length, false);

	m_length = length - 44U;

	if (m_length > BUFFER_LENGTH)
		m_length = BUFFER_LENGTH;

	::memcpy(m_frame, data + 44U, m_length);

	return true;
}

unsigned int CDDData::getIcomRepeaterData(unsigned char *data, unsigned int length)
{
	assert(data != NULL);
	assert(length >= 32U);

	// This section is used when it's normal data (i.e. not an ack) for an Icom controller
	data[0] = 'D';
	data[1] = 'S';
	data[2] = 'T';
	data[3] = 'R';

	data[4] = m_header.getRptSeq() / 256U;		// Packet sequence number
	data[5] = m_header.getRptSeq() % 256U;

	data[6] = 0x73;					// Not a response
	data[7] = 0x11;					// DD Data type

	unsigned int dataLength = m_length + 50U;

	data[8] = dataLength / 256U;	// Length
	data[9] = dataLength % 256U;

	data[10] = 0x40;				// DD Data

	data[11] = 0xFF;
	data[12] = 0xFF;
	data[13] = 0xFF;

	data[14] = 0x00;				// Dummy ID
	data[15] = 0x00;

	data[16] = 0xC0;				// DD Data

	m_header.getData(data + 17U, RADIO_HEADER_LENGTH_BYTES, true);

	// Another length field
	data[58] = m_length % 256U;
	data[59] = m_length / 256U;

	// Now copy the payload
	::memcpy(data + 60U, m_frame, m_length);

	return 60U + m_length;
}

unsigned int CDDData::getHBRepeaterData(unsigned char *data, unsigned int length)
{
	assert(data != NULL);
	assert(length >= 1600U);

	data[0] = 'D';
	data[1] = 'S';
	data[2] = 'R';
	data[3] = 'P';

	data[4] = 0x24U;

	m_header.getData(data + 5U, RADIO_HEADER_LENGTH_BYTES, false);

	// Now copy the payload
	::memcpy(data + 44U, m_frame, m_length);

	return 44U + m_length;
}

unsigned int CDDData::getRptSeq() const
{
	return m_header.getRptSeq();
}

void CDDData::setRptSeq(unsigned int seqNo)
{
	m_header.setRptSeq(seqNo);
}

unsigned char CDDData::getBand1() const
{
	return m_header.getBand1();
}

unsigned char CDDData::getBand2() const
{
	return m_header.getBand2();
}

unsigned char CDDData::getBand3() const
{
	return m_header.getBand3();
}

void CDDData::setBand1(unsigned char band)
{
	m_header.setBand1(band);
}

void CDDData::setBand2(unsigned char band)
{
	m_header.setBand2(band);
}

void CDDData::setBand3(unsigned char band)
{
	m_header.setBand3(band);
}

unsigned char CDDData::getFlag1() const
{
	return m_header.getFlag1();
}

unsigned char CDDData::getFlag2() const
{
	return m_header.getFlag2();
}

unsigned char CDDData::getFlag3() const
{
	return m_header.getFlag3();
}

void CDDData::setFlags(unsigned char flag1, unsigned char flag2, unsigned char flag3)
{
	m_header.setFlags(flag1, flag2, flag3);
}

std::string CDDData::getMyCall1() const
{
	return m_header.getMyCall1();
}

std::string CDDData::getMyCall2() const
{
	return m_header.getMyCall2();
}

std::string CDDData::getYourCall() const
{
	return m_header.getYourCall();
}

std::string CDDData::getRptCall1() const
{
	return m_header.getRptCall1();
}

std::string CDDData::getRptCall2() const
{
	return m_header.getRptCall2();
}

void CDDData::setMyCall1(const std::string& callsign)
{
	m_header.setMyCall1(callsign);
}

void CDDData::setMyCall2(const std::string& callsign)
{
	m_header.setMyCall2(callsign);
}

void CDDData::setYourCall(const std::string& callsign)
{
	m_header.setYourCall(callsign);
}

void CDDData::setRptCall1(const std::string& callsign)
{
	m_header.setRptCall1(callsign);
}

void CDDData::setRptCall2(const std::string& callsign)
{
	m_header.setRptCall2(callsign);
}

void CDDData::setRepeaters(const std::string& rpt1, const std::string& rpt2)
{
	m_header.setRepeaters(rpt1, rpt2);
}

void CDDData::setDestination(const in_addr& address, unsigned int port)
{
	m_header.setDestination(address, port);
}

in_addr CDDData::getYourAddress() const
{
	return m_header.getYourAddress();
}

unsigned int CDDData::getYourPort() const
{
	return m_header.getYourPort();
}

void CDDData::setEthernetFrame(const unsigned char *frame, unsigned int length)
{
	assert(frame != NULL);
	assert(length > 0U);

	m_length = length;
	if (m_length > BUFFER_LENGTH)
		m_length = BUFFER_LENGTH;

	::memcpy(m_frame, frame, m_length);
}

unsigned int CDDData::getEthernetFrame(unsigned char *frame, unsigned int length) const
{
	assert(frame != NULL);
	assert(length > 0U);

	if (length > m_length)
		length = m_length;

	::memcpy(frame, m_frame, length);

	return length;
}

unsigned char* CDDData::getSourceAddress() const
{
	return m_frame + ETHERNET_ADDRESS_LENGTH;
}

unsigned char* CDDData::getDestinationAddress() const
{
	return m_frame + 0U;
}

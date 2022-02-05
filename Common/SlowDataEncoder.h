/*
 *	Copyright (C) 2010 by Jonathan Naylor, G4KLX
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

#include "HeaderData.h"

#ifdef __UNIT_TEST__
	class SlowDataEncoderTests;
#endif

class CSlowDataEncoder {
public:
	CSlowDataEncoder();
	~CSlowDataEncoder();


	void setHeaderData(const CHeaderData& header);
	void setTextData(const std::string& text);
	void setGPSData(const std::string& gpsData);

	void clearHeaderData();
	void clearTextData();
	void clearGPSData();
	void clearInterleavedData();

	void getHeaderData(unsigned char* data);
	void getTextData(unsigned char* data);
	void getGPSData(unsigned char* data);

	void getInterleavedData(unsigned char* data);

	unsigned int getInterleavedDataLength();

	void reset();
	void sync();

private:
	void getData(unsigned char* source, unsigned char* data, unsigned int &sourcePtr, unsigned int sourceLength);
	void buildInterleavedData();

	unsigned char* m_headerData;
	unsigned char* m_textData;
	unsigned char* m_gpsData;
	unsigned char* m_interleavedData;

	unsigned int   m_headerPtr;
	unsigned int   m_textPtr;
	unsigned int   m_gpsPtr;
	unsigned int   m_interleavedPtr;

	unsigned int   m_gpsDataSize; //actual useful data size
	unsigned int   m_gpsDataFullSize; //size including filler bytes

	unsigned int   m_interleavedDataFullSize; //size of interleaved data including filler bytes

#ifdef __UNIT_TEST__
	friend SlowDataEncoderTests;
#endif

};

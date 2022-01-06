/*
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <boost/algorithm/string.hpp>

#include "APRSUnit.h"
#include "APRSFormater.h"
#include "StringUtils.h"
#include "APRSUtils.h"
#include "SlowDataEncoder.h"

CAPRSUnit::CAPRSUnit(IRepeaterCallback * repeaterHandler) :
m_frameBuffer(20U),
m_status(APS_IDLE),
m_repeaterHandler(repeaterHandler),
m_headerData(nullptr),
m_timer(1000U, 2U)
{
    m_timer.start();
}

void CAPRSUnit::writeFrame(CAPRSFrame& frame)
{
    auto frameCopy = new CAPRSFrame(frame);
    frameCopy->getPath().clear();//path is of no use for us, just clear it

    m_frameBuffer.push_back(frameCopy);
}

void CAPRSUnit::clock(unsigned int ms)
{
    m_timer.clock(ms);
    if(m_status == APS_IDLE && !m_frameBuffer.empty() && m_timer.hasExpired()) {
        auto frame = m_frameBuffer.front();

        m_id = CHeaderData::createId();

		m_headerData = new CHeaderData();
		m_headerData->setMyCall1(frame->getSource());
		m_headerData->setMyCall2("APRS");
		m_headerData->setYourCall("CQCQCQ  ");
		m_headerData->setId(m_id);

        m_repeaterHandler->process(*m_headerData, DIR_INCOMING, AS_INFO);

        m_status = APS_TRANSMIT;
    }

    if(m_status == APS_TRANSMIT && !m_frameBuffer.empty())
    {
        auto frame = m_frameBuffer.front();
        std::string frameString;
        CAPRSFormater::frameToString(frameString, *frame);
        boost::trim_right_if(frameString, [](char c) { return c == '\n' || c == '\r'; });
        frameString.push_back('\r');

        std::string crc = CStringUtils::string_format("$$CRC%04X", CAPRSUtils::calcGPSAIcomCRC(frameString));
        frameString.insert(0, crc);

        CSlowDataEncoder encoder;
        encoder.setHeaderData(*m_headerData);
        encoder.setGPSData(frameString);
        encoder.setTextData("APRS to DPRS");

        CAMBEData data;
        data.setId(m_id);

        unsigned int out = 0U;
        unsigned int dataOut = 0U;
        unsigned int needed = (encoder.getInterleavedDataLength() / (DATA_FRAME_LENGTH_BYTES)) * 2U;
        unsigned char buffer[DV_FRAME_LENGTH_BYTES];

        while (dataOut < needed) {
            data.setSeq(out);

            ::memcpy(buffer + 0U, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

            // Insert sync bytes when the sequence number is zero, slow data otherwise
            if (out == 0U) {
                ::memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
            } else {
                encoder.getInterleavedData(buffer + VOICE_FRAME_LENGTH_BYTES);		
                dataOut++;
            }

            data.setData(buffer, DV_FRAME_LENGTH_BYTES);

            m_repeaterHandler->process(data, DIR_INCOMING, AS_INFO);
            out++;

            if (out == 21U) out = 0U;
        }

        m_frameBuffer.pop_front();
        delete frame;
        m_status = APS_IDLE;
        m_timer.start();
    }
}
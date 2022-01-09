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
#include "APRStoDPRS.h"

CAPRSUnit::CAPRSUnit(IRepeaterCallback * repeaterHandler) :
m_frameBuffer(20U),
m_status(APS_IDLE),
m_repeaterHandler(repeaterHandler),
m_headerData(nullptr),
m_slowData(nullptr),
m_out(0U),
m_seq(0U),
m_totalNeeded(0U),
m_timer(1000U, 2U),
m_dprs(),
m_start()
{
    m_timer.start();
}

void CAPRSUnit::writeFrame(CAPRSFrame& frame)
{
    auto frameCopy = new CAPRSFrame(frame);
    frameCopy->getPath().clear();//path is of no use for us, just clear it

    m_frameBuffer.push_back(frameCopy);
    m_timer.start();
}

void CAPRSUnit::clock(unsigned int ms)
{
    m_timer.clock(ms);
    if(m_status == APS_IDLE && !m_frameBuffer.empty() && m_timer.hasExpired()) {
        m_status = APS_TRANSMIT;
        auto frame = m_frameBuffer.front();
        m_frameBuffer.pop_front();

        m_headerData = new CHeaderData();
        CAPRSToDPRS::aprsToDPRS(m_dprs, *m_headerData, *frame);

        m_slowData = new CSlowDataEncoder();
        // icom rs-ms1 seem to not support messaiging mixed with other slow data 
        // send the message on its own for now
        // m_slowData->setHeaderData(*m_headerData);
        m_slowData->setGPSData(m_dprs);
        // m_slowData->setTextData("APRS to DPRS");

        m_totalNeeded = (m_slowData->getInterleavedDataLength() / (DATA_FRAME_LENGTH_BYTES)) * 2U;

        m_repeaterHandler->process(*m_headerData, DIR_INCOMING, AS_INFO);

        m_out = 0U;
        m_seq = 0U;

        m_start = std::chrono::high_resolution_clock::now();
        return;
    }

    if(m_status == APS_TRANSMIT) {
        unsigned int needed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start).count();
		needed /= DSTAR_FRAME_TIME_MS;

        unsigned char buffer[DV_FRAME_LENGTH_BYTES];

        while (m_out < needed && m_out < m_totalNeeded) {
            CAMBEData data;
            data.setId(m_headerData->getId());
            data.setSeq(m_seq);
            if(m_out == m_totalNeeded - 1U)
                data.setEnd(true);

            ::memcpy(buffer + 0U, NULL_AMBE_DATA_BYTES, VOICE_FRAME_LENGTH_BYTES);

            // Insert sync bytes when the sequence number is zero, slow data otherwise
            if (m_seq == 0U) {
                ::memcpy(buffer + VOICE_FRAME_LENGTH_BYTES, DATA_SYNC_BYTES, DATA_FRAME_LENGTH_BYTES);
            } else {
                m_slowData->getInterleavedData(buffer + VOICE_FRAME_LENGTH_BYTES);		
                m_out++;
            }

            data.setData(buffer, DV_FRAME_LENGTH_BYTES);
            m_repeaterHandler->process(data, DIR_INCOMING, AS_INFO);

            m_seq++;
            if (m_seq == 21U) m_seq = 0U;
        }

        if(m_out >= m_totalNeeded) {
            m_status = APS_IDLE;
            delete m_headerData;
            delete m_slowData;
        }
    }
}
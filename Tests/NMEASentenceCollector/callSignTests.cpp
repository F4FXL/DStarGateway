/*
 *   Copyright (C) 2021-2023 by Geoffrey Merck F4FXL / KC3FRA
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

#include <gtest/gtest.h>

#include "NMEASentenceCollector.h"

namespace NMEASentenceCollectorTests
{
    class NMEASentenceCollectorTests_APRSFrame: public ::testing::Test
    {
        protected:
            CNMEASentenceCollector * m_collector;

        void SetUp() override
        {
            m_collector = new CNMEASentenceCollector("$GPRMC");

            std::string nmea("$GPRMC,092751.000,A,5321.6802,N,00630.3371,W,0.06,31.66,280511,,,A*45\x0A");

            unsigned char data[3];
            unsigned int len = nmea.length() + (nmea.length() % 3);
            bool first = true;

            for (unsigned int i = 0; i < len;) {
                if(first) {
                    data[0] = SLOW_DATA_TYPE_GPS ^ SCRAMBLER_BYTE1;
                    first = false;
                }
                else {
                    data[0] = (i < nmea.length() ? nmea[i] : 'f') ^ SCRAMBLER_BYTE1;
                    i++;
                    first = true;
                }
                data[1] = (i < nmea.length() ? nmea[i] : 'f') ^ SCRAMBLER_BYTE2;
                i++;
                data[2] = (i < nmea.length() ? nmea[i] : 'f') ^ SCRAMBLER_BYTE3;
                i++;

                m_collector->writeData(data);
            }
            
        }
        
        void TearDown() override
        {
            delete m_collector;
        }
    };

    TEST_F(NMEASentenceCollectorTests_APRSFrame, noSSIDinCallsign)
    {
        std::string data;

        m_collector->setMyCall1("N0CALL");
        m_collector->setMyCall2("5100");
        m_collector->getData(data);

        EXPECT_TRUE(data.find("N0CALL-5") != std::string::npos) << "Callsign in APRS frame shall be N0CALL-5";
    }

    TEST_F(NMEASentenceCollectorTests_APRSFrame, SSIDinCallsign)
    {
        std::string data;

        m_collector->setMyCall1("N0CALL H");
        m_collector->setMyCall2("5100");
        m_collector->getData(data);

        EXPECT_TRUE(data.find("N0CALL") != std::string::npos) << "Callsign in APRS frame shall be N0CALL-H";
    }
}
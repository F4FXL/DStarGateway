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

#include <string>
#include <cmath>
#include <boost/algorithm/string.hpp>

#include "APRSFixedIdFrameProvider.h"
#include "StringUtils.h"

CAPRSFixedIdFrameProvider::CAPRSFixedIdFrameProvider(const std::string& gateway) :
CAPRSIdFrameProvider(gateway, 20U) // Initial timeout of 20 seconds
{

}

bool CAPRSFixedIdFrameProvider::buildAPRSFramesInt(const CAPRSEntry * entry, std::vector<CAPRSFrame *>& frames)
{
    if (entry == nullptr)
        return false;

    // Default values aren't passed on
    if (entry->getLatitude() == 0.0 && entry->getLongitude() == 0.0)
        return false;

    frames.clear();

    time_t now;
	::time(&now);
	struct tm* tm = ::gmtime(&now);

    std::string desc;
    if (entry->getBand().length() > 1U) {
        if (entry->getFrequency() != 0.0)
            desc = CStringUtils::string_format("Data %.5lfMHz", entry->getFrequency());
        else
            desc = "Data";
    } else {
        if (entry->getFrequency() != 0.0)
            desc = CStringUtils::string_format("Voice %.5lfMHz %c%.4lfMHz",
                    entry->getFrequency(),
                    entry->getOffset() < 0.0 ? '-' : '+',
                    std::fabs(entry->getOffset()));
        else
            desc = "Voice";
    }

    std::string band;
    if (entry->getFrequency() >= 1200.0)
        band = "1.2";
    else if (entry->getFrequency() >= 420.0)
        band = "440";
    else if (entry->getFrequency() >= 144.0)
        band = "2m";
    else if (entry->getFrequency() >= 50.0)
        band = "6m";
    else if (entry->getFrequency() >= 28.0)
        band = "10m";

    double tempLat  = ::fabs(entry->getLatitude());
    double tempLong = ::fabs(entry->getLongitude());

    double latitude  = ::floor(tempLat);
    double longitude = ::floor(tempLong);

    latitude  = (tempLat  - latitude)  * 60.0 + latitude  * 100.0;
    longitude = (tempLong - longitude) * 60.0 + longitude * 100.0;

    std::string lat;
    if (latitude >= 1000.0F)
        lat = CStringUtils::string_format("%.2lf", latitude);
    else if (latitude >= 100.0F)
        lat = CStringUtils::string_format("0%.2lf", latitude);
    else if (latitude >= 10.0F)
        lat = CStringUtils::string_format("00%.2lf", latitude);
    else
        lat = CStringUtils::string_format("000%.2lf", latitude);

    std::string lon;
    if (longitude >= 10000.0F)
        lon = CStringUtils::string_format("%.2lf", longitude);
    else if (longitude >= 1000.0F)
        lon = CStringUtils::string_format("0%.2lf", longitude);
    else if (longitude >= 100.0F)
        lon = CStringUtils::string_format("00%.2lf", longitude);
    else if (longitude >= 10.0F)
        lon = CStringUtils::string_format("000%.2lf", longitude);
    else
        lon = CStringUtils::string_format("0000%.2lf", longitude);

    // Convert commas to periods in the latitude and longitude
    boost::replace_all(lat, ",", ".");
    boost::replace_all(lon, ",", ".");

    std::string body = CStringUtils::string_format(";%-7s%-2s*%02d%02d%02dz%s%cD%s%caRNG%04.0lf/A=%06.0lf %s %s\r\n",
                                                        entry->getCallsign().c_str(), entry->getBand().c_str(),
                                                        tm->tm_mday, tm->tm_hour, tm->tm_min,
                                                        lat.c_str(), (entry->getLatitude() < 0.0F)  ? 'S' : 'N',
                                                        lon.c_str(), (entry->getLongitude() < 0.0F) ? 'W' : 'E',
                                                        entry->getRange() * 0.6214, entry->getAGL() * 3.28, band.c_str(), desc.c_str());

    CAPRSFrame * frame = new CAPRSFrame(m_gateway + "-S",
                                        "APD5T1",
                                        { "TCPIP*", "qAC" , m_gateway + "-GS" },
                                        body, APFT_OBJECT);

    frames.push_back(frame);

    if (entry->getBand().length() == 1U) {
        body = CStringUtils::string_format("!%s%cD%s%c&RNG%04.0lf/A=%06.0lf %s %s\r\n",
            lat.c_str(), (entry->getLatitude() < 0.0F)  ? 'S' : 'N',
            lon.c_str(), (entry->getLongitude() < 0.0F) ? 'W' : 'E',
            entry->getRange() * 0.6214, entry->getAGL() * 3.28, band.c_str(), desc.c_str());

        frame = new CAPRSFrame(entry->getCallsign() + "-" + entry->getBand(),
                                "APD5T2",
                                { "TCPIP*", "qAC", entry->getCallsign() + "-" + entry->getBand() + "S"},
                                body, APFT_POSITION);


        frames.push_back(frame);
    }

    setTimeout(20U * 60U);//20 minutes, plenty enough for fixed

    return true;
}
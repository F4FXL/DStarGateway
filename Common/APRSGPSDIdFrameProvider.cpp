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

#ifdef USE_GPSD
#include <cmath>
#include <boost/algorithm/string.hpp>

#include "APRSGPSDIdFrameProvider.h"
#include "StringUtils.h"
#include "Log.h"

CAPRSGPSDIdFrameProvider::CAPRSGPSDIdFrameProvider(const std::string& gateway, const std::string& address, const std::string& port) :
CAPRSIdFrameProvider(gateway, 20U),
m_gpsdAddress(address),
m_gpsdPort(port),
m_gpsdData(),
m_hasConnection(false)
{

}

void CAPRSGPSDIdFrameProvider::start()
{
    int ret = ::gps_open(m_gpsdAddress.c_str(), m_gpsdPort.c_str(), &m_gpsdData);
    if (ret != 0) {
        CLog::logError("Error when opening access to gpsd - %d - %s", errno, ::gps_errstr(errno));
        m_hasConnection = false;
    }
    else {
        ::gps_stream(&m_gpsdData, WATCH_ENABLE | WATCH_JSON, NULL);
        CLog::logError("Connected to GPSD");
        m_hasConnection = true;
    }
}

void CAPRSGPSDIdFrameProvider::close()
{
    if(m_hasConnection) {
        ::gps_stream(&m_gpsdData, WATCH_DISABLE, NULL);
        ::gps_close(&m_gpsdData);
    }
}

bool CAPRSGPSDIdFrameProvider::buildAPRSFramesInt(const CAPRSEntry * entry, std::vector<CAPRSFrame *>& frames)
{
    if(!m_hasConnection) {
        this->start();
    }

	if(!m_hasConnection || !::gps_waiting(&m_gpsdData, 0))
	    return false;

#if GPSD_API_MAJOR_VERSION >= 7
    char message[1024];
	if (::gps_read(&m_gpsdData, message, 1024) <= 0)
		return false;
#else
	if (::gps_read(&m_gpsdData) <= 0)
		return false;
#endif

#if GPSD_API_MAJOR_VERSION < 10
	if (m_gpsdData.status != STATUS_FIX)
		return false;
#elif GPSD_API_MAJOR_VERSION == 11
	if(m_gpsdData.fix.status == STATUS_NO_FIX)
		return false;
#elif GPSD_API_MAJOR_VERSION <= 16
	if(m_gpsdData.fix.status == STATUS_UNK)
        return false;
#elif 
    #error Unsupported GPSD_API_MAJOR_VERSION
#endif

	bool latlonSet   = (m_gpsdData.set & LATLON_SET) == LATLON_SET;
	bool altitudeSet = (m_gpsdData.set & ALTITUDE_SET) == ALTITUDE_SET;
	bool velocitySet = (m_gpsdData.set & SPEED_SET) == SPEED_SET;
	bool bearingSet  = (m_gpsdData.set & TRACK_SET) == TRACK_SET;

	if (!latlonSet)
		return false;

	float rawLatitude  = float(m_gpsdData.fix.latitude);
	float rawLongitude = float(m_gpsdData.fix.longitude);
#if GPSD_API_MAJOR_VERSION >= 9
	float rawAltitude  = float(m_gpsdData.fix.altMSL);
#else
	float rawAltitude  = float(m_gpsdData.fix.altitude);
#endif
	float rawVelocity  = float(m_gpsdData.fix.speed);
	float rawBearing   = float(m_gpsdData.fix.track);

	time_t now;
	::time(&now);
	struct tm* tm = ::gmtime(&now);
    if (entry == NULL)
        return false;

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
                    ::fabs(entry->getOffset()));
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

    double tempLat  = ::fabs(rawLatitude);
    double tempLong = ::fabs(rawLongitude);

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

    std::string body = CStringUtils::string_format(";%-7s%-2s*%02d%02d%02dz%s%cD%s%ca/A=%06.0lf",
                                                        entry->getCallsign().c_str(), entry->getBand().c_str(),
                                                        tm->tm_mday, tm->tm_hour, tm->tm_min,
                                                        lat.c_str(), (rawLatitude < 0.0)  ? 'S' : 'N',
                                                        lon.c_str(), (rawLongitude < 0.0) ? 'W' : 'E',
                                                        rawAltitude * 3.28);

    if (bearingSet && velocitySet)
        body.append(CStringUtils::string_format("%03.0lf/%03.0lf", rawBearing, rawVelocity * 0.539957F));

    body.append(CStringUtils::string_format("RNG%04.0lf %s %s\r\n", entry->getRange() * 0.6214, band.c_str(), desc.c_str()));


    CAPRSFrame * frame = new CAPRSFrame(m_gateway + "-S",
                                    "APD5T1",
                                    { "TCPIP*", "qAC" , m_gateway + "-GS" },
                                    body, APFT_OBJECT);


    frames.push_back(frame);

    if (entry->getBand().length() == 1U) {
        if (altitudeSet)
            body = CStringUtils::string_format("%s%cD%s%c&/A=%06.0lf",
                lat.c_str(), (rawLatitude < 0.0)  ? 'S' : 'N',
                lon.c_str(), (rawLongitude < 0.0) ? 'W' : 'E',
                rawAltitude * 3.28);
        else
            body = CStringUtils::string_format("!%s%cD%s%c&",
                lat.c_str(), (rawLatitude < 0.0)  ? 'S' : 'N',
                lon.c_str(), (rawLongitude < 0.0) ? 'W' : 'E');

        frame = new CAPRSFrame(m_gateway,
                                    "APD5T2",
                                    { "TCPIP*", "qAC" , m_gateway + "-GS" },
                                    body, APFT_POSITION);

        frames.push_back(frame);
    }

    setTimeout(60U * 5U);//5 Minutes is plenty enough we aint an APRS tracker !

    return true;
}
#endif
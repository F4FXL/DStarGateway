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

#pragma once

#include "Defs.h"

#ifdef USE_GPSD
#include <gps.h>
#include <string>

#include "APRSIdFrameProvider.h"

class CAPRSGPSDIdFrameProvider : public CAPRSIdFrameProvider
{
public:
    CAPRSGPSDIdFrameProvider(std::string address, std::string port);

    virtual void start();
    virtual void close();

protected:
    virtual bool buildAPRSFramesInt(const std::string& gateway, const CAPRSEntry * aprsEntry, std::vector<std::string>& frames);

private:
	std::string			m_gpsdAddress;
	std::string			m_gpsdPort;
	struct gps_data_t	m_gpsdData;
    bool                m_hasConnection;
};
#endif

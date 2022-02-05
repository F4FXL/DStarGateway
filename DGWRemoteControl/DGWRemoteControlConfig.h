/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <string>
#include <vector>

#include "Config.h"

typedef struct {
    std::string m_name;
	std::string m_address;
    unsigned int m_port; 
    std::string m_password;
} TRemoteGateway;

class CDGWRemoteControlConfig
{
public:
    CDGWRemoteControlConfig(const std::string& fileName);
    ~CDGWRemoteControlConfig();
    bool load();
    bool getGateway(const std::string& name, TRemoteGateway& gateway);

private:
    bool open(CConfig& config);
    bool loadGateways(CConfig& config);

    std::string m_fileName;
    std::vector<TRemoteGateway *> m_gateways;
};
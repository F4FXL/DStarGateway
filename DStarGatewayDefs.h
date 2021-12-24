/*
 *   Copyright (C) 2010-2015 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

const std::string APPLICATION_NAME("ircDDB Gateway");

const std::string CONFIG_FILE_NAME("ircddbgateway");

const std::string STATUS1_FILE_NAME("status1.txt");
const std::string STATUS2_FILE_NAME("status2.txt");
const std::string STATUS3_FILE_NAME("status3.txt");
const std::string STATUS4_FILE_NAME("status4.txt");
const std::string STATUS5_FILE_NAME("status5.txt");

const std::string PRIMARY_WHITELIST_FILE_NAME  ("gw_whitelist.dat");
const std::string SECONDARY_WHITELIST_FILE_NAME("whitelist.dat");
const std::string PRIMARY_BLACKLIST_FILE_NAME  ("gw_blacklist.dat");
const std::string SECONDARY_BLACKLIST_FILE_NAME("blacklist.dat");
const std::string RESTRICT_FILE_NAME           ("restrict.txt");

const unsigned int MAX_OUTGOING       = 6U;
const unsigned int MAX_REPEATERS      = 4U;
const unsigned int MAX_DEXTRA_LINKS   = 5U;
const unsigned int MAX_DPLUS_LINKS    = 5U;
const unsigned int MAX_DCS_LINKS      = 5U;
const unsigned int MAX_STARNETS       = 5U;
const unsigned int MAX_ROUTES         = MAX_REPEATERS + 5U;
const unsigned int MAX_DD_ROUTES      = 20U;

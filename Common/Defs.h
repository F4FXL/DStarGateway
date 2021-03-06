/*
 *   Copyright (C) 2010-2015 by Jonathan Naylor G4KLX
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

#pragma once

#include <string>

const std::string DEXTRA_HOSTS_FILE_NAME("DExtra_Hosts.txt");
const std::string DCS_HOSTS_FILE_NAME("DCS_Hosts.txt");
const std::string DPLUS_HOSTS_FILE_NAME("DPlus_Hosts.txt");
const std::string GATEWAY_HOSTS_FILE_NAME("Gateway_Hosts.txt");

const std::string LINKS_BASE_NAME("Links");
const std::string TEXT_BASE_NAME("Text");
const std::string USERS_BASE_NAME("Users");
const std::string STARNET_BASE_NAME("STARnet");
const std::string HEADERS_BASE_NAME("Headers");
const std::string DDMODE_BASE_NAME("DDMode");

enum RECONNECT {
	RECONNECT_NEVER,
	RECONNECT_FIXED,
	RECONNECT_5MINS,
	RECONNECT_10MINS,
	RECONNECT_15MINS,
	RECONNECT_20MINS,
	RECONNECT_25MINS,
	RECONNECT_30MINS,
	RECONNECT_60MINS,
	RECONNECT_90MINS,
	RECONNECT_120MINS,
	RECONNECT_180MINS
};

enum DIRECTION {
	DIR_INCOMING,
	DIR_OUTGOING
};

enum PROTOCOL {
	PROTO_DEXTRA,
	PROTO_DPLUS,
	PROTO_DCS,
	PROTO_CCS
};

enum HW_TYPE {
	HW_HOMEBREW,
	HW_ICOM,
	HW_DUMMY
};

enum TEXT_LANG {
	TL_ENGLISH_UK,
	TL_DEUTSCH,
	TL_DANSK,
	TL_FRANCAIS,
	TL_ITALIANO,
	TL_POLSKI,
	TL_ENGLISH_US,
	TL_ESPANOL,
	TL_SVENSKA,
	TL_NEDERLANDS_NL,
	TL_NEDERLANDS_BE,
	TL_NORSK,
	TL_PORTUGUES
};

enum IRCDDB_STATUS {
	IS_DISABLED,
	IS_DISCONNECTED,
	IS_CONNECTING,
	IS_CONNECTED
};

enum G2_STATUS {
	G2_NONE,
	G2_LOCAL,
	G2_USER,
	G2_REPEATER,
	G2_OK,
	G2_XBAND,
	G2_ECHO,
	G2_VERSION,
#ifdef USE_STARNET
	G2_STARNET
#endif
};

enum LINK_STATUS {
	LS_NONE,
	LS_PENDING_IRCDDB,
	LS_LINKING_LOOPBACK,
	LS_LINKING_DEXTRA,
	LS_LINKING_DPLUS,
	LS_LINKING_DCS,
	LS_LINKING_CCS,
	LS_LINKED_LOOPBACK,
	LS_LINKED_DEXTRA,
	LS_LINKED_DPLUS,
	LS_LINKED_DCS,
	LS_LINKED_CCS,
	LS_INIT
};

enum SLOWDATA_STATE {
	SS_FIRST,
	SS_SECOND
};

enum CALLSIGN_SWITCH {
	SCS_GROUP_CALLSIGN,
	SCS_USER_CALLSIGN
};

enum GATEWAY_TYPE {
	GT_REPEATER,
	GT_HOTSPOT,
	GT_DONGLE,
	GT_SMARTGROUP
};

enum POSITION_SOURCE {
	POSSRC_FIXED,
	POSSRC_GPSD
};

const unsigned int TIME_PER_TIC_MS = 5U;


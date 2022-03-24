/*
 *   Copyright (C) 2012,2013,2014 by Jonathan Naylor G4KLX
 *   Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

const std::string APPLICATION_NAME("DStarGateway Time Server");

enum LANGUAGE {
	LANG_ENGLISH_UK_1,
	LANG_ENGLISH_UK_2,
	LANG_ENGLISH_US_1,
	LANG_ENGLISH_US_2,
	LANG_DEUTSCH_1,
	LANG_DEUTSCH_2,
	LANG_FRANCAIS,
	LANG_NEDERLANDS,
	LANG_SVENSKA,
	LANG_ESPANOL,
	LANG_NORSK,
	LANG_PORTUGUES
};

enum INTERVAL {
	INTERVAL_15MINS,
	INTERVAL_30MINS,
	INTERVAL_60MINS
};

enum FORMAT {
	FORMAT_VOICE_TIME,
	FORMAT_TEXT_TIME
};

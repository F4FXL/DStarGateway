/*
 *   Copyright (C) 2011,2012,2013 by Jonathan Naylor G4KLX
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
#include "DStarDefines.h"
#include "HeaderData.h"
#include "AMBEData.h"
#include "Defs.h"

class IRepeaterCallback {
public:
	virtual bool process(CHeaderData& header, DIRECTION direction, AUDIO_SOURCE source) = 0;

	virtual bool process(CAMBEData& data, DIRECTION direction, AUDIO_SOURCE source) = 0;

private:
};

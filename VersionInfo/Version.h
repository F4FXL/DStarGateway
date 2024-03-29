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

#include "GitVersion.h"

const std::string PRODUCT_NAME("DStarGateway");
const std::string VENDOR_NAME("Geoffrey Merck F4FXL / KC3FRA and Contributors");
const std::string VERSION("1.0");
const std::string LONG_VERSION = VERSION + "-" + gitversion;
const std::string FULL_PRODUCT_NAME = PRODUCT_NAME + " v" + VERSION + "-" + gitversion;
const std::string SHORT_PRODUCT_NAME = "DStarGW v" + VERSION + "-" + gitversion;

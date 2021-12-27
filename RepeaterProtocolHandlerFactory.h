/*
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

#include "IcomRepeaterProtocolHandler.h"
#include "HBRepeaterProtocolHandler.h"
#include "DummyRepeaterProtocolHandler.h"
#include "RepeaterProtocolHandler.h"
#include "DStarGatewayConfig.h"

class CRepeaterHandlerFactory
{
public:
    CRepeaterHandlerFactory();

    IRepeaterProtocolHandler * getRepeaterProtocolHandler(HW_TYPE hwType, const TGateway & gatewyaConfig, const std::string & repeaterAddress, unsigned int repeaterPort);

private:
	CIcomRepeaterProtocolHandler* m_icomRepeaterHandler;
	CHBRepeaterProtocolHandler* m_hbRepeaterHandler;
	CDummyRepeaterProtocolHandler* m_dummyRepeaterHandler;
};
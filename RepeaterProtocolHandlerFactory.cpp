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

#include "RepeaterProtocolHandlerFactory.h"
#include "Log.h"

CRepeaterHandlerFactory::CRepeaterHandlerFactory() :
m_icomRepeaterHandler(NULL),
m_hbRepeaterHandler(NULL),
m_dummyRepeaterHandler(NULL)
{

}

IRepeaterProtocolHandler * CRepeaterHandlerFactory::getRepeaterProtocolHandler(HW_TYPE hwType, const TGateway & gatewayConfig, const std::string & repeaterAddress, unsigned int repeaterPort)
{
    IRepeaterProtocolHandler * handler = NULL;
    switch (hwType)
    {
    case HW_ICOM:
        if(m_icomRepeaterHandler == NULL) {
            CIcomRepeaterProtocolHandler * icomRepeaterHandler = new CIcomRepeaterProtocolHandler(gatewayConfig.icomAddress, gatewayConfig.icomPort, repeaterAddress, repeaterPort);
			bool res = icomRepeaterHandler->open();
            if (!res) {
				CLog::logError("Cannot open the Icom repeater protocol handler");
				delete icomRepeaterHandler;
				icomRepeaterHandler = NULL;
			}
            m_icomRepeaterHandler = icomRepeaterHandler;
        }
        handler = m_icomRepeaterHandler;
        break;
    case HW_HOMEBREW:
        if(m_hbRepeaterHandler == NULL) {
    		CHBRepeaterProtocolHandler * hbRepeaterHandler = new CHBRepeaterProtocolHandler(gatewayConfig.hbAddress, gatewayConfig.hbPort);
			bool res = hbRepeaterHandler->open();
			if (!res) {
				CLog::logError("Cannot open the Homebrew repeater protocol handler");
				delete hbRepeaterHandler;
				hbRepeaterHandler = NULL;
			}
            m_hbRepeaterHandler = hbRepeaterHandler;
        }
        handler = m_hbRepeaterHandler;
        break;
    case HW_DUMMY:
        if(m_dummyRepeaterHandler == NULL) {
            CDummyRepeaterProtocolHandler * dummyRepeaterHandler = new CDummyRepeaterProtocolHandler;
			bool res = dummyRepeaterHandler->open();
			if (!res) {
				CLog::logError("Cannot open the Dummy repeater protocol handler");
				delete dummyRepeaterHandler;
				dummyRepeaterHandler = NULL;
			}
            m_dummyRepeaterHandler = dummyRepeaterHandler;
            handler = m_dummyRepeaterHandler;
        }
        break;
    default:
        break;
    }

    return handler;
}
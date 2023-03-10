/*
 *   Copyright (C) 2010-2015,2018 by Jonathan Naylor G4KLX
 *	 copyright (c) 2021 by Geoffrey F4FXL / KC3FRA
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

#ifndef	IRCDDBGatewayThread_H
#define	IRCDDBGatewayThread_H

#include "DummyRepeaterProtocolHandler.h"
#include "IcomRepeaterProtocolHandler.h"
#include "HBRepeaterProtocolHandler.h"
#include "DExtraProtocolHandlerPool.h"
#include "DPlusProtocolHandlerPool.h"
#include "RepeaterProtocolHandler.h"
#include "DStarGatewayStatusData.h"
#include "DCSProtocolHandlerPool.h"
#include "G2ProtocolHandlerPool.h"
#include "RemoteHandler.h"
#include "CacheManager.h"
#include "CallsignList.h"
#include "APRSHandler.h"
#include "IRCDDB.h"
#include "Timer.h"
#include "Defs.h"
#include "Thread.h"

class CDStarGatewayThread : public CThread{
public:
	CDStarGatewayThread(const std::string& logDir, const std::string& dataDir, const std::string& name);
	virtual ~CDStarGatewayThread();

	virtual void setGateway(GATEWAY_TYPE type, const std::string& callsign, const std::string& address);
	virtual void addRepeater(const std::string& callsign, const std::string& band, const std::string& address, unsigned int port, HW_TYPE hwType, const std::string& reflector, bool atStartup, RECONNECT reconnect, bool dratsEnabled, double frequency, double offset, double range, double latitude, double longitude, double agl, const std::string& description1, const std::string& description2, const std::string& url, IRepeaterProtocolHandler* handler, unsigned char band1 = 0x00U, unsigned char band2 = 0x00U, unsigned char band3 = 0x00U);

#ifdef USE_STARNET
#if defined(DEXTRA_LINK) || defined(DCS_LINK)
	virtual void addStarNet(const std::string& callsign, const std::string& logoff, const std::string& repeater, const std::string& infoText, const std::string& permanent, unsigned int userTimeout, unsigned int groupTimeout, STARNET_CALLSIGN_SWITCH callsignSwitch, bool txMsgSwitch, const std::string& reflector);
#else
	virtual void addStarNet(const std::string& callsign, const std::string& logoff, const std::string& repeater, const std::string& infoText, const std::string& permanent, unsigned int userTimeout, unsigned int groupTimeout, STARNET_CALLSIGN_SWITCH callsignSwitch, bool txMsgSwitch);
#endif
#endif
	virtual void setIcomRepeaterHandler(CIcomRepeaterProtocolHandler* handler);
	virtual void setHBRepeaterHandler(CHBRepeaterProtocolHandler* handler);
	virtual void setDummyRepeaterHandler(CDummyRepeaterProtocolHandler* handler);
	virtual void setIRC(CIRCDDB* irc);
	virtual void setLanguage(TEXT_LANG language);
	virtual void setDExtra(bool enabled, unsigned int maxDongles);
	virtual void setDPlus(bool enabled, unsigned int maxDongles, const std::string& login);
	virtual void setDCS(bool enabled);
	virtual void setXLX(bool enabled, const std::string& fileName);
#ifdef USE_CCS
	virtual void setCCS(bool enabled, const std::string& host);
#endif
	virtual void setLog(bool enabled);
	virtual void setAPRSWriters(CAPRSHandler* outgoingAprsWriter, CAPRSHandler* incomingAPRSHandler);
	virtual void setInfoEnabled(bool enabled);
	virtual void setEchoEnabled(bool enabled);
	virtual void setDTMFEnabled(bool enabled);
	virtual void setDDModeEnabled(bool enabled);
	virtual void setRemote(bool enabled, const std::string& password, unsigned int port);
	virtual void setLocation(double latitude, double longitude);
	virtual void setWhiteList(CCallsignList* list);
	virtual void setBlackList(CCallsignList* list);
	virtual void setRestrictList(CCallsignList* list);

	virtual CDStarGatewayStatusData* getStatus() const;

	virtual void kill();
	
protected:
	void* Entry();
	
private:
	std::string                  m_logDir;
	std::string					 m_dataDir;
	std::string                  m_name;
	bool                      m_killed;
	bool                      m_stopped;
	GATEWAY_TYPE              m_gatewayType;
	std::string                  m_gatewayCallsign;
	std::string                  m_gatewayAddress;
	CIcomRepeaterProtocolHandler*  m_icomRepeaterHandler;
	CHBRepeaterProtocolHandler*    m_hbRepeaterHandler;
	CDummyRepeaterProtocolHandler* m_dummyRepeaterHandler;
	CDExtraProtocolHandlerPool*    m_dextraPool;
	CDPlusProtocolHandlerPool*     m_dplusPool;
	CDCSProtocolHandlerPool*       m_dcsPool;
	CG2ProtocolHandlerPool*       m_g2HandlerPool;
	CAPRSHandler*              m_outgoingAprsHandler;
	CAPRSHandler*			   m_incomingAprsHandler;
	CIRCDDB*                  m_irc;
	CCacheManager             m_cache;
	TEXT_LANG                 m_language;
	bool                      m_dextraEnabled;
	unsigned int              m_dextraMaxDongles;
	bool                      m_dplusEnabled;
	unsigned int              m_dplusMaxDongles;
	std::string                  m_dplusLogin;
	bool                      m_dcsEnabled;
	bool			  m_xlxEnabled;
	std::string		  m_xlxHostsFileName;
	bool                      m_ccsEnabled;
	std::string                  m_ccsHost;
	bool                      m_infoEnabled;
	bool                      m_echoEnabled;
	bool                      m_dtmfEnabled;
	bool                      m_logEnabled;
	bool                      m_ddModeEnabled;
	IRCDDB_STATUS             m_lastStatus;
	CTimer                    m_statusTimer1;
	CTimer                    m_statusTimer2;
	bool                      m_remoteEnabled;
	std::string                  m_remotePassword;
	unsigned int              m_remotePort;
	CRemoteHandler*           m_remote;
	CTimer                    m_statusFileTimer;
	std::string                  m_status1;
	std::string                  m_status2;
	std::string                  m_status3;
	std::string                  m_status4;
	std::string                  m_status5;
	double                    m_latitude;
	double                    m_longitude;
	CCallsignList*            m_whiteList;
	CCallsignList*            m_blackList;
	CCallsignList*            m_restrictList;

	void processIrcDDB();
	void processRepeater(IRepeaterProtocolHandler* handler);
	void processDExtra();
	void processDPlus();
	void processDCS();
	void processG2();
	void processDD();

	void loadGateways();
	void loadAllReflectors();
	void loadReflectors(std::string hostFileName, DSTAR_PROTOCOL proto);
	void loadDExtraReflectors(const std::string& fileName);
	void loadDPlusReflectors(const std::string& fileName);
	void loadDCSReflectors(const std::string& fileName);
	void loadXLXReflectors();

	void writeStatus();

	void readStatusFiles();
	void readStatusFile(const std::string& filename, unsigned int n, std::string& var);
};

#endif

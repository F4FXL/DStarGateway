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

#include <string>
#include <cstdio>
#include <unordered_map>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>

#include "DGWRemoteControlApp.h"
#include "DGWRemoteControlConfig.h"
#include "Version.h"
#include "ProgramArgs.h"
#include "DStarDefines.h"
#include "SHA256.h"

const std::string NAME_OPTION("name");
const std::string REPEATER_PARAM("Callsign");
const std::string ACTION_PARAM("Action");
const std::string RECONNECT_PARAM("Param1");
const std::string REFLECTOR_PARAM("Param2");
const std::string CONFIG_FILENAME("dgwremotecontrol.cfg");

int main(int argc, const char* argv[])
{
    std::string name, repeater, actionText, user, reflector;
    RECONNECT reconnect;

    if(!getCLIParams(argc, argv, name, repeater, actionText, reconnect, user, reflector)) {
		::fprintf(stderr, "\ndgwremotecontrol v%s : invalid command line usage:\n\n", LONG_VERSION.c_str());
        ::fprintf(stderr, "\t\tdgwremotecontrol [-name <name>] <repeater> link <reconnect> <reflector>\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [-name <name>] <repeater> unlink\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [-name <name>] <starnet> drop <user>\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [-name <name>] <starnet> drop all\n\n");
        return 1;
    }

    CDGWRemoteControlConfig config(std::string(CFG_DIR) + "/" + CONFIG_FILENAME);
	TRemoteGateway gatewayConfig;

	if(!config.load() || !config.getGateway(name, gatewayConfig)) {
		::fprintf(stderr, "Configuration failed to load\n");
		return 1;
	}

	std::string password(gatewayConfig.m_password);

    CRemoteControlRemoteControlHandler handler(gatewayConfig.m_address, gatewayConfig.m_port);

    bool ret = handler.open();
	if (!ret) {
		::fprintf(stderr, "dgwremotecontrol: unable to open the UDP port\n");
		return 1;
	}

	ret = handler.login();
	if (!ret) {
		handler.close();
		::fprintf(stderr, "dgwremotecontrol: unable to login to the gateway/starnetserver\n");
		return 1;
	}

	unsigned int count = 0U;
	while (count < 10U) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100U));

		RC_TYPE type = handler.readType();
		if (type == RCT_RANDOM)
			break;

		if (type == RCT_NONE)
			handler.retry();

		count++;
	}

	if (count >= 10U) {
		handler.close();
		::fprintf(stderr, "dgwremotecontrol: unable to get a response from the gateway/starnetserver\n");
		return 1;
	}

	unsigned int rnd = handler.readRandom();
	sendHash(&handler, password, rnd);

	count = 0U;
	while (count < 10U) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100U));

		RC_TYPE type = handler.readType();
		if (type == RCT_ACK)
			break;

		if (type == RCT_NAK) {
			handler.close();
			::fprintf(stderr, "dgwremotecontrol: invalid password sent to the gateway/starnetserver\n");
			return 1;
		}

		if (type == RCT_NONE)
			handler.retry();

		count++;
	}

	if (count >= 10U) {
		handler.close();
		::fprintf(stderr, "dgwremotecontrol: unable to get a response from the gateway/starnetserver\n");
		return 1;
	}

	handler.setLoggedIn(true);

	if (actionText == "drop")
		handler.logoff(repeater, user);
	else
		handler.link(repeater, reconnect, reflector);

	count = 0U;
	while (count < 10U) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100U));

		RC_TYPE type = handler.readType();
		if (type == RCT_ACK)
			break;

		if (type == RCT_NAK) {
			handler.close();
			::fprintf(stderr, "dgwremotecontrol: drop/link/unlink command rejected by the gateway/starnetserver\n");
			return 1;
		}

		if (type == RCT_NONE)
			handler.retry();

		count++;
	}

	if (count >= 10U) {
		handler.close();
		::fprintf(stderr, "dgwremotecontrol: unable to get a response from the gateway/starnetserver\n");
		return 1;
	}

	::fprintf(stdout, "dgwremotecontrol: command accepted by the gateway/starnetserver\n");

	handler.logout();
	handler.close();


    return 0;
}

bool getCLIParams(int argc, const char* argv[], std::string& name, std::string& repeater, std::string& actionText, RECONNECT& reconnect, std::string& user, std::string& reflector)
{
    bool ret = true;

    std::unordered_map<std::string, std::string> namedArgs;
    std::vector<std::string> positionalArgs;

    CProgramArgs::eatArguments(argc, argv, namedArgs, positionalArgs);

    if(namedArgs.count(NAME_OPTION) > 0U) {
        name = namedArgs[NAME_OPTION];
    }

    if(positionalArgs.size() < 1) {
        return false;
    }

    // dgwremotecontrol [-name <name>] <repeater> link <reconnect> <reflector>
    if(positionalArgs.size() == 4U) {
        repeater = positionalArgs[0];

        actionText = boost::to_lower_copy(positionalArgs[1]);
        if(actionText != "link") {
            ::fprintf(stderr, "Invalid action %s. Expected link\n", positionalArgs[1].c_str());
            ret = false;
        }

        std::string reconnectText = boost::to_lower_copy(positionalArgs[2]);
        if(reconnectText == "never")		reconnect = RECONNECT_NEVER;
        else if(reconnectText == "5")		reconnect = RECONNECT_5MINS;
        else if(reconnectText == "10")		reconnect = RECONNECT_10MINS;
        else if(reconnectText == "15")		reconnect = RECONNECT_15MINS;
        else if(reconnectText == "20")		reconnect = RECONNECT_20MINS;
        else if(reconnectText == "25")		reconnect = RECONNECT_25MINS;
        else if(reconnectText == "30")		reconnect = RECONNECT_30MINS;
        else if(reconnectText == "60")		reconnect = RECONNECT_60MINS;
        else if(reconnectText == "90")		reconnect = RECONNECT_90MINS;
        else if(reconnectText == "120")		reconnect = RECONNECT_120MINS;
        else if(reconnectText == "180")		reconnect = RECONNECT_180MINS;
        else if(reconnectText == "fixed")	reconnect = RECONNECT_FIXED;
        else {
            ::fprintf(stderr, "Invalid reconnect value %s. Valid values are 5,10,15,20,25,30,60,90,120,180,fixed\n", positionalArgs[2].c_str());
            ret = false;
        }
        reflector = boost::to_upper_copy(positionalArgs[3]);
        boost::replace_all(reflector, "_", " ");
    }
    // dgwremotecontrol [-name <name>] <repeater> unlink
    else if(positionalArgs.size() == 2U) {
        repeater = positionalArgs[0];
        actionText = boost::to_lower_copy(positionalArgs[1]);
        if(actionText != "unlink") {
            ::fprintf(stderr, "Invalid action %s. Expected unlink\n", positionalArgs[1].c_str());
            ret = false;
        }
        reconnect = RECONNECT_NEVER;
    }
    // dgwremotecontrol [-name <name>] <starnet> drop <user>
    // dgwremotecontrol [-name <name>] <starnet> drop all
    else if(positionalArgs.size() == 3U) {
        repeater = positionalArgs[0];

        actionText = boost::to_lower_copy(positionalArgs[1]);
        if(actionText != "drop") {
            ::fprintf(stderr, "Invalid action %s. Expected drop\n", positionalArgs[1].c_str());
            ret = false;
        }

        user = positionalArgs[2];
    }

    boost::to_upper(repeater);
    repeater.resize(LONG_CALLSIGN_LENGTH, ' ');
    boost::replace_all(repeater, "_", " ");

    boost::to_upper(reflector);
    reflector.resize(LONG_CALLSIGN_LENGTH, ' ');
    boost::replace_all(reflector, "_", " ");

    boost::to_upper(user);
    user.resize(LONG_CALLSIGN_LENGTH, ' ');
    boost::replace_all(user, "_", " ");

    return ret;
}

void sendHash(CRemoteControlRemoteControlHandler* handler, const std::string& password, unsigned int rnd)
{
	assert(handler != NULL);

	unsigned int len = password.length() + sizeof(unsigned int);
	unsigned char*  in = new unsigned char[len];
	unsigned char* out = new unsigned char[32U];

	::memcpy(in, &rnd, sizeof(unsigned int));
	for (unsigned int i = 0U; i < password.length(); i++)
		in[i + sizeof(unsigned int)] = password.at(i);

	CSHA256 sha256;
	sha256.buffer(in, len, out);

	handler->sendHash(out, 32U);

	delete[] in;
	delete[] out;
}
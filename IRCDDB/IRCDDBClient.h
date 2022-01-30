/*

CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2011,2012   Jonathan Naylor, G4KLX

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#pragma once

#include <string>
#include <vector>

#include "IRCDDB.h"

struct CIRCDDBPrivate;

class CIRCDDBClient : public CIRCDDB{
public:
	CIRCDDBClient(const std::string& hostName, unsigned int port, const std::string& callsign, const std::string& password, const std::string& versionInfo,
		const std::string& localAddr = std::string(""), bool isQuadNet = false);
	~CIRCDDBClient();

	// A false return implies a network error, or unable to log in
	bool open();

	// rptrQTH can be called multiple times if necessary
	//   callsign     The callsign of the repeater
	//   latitude     WGS84 position of antenna in degrees, positive value -> NORTH
	//   longitude    WGS84 position of antenna in degrees, positive value -> EAST
	//   desc1, desc2   20-character description of QTH
	//   infoURL      URL of a web page with information about the repeater
	void rptrQTH( const std::string& callsign, double latitude, double longitude, const std::string& desc1, const std::string& desc2, const std::string& infoURL);

	// rptrQRG can be called multiple times if necessary
	//  callsign      callsign of the repeater
	//  txFrequency   repeater TX frequency in MHz
	//  duplexShift   duplex shift in MHz (positive or negative value):  RX_freq = txFrequency + duplexShift
	//  range       range of the repeater in meters (meters = miles * 1609.344)
	//  agl         height of the antenna above ground in meters (meters = feet * 0.3048)
	void rptrQRG( const std::string& callsign, double txFrequency, double duplexShift, double range, double agl );

	// If you call this method once, watchdog messages will be sent to the
	// to the ircDDB network every 15 minutes. Invoke this method every 1-2 minutes to indicate
	// that the gateway is working properly. After activating the watchdog, a red LED will be displayed
	// on the ircDDB web page if this method is not called within a period of about 30 minutes.
	// The string wdInfo should contain information about the source of the alive messages, e.g.,
	// version of the RF decoding software. For example, the ircDDB java software sets this
	// to "rpm_ircddbmhd-x.z-z".  The string wdInfo must contain at least one non-space character.
	void kickWatchdog(const std::string& callsign, const std::string& wdInfo);

	// get internal network status
	int getConnectionState();
	// one of these values is returned:
	//  0  = not (yet) connected to the IRC server
	//  1-6  = a new connection was established, download of repeater info etc. is
	//         in progress
	//  7 = the ircDDB connection is fully operational
	//  10 = some network error occured, next state is "0" (new connection attempt)

	// Send heard data, a false return implies a network error
	bool sendHeard(const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1, const std::string& rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3);

	// same as sendHeard with two new fields:
	//   network_destination:  empty string or 8-char call sign of the repeater
	//	    or reflector, where this transmission is relayed to.
	//   tx_message:  20-char TX message or empty string, if the user did not
	//       send a TX message
	bool sendHeardWithTXMsg(const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1, const std::string& rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, const std::string& network_destination, const std::string& tx_message);

	// this method should be called at the end of a transmission
	//  num_dv_frames: number of DV frames sent out (96 bit frames, 20ms)
	//  num_dv_silent_frames: number of DV silence frames sent out in the
	//	last transmission, or -1 if the information is not available
	//  num_bit_errors: number of bit errors of the received data. This should
	//      be the derived from the first Golay block of the voice data. This
	//      error correction code only looks at 24 bits of the 96 bit frame.
	//      So, the overall bit error rate is calculated like this:
	//      BER = num_bit_errors / (num_dv_frames * 24)
	//      Set num_bit_errors = -1, if the error information is not available.
	bool sendHeardWithTXStats(const std::string& myCall, const std::string& myCallExt, const std::string& yourCall, const std::string& rpt1, const std::string& rpt2, unsigned char flag1, unsigned char flag2, unsigned char flag3, int num_dv_frames, int num_dv_silent_frames, int num_bit_errors);

	// The following three functions don't block waiting for a reply, they just send the data

	// Send query for a gateway/reflector, a false return implies a network error
	bool findGateway(const std::string& gatewayCallsign);

	// Send query for a repeater module, a false return implies a network error
	bool findRepeater(const std::string& repeaterCallsign);

	// Send query for a user, a false return implies a network error
	bool findUser(const std::string& userCallsign);

	// notify another repeater for NAT Traversal, a false return implies a network error
	bool notifyRepeaterG2NatTraversal(const std::string& repeater);
	bool notifyRepeaterDextraNatTraversal(const std::string& repeater, unsigned int myport);
	bool notifyRepeaterDPlusNatTraversal(const std::string& repeater, unsigned int myport);

	// Support for the Smart Group Server
	void sendDStarGatewayInfo(const std::string subcommand, const std::vector<std::string> parms);

	// The following functions are for processing received messages
	
	// Get the waiting message type
	IRCDDB_RESPONSE_TYPE getMessageType();

	// Get a gateway message, as a result of IDRT_REPEATER returned from getMessageType()
	// A false return implies a network error
	bool receiveRepeater(std::string& repeaterCallsign, std::string& gatewayCallsign, std::string& address);

	// Get a gateway message, as a result of IDRT_GATEWAY returned from getMessageType()
	// A false return implies a network error
	bool receiveGateway(std::string& gatewayCallsign, std::string& address);

	// Get a user message, as a result of IDRT_USER returned from getMessageType()
	// A false return implies a network error
	bool receiveUser(std::string& userCallsign, std::string& repeaterCallsign, std::string& gatewayCallsign, std::string& address);

	bool receiveUser(std::string& userCallsign, std::string& repeaterCallsign, std::string& gatewayCallsign, std::string& address, std::string& timeStamp);

    bool receiveNATTraversalG2(std::string& address);
	bool receiveNATTraversalDextra(std::string& address, std::string& remotePort);
	bool receiveNATTraversalDPlus(std::string& address, std::string& remotePort);

	void close();		// Implictely kills any threads in the IRC code

private:
	struct CIRCDDBClientPrivate * const m_d;
	bool m_isQuadNet;
};



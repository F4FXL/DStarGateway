/*
 *   Copyright (C) 2012,2013,2015 by Jonathan Naylor G4KLX
 *   Copyright (C) 2011 by DV Developer Group. DJ0ABR
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

#include <cstdio>
#include "DTMF.h"

const unsigned char DTMF_MASK[] = {0x82U, 0x08U, 0x20U, 0x82U, 0x00U, 0x00U, 0x82U, 0x00U, 0x00U};
const unsigned char DTMF_SIG[]  = {0x82U, 0x08U, 0x20U, 0x82U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const unsigned char DTMF_SYM_MASK[] = {0x10U, 0x40U, 0x08U, 0x20U};
const unsigned char DTMF_SYM0[]     = {0x00U, 0x40U, 0x08U, 0x20U};
const unsigned char DTMF_SYM1[]     = {0x00U, 0x00U, 0x00U, 0x00U};
const unsigned char DTMF_SYM2[]     = {0x00U, 0x40U, 0x00U, 0x00U};
const unsigned char DTMF_SYM3[]     = {0x10U, 0x00U, 0x00U, 0x00U};
const unsigned char DTMF_SYM4[]     = {0x00U, 0x00U, 0x00U, 0x20U};
const unsigned char DTMF_SYM5[]     = {0x00U, 0x40U, 0x00U, 0x20U};
const unsigned char DTMF_SYM6[]     = {0x10U, 0x00U, 0x00U, 0x20U};
const unsigned char DTMF_SYM7[]     = {0x00U, 0x00U, 0x08U, 0x00U};
const unsigned char DTMF_SYM8[]     = {0x00U, 0x40U, 0x08U, 0x00U};
const unsigned char DTMF_SYM9[]     = {0x10U, 0x00U, 0x08U, 0x00U};
const unsigned char DTMF_SYMA[]     = {0x10U, 0x40U, 0x00U, 0x00U};
const unsigned char DTMF_SYMB[]     = {0x10U, 0x40U, 0x00U, 0x20U};
const unsigned char DTMF_SYMC[]     = {0x10U, 0x40U, 0x08U, 0x00U};
const unsigned char DTMF_SYMD[]     = {0x10U, 0x40U, 0x08U, 0x20U};
const unsigned char DTMF_SYMS[]     = {0x00U, 0x00U, 0x08U, 0x20U};
const unsigned char DTMF_SYMH[]     = {0x10U, 0x00U, 0x08U, 0x20U};

CDTMF::CDTMF() :
m_data(),
m_command(),
m_pressed(false),
m_releaseCount(0U),
m_pressCount(0U),
m_lastChar(' ')
{
}

CDTMF::~CDTMF()
{
}

bool CDTMF::decode(const unsigned char* ambe, bool end)
{
	// DTMF begins with these byte values
	if (!end && (ambe[0] & DTMF_MASK[0]) == DTMF_SIG[0] && (ambe[1] & DTMF_MASK[1]) == DTMF_SIG[1] &&
				(ambe[2] & DTMF_MASK[2]) == DTMF_SIG[2] && (ambe[3] & DTMF_MASK[3]) == DTMF_SIG[3] &&
				(ambe[4] & DTMF_MASK[4]) == DTMF_SIG[4] && (ambe[5] & DTMF_MASK[5]) == DTMF_SIG[5] &&
				(ambe[6] & DTMF_MASK[6]) == DTMF_SIG[6] && (ambe[7] & DTMF_MASK[7]) == DTMF_SIG[7] &&
				(ambe[8] & DTMF_MASK[8]) == DTMF_SIG[8]) {
		unsigned char sym0 = ambe[4] & DTMF_SYM_MASK[0];
		unsigned char sym1 = ambe[5] & DTMF_SYM_MASK[1];
		unsigned char sym2 = ambe[7] & DTMF_SYM_MASK[2];
		unsigned char sym3 = ambe[8] & DTMF_SYM_MASK[3];

		char c = ' ';
		if (sym0 == DTMF_SYM0[0] && sym1 == DTMF_SYM0[1] && sym2 == DTMF_SYM0[2] && sym3 == DTMF_SYM0[3])
			c = '0';
		else if (sym0 == DTMF_SYM1[0] && sym1 == DTMF_SYM1[1] && sym2 == DTMF_SYM1[2] && sym3 == DTMF_SYM1[3])
			c = '1';
		else if (sym0 == DTMF_SYM2[0] && sym1 == DTMF_SYM2[1] && sym2 == DTMF_SYM2[2] && sym3 == DTMF_SYM2[3])
			c = '2';
		else if (sym0 == DTMF_SYM3[0] && sym1 == DTMF_SYM3[1] && sym2 == DTMF_SYM3[2] && sym3 == DTMF_SYM3[3])
			c = '3';
		else if (sym0 == DTMF_SYM4[0] && sym1 == DTMF_SYM4[1] && sym2 == DTMF_SYM4[2] && sym3 == DTMF_SYM4[3])
			c = '4';
		else if (sym0 == DTMF_SYM5[0] && sym1 == DTMF_SYM5[1] && sym2 == DTMF_SYM5[2] && sym3 == DTMF_SYM5[3])
			c = '5';
		else if (sym0 == DTMF_SYM6[0] && sym1 == DTMF_SYM6[1] && sym2 == DTMF_SYM6[2] && sym3 == DTMF_SYM6[3])
			c = '6';
		else if (sym0 == DTMF_SYM7[0] && sym1 == DTMF_SYM7[1] && sym2 == DTMF_SYM7[2] && sym3 == DTMF_SYM7[3])
			c = '7';
		else if (sym0 == DTMF_SYM8[0] && sym1 == DTMF_SYM8[1] && sym2 == DTMF_SYM8[2] && sym3 == DTMF_SYM8[3])
			c = '8';
		else if (sym0 == DTMF_SYM9[0] && sym1 == DTMF_SYM9[1] && sym2 == DTMF_SYM9[2] && sym3 == DTMF_SYM9[3])
			c = '9';
		else if (sym0 == DTMF_SYMA[0] && sym1 == DTMF_SYMA[1] && sym2 == DTMF_SYMA[2] && sym3 == DTMF_SYMA[3])
			c = 'A';
		else if (sym0 == DTMF_SYMB[0] && sym1 == DTMF_SYMB[1] && sym2 == DTMF_SYMB[2] && sym3 == DTMF_SYMB[3])
			c = 'B';
		else if (sym0 == DTMF_SYMC[0] && sym1 == DTMF_SYMC[1] && sym2 == DTMF_SYMC[2] && sym3 == DTMF_SYMC[3])
			c = 'C';
		else if (sym0 == DTMF_SYMD[0] && sym1 == DTMF_SYMD[1] && sym2 == DTMF_SYMD[2] && sym3 == DTMF_SYMD[3])
			c = 'D';
		else if (sym0 == DTMF_SYMS[0] && sym1 == DTMF_SYMS[1] && sym2 == DTMF_SYMS[2] && sym3 == DTMF_SYMS[3])
			c = '*';
		else if (sym0 == DTMF_SYMH[0] && sym1 == DTMF_SYMH[1] && sym2 == DTMF_SYMH[2] && sym3 == DTMF_SYMH[3])
			c = '#';

		if (c == m_lastChar) {
			m_pressCount++;
		} else {
			m_lastChar = c;
			m_pressCount = 0U;
		}

		if (c != ' ' && !m_pressed && m_pressCount >= 3U) {
			m_data.push_back(c);
			m_releaseCount = 0U;
			m_pressed = true;
		}

		return c != ' ';
	} else {
		// If it is not a DTMF Code
		if ((end || m_releaseCount >= 100U) && m_data.size() > 0U) {
			m_command = m_data;
			m_data.clear();
			m_releaseCount = 0U;
		}

		m_pressed = false;
		m_releaseCount++;
		m_pressCount = 0U;
		m_lastChar = ' ';

		return false;
	}
}

bool CDTMF::hasCommand() const
{
	return m_command.size() > 0;
}

// DTMF to YOUR call command
std::string CDTMF::translate()
{
	std::string command = m_command;
	m_command.clear();

	if (0 == command.size())
		return std::string("");

	if (0 == command.compare("#"))
		return "       U";

	if (0 == command.compare("0"))
		return "       I";

	if (0 == command.compare("A"))
		return "CA      ";

	if (0 == command.compare("00"))
		return "       I";

	if (0 == command.compare("**"))
		return "       L";

	if      (command.at(0) == '*')
		return processReflector("REF", command.substr(1));
	else if (command.at(0) == 'B')
		return processReflector("XRF", command.substr(1));
	else if (command.at(0) == 'D')
		return processReflector("DCS", command.substr(1));
	else
		return processCCS(command);
}

void CDTMF::reset()
{
	m_data.clear();
	m_command.clear();
	m_pressed = false;
	m_pressCount = 0U;
	m_releaseCount = 0U;
	m_lastChar = ' ';
}

std::string CDTMF::processReflector(const std::string& prefix, const std::string& command) const
{
	unsigned int len = command.size();

	char c = command.at(len - 1U);
	if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
		if (len < 2U || len > 4U)
			return std::string("");

		unsigned long n = std::stoul(command.substr(0, len-1U));
		if (n == 0UL)
			return std::string("");

		char ostr[32];
		snprintf(ostr, 32, "%s%03lu%cL", prefix.c_str(), n, c);
	
		return std::string(ostr);
	} else {
		if (len < 3U || len > 5U)
			return std::string("");

		unsigned long n1 = std::stoul(command.substr(0,len-2U));
		if (n1 == 0UL)
			return std::string("");

		unsigned long n2 = std::stoul(command.substr(2));
		if (n2 == 0UL || n2 > 26UL)
			return std::string("");

		c = 'A' + n2 - 1UL;

		char ostr[32];
		snprintf(ostr, 32, "%s%03lu%cL", prefix.c_str(), n1, c);
	
		return std::string(ostr);
	}
}

std::string CDTMF::processCCS(const std::string& command) const
{
	unsigned int len = command.size();

	std::string out("");
	char ostr[32];
	switch (len) {
		case 3U: {
				// CCS7 for local repeater without band
				unsigned long n = std::stoul(command);
				if (n == 0UL)
					return out;
				snprintf(ostr, 32, "C%03lu    ", n);
			}
			break;
		case 4U: {
				char c = command.at(3U);
				if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
					// CCS7 for local repeater with band
					unsigned long n = std::stoul(command.substr(0, 3));
					if (n == 0UL)
						return out;
					snprintf(ostr, 32, "C%03lu%c   ", n, c);
				} else {
					// CCS7 for local user
					unsigned long n = std::stoul(command);
					if (n == 0UL)
						return out;
					snprintf(ostr, 32, "C%04lu   ", n);
				}
			}
			break;
		case 5U: {
				char c = command.at(4U);
				if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
					// CCS7 for local hostspot with band
					unsigned long n = std::stoul(command.substr(0, 4));
					if (n == 0UL)
						return out;
					snprintf(ostr, 32, "C%04lu%c  ", n, c);
				}
			}
			break;
		case 6U: {
				// CCS7 for full repeater without band
				unsigned long n = std::stoul(command);
				if (n == 0UL)
					return out;
				snprintf(ostr, 32, "C%06lu ", n);
			}
			break;
		case 7U: {
				char c = command.at(6U);
				if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
					// CCS7 for full repeater with band
					unsigned long n = std::stoul(command.substr(0, 6));
					if (n == 0UL)
						return out;
					snprintf(ostr, 32, "C%06lu%c", n, c);
				} else {
					// CCS7 for full user or CCS7 for full hostpot without band
					unsigned long n = std::stoul(command);
					if (n == 0UL)
						return out;
					snprintf(ostr, 32, "C%07lu", n);
				}
			}
			break;
		case 8U: {
				char c = command.at(7U);
				if (c == 'A' || c == 'B' || c == 'C' || c == 'D') {
					// CCS7 for full hotspot with band
					unsigned long n = std::stoul(command.substr(0, 7));
					if (n == 0UL)
						return out;
					snprintf(ostr, 32, "C%07lu%c", n, c);
				}
			}
			break;
		default:
			break;
	}
	out = ostr;
	return out;
}

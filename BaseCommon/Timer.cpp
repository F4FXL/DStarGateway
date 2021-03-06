/*
 *   Copyright (C) 2009,2010 by Jonathan Naylor G4KLX
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

#include <cassert>
#include "Timer.h"


CTimer::CTimer(unsigned int ticksPerSec, unsigned int secs, unsigned int msecs) :
m_ticksPerSec(ticksPerSec),
m_timeout(0U),
m_timer(0U)
{
	assert(ticksPerSec > 0U);

	if (secs > 0U || msecs > 0U) {
		// m_timeout = ((secs * 1000U + msecs) * m_ticksPerSec) / 1000U + 1U;
		unsigned long long temp = (secs * (unsigned long long)(1000) + msecs) * m_ticksPerSec;
		m_timeout = (unsigned int)(temp / (unsigned long long)(1000) + (unsigned long long)(1));
	}
}

CTimer::~CTimer()
{
}

void CTimer::setTimeout(unsigned int secs, unsigned int msecs)
{
	if (secs > 0U || msecs > 0U) {
		// m_timeout = ((secs * 1000U + msecs) * m_ticksPerSec) / 1000U + 1U;
		unsigned long long temp = (secs * (unsigned long long)(1000) + msecs) * m_ticksPerSec;
		m_timeout = (unsigned int)(temp / (unsigned long long)(1000) + (unsigned long long)(1));
	} else {
		m_timeout = 0U;
		m_timer = 0U;
	}
}

unsigned int CTimer::getTimeout() const
{
	if (m_timeout == 0U)
		return 0U;

	return (m_timeout - 1U) / m_ticksPerSec;
}

unsigned int CTimer::getTimer() const
{
	if (m_timer == 0U)
		return 0U;

	return (m_timer - 1U) / m_ticksPerSec;
}

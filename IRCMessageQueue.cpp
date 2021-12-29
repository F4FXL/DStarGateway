/*
CIRCDDB - ircDDB client library in C++

Based on code by:
Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)

Completely rewritten by:
Copyright (c) 2017 by Thomas A. Early N7TAE

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

#include "IRCMessageQueue.h"

IRCMessageQueue::IRCMessageQueue()
{
	m_eof = false;
}

IRCMessageQueue::~IRCMessageQueue()
{
	std::lock_guard lockAccessQueue(accessMutex);
	while (! m_queue.empty()) {
		delete m_queue.front();
		m_queue.pop();
	}
}

bool IRCMessageQueue::isEOF()
{
	return m_eof;
}

void IRCMessageQueue::signalEOF()
{
	m_eof = true;
}

bool IRCMessageQueue::messageAvailable()
{
  std::lock_guard lockAccessQueue(accessMutex);
  bool retv = ! m_queue.empty();

  return retv;
}

IRCMessage *IRCMessageQueue::peekFirst()
{
	std::lock_guard lockAccessQueue(accessMutex);
	IRCMessage *msg = m_queue.empty() ? NULL : m_queue.front();
	return msg;
}

IRCMessage *IRCMessageQueue::getMessage()
{
	std::lock_guard lockAccessQueue(accessMutex);
	IRCMessage *msg = m_queue.empty() ? NULL : m_queue.front();
	if (msg)
		m_queue.pop();
	
	return msg;
}

void IRCMessageQueue::putMessage(IRCMessage *m)
{
	std::lock_guard lockAccessQueue(accessMutex);
	m_queue.push(m);
}





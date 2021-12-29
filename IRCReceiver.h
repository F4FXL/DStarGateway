/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)
Copyright (C) 2012   Jonathan Naylor, G4KLX
Copyright (c) 2017 by Thomas A. Early N7TAE
Copyright (c) 2021 by Thomas Geoffrey Merck F4FXL / KC3FRA

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

#include <future>

#include "IRCMessageQueue.h"

class IRCReceiver 
{
public:
	IRCReceiver(int sock, IRCMessageQueue *q);
	~IRCReceiver();

	void startWork();
	void stopWork();

protected:
	void Entry();

private:
	bool m_terminateThread;
	int m_sock;
	IRCMessageQueue *m_recvQ;
	std::future<void> m_future;
};

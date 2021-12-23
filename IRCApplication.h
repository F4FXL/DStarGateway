/*

CIRCDDB - ircDDB client library in C++

Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)
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

#pragma once

#include "IRCMessageQueue.h"

class IRCApplication
{
public:
  virtual void userJoin(const std::string& nick, const std::string& name, const std::string& host) = 0;
  virtual void userLeave(const std::string& nick) = 0;
  virtual void userChanOp(const std::string& nick, bool op) = 0;
  virtual void userListReset(void) = 0;
	
  virtual void msgChannel(IRCMessage *m) = 0;
  virtual void msgQuery(IRCMessage *m) = 0;

  virtual void setCurrentNick(const std::string& nick) = 0;
  virtual void setTopic(const std::string& topic) = 0;
  
  virtual void setBestServer(const std::string& ircUser) = 0;

  virtual void setSendQ(IRCMessageQueue *s) = 0;
  virtual IRCMessageQueue *getSendQ(void) = 0;
};

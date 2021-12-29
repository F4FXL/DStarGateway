/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010-2011   Michael Dirska, DL1BFF (dl1bff@mdx.de)
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

#include "IRCMessage.h"
#include "Utils.h"

IRCMessage::IRCMessage()
{
	numParams = 0;
	prefixParsed = false;
}

IRCMessage::IRCMessage(const std::string& toNick, const std::string& msg)
{
	command.assign("PRIVMSG");
	numParams = 2;
	params.push_back(toNick);
	params.push_back(msg);
	prefixParsed = false;
}

IRCMessage::IRCMessage(const std::string& cmd)
{
	command = cmd;
	numParams = 0;
	prefixParsed = false;
}

IRCMessage::~IRCMessage()
{
}


void IRCMessage::addParam(const std::string& p)
{
	params.push_back(p);
	numParams = params.size();
}

int IRCMessage::getParamCount()
{
	return params.size();
}

std::string IRCMessage::getParam(int pos)
{
	return params[pos];
}

std::string IRCMessage::getCommand()
{
	return command;
}

	
bool IRCMessage::parsePrefix()
{
	std::string::size_type p1 = prefix.find('!');
	if (std::string::npos == p1)
		return false;
	std::string::size_type p2 = prefix.find('@');
	if (std::string::npos == p2)
		return false;

	prefixComponents.push_back(prefix.substr(0, p1));
	prefixComponents.push_back(prefix.substr(p1+1, p2-p1-1));
	prefixComponents.push_back(prefix.substr(p2 + 1));
	return true;
}

std::string& IRCMessage::getPrefixNick()
{
	if (!prefixParsed)
		prefixParsed = parsePrefix();

	return prefixParsed ? prefixComponents[0] : prefix;
}

std::string& IRCMessage::getPrefixName()
{
	if (!prefixParsed)
		prefixParsed = parsePrefix();
  
	return prefixParsed ? prefixComponents[1] : prefix;
}

std::string& IRCMessage::getPrefixHost()
{
	if (!prefixParsed)
		prefixParsed = parsePrefix();
  
	return prefixParsed ? prefixComponents[2] : prefix;
}

void IRCMessage::composeMessage(std::string& output)
{
	std::string o;

	if (prefix.size() > 0)
		o = std::string(":") + prefix + std::string(" ");

	o.append(command);

	for (int i=0; i < numParams; i++) {
		if (i == (numParams - 1))
			o.append(std::string(" :") + params[i]);
		else
			o.append(std::string(" ") + params[i]);
	}

	o.append(std::string("\r\n"));

	output = o;
}

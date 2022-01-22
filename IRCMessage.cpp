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
	m_numParams = 0;
	m_prefixParsed = false;
}

IRCMessage::IRCMessage(const std::string& toNick, const std::string& msg)
{
	m_command.assign("PRIVMSG");
	m_numParams = 2;
	m_params.push_back(toNick);
	m_params.push_back(msg);
	m_prefixParsed = false;
}

IRCMessage::IRCMessage(const std::string& cmd)
{
	m_command = cmd;
	m_numParams = 0;
	m_prefixParsed = false;
}

IRCMessage::~IRCMessage()
{
}


void IRCMessage::addParam(const std::string& p)
{
	m_params.push_back(p);
	m_numParams = m_params.size();
}

int IRCMessage::getParamCount()
{
	return m_params.size();
}

std::string IRCMessage::getParam(int pos)
{
	return m_params[pos];
}

std::string IRCMessage::getCommand()
{
	return m_command;
}

	
bool IRCMessage::parsePrefix()
{
	std::string::size_type p1 = m_prefix.find('!');
	if (std::string::npos == p1)
		return false;
	std::string::size_type p2 = m_prefix.find('@');
	if (std::string::npos == p2)
		return false;

	m_prefixComponents.push_back(m_prefix.substr(0, p1));
	m_prefixComponents.push_back(m_prefix.substr(p1+1, p2-p1-1));
	m_prefixComponents.push_back(m_prefix.substr(p2 + 1));
	return true;
}

std::string& IRCMessage::getPrefixNick()
{
	if (!m_prefixParsed)
		m_prefixParsed = parsePrefix();

	return m_prefixParsed ? m_prefixComponents[0] : m_prefix;
}

std::string& IRCMessage::getPrefixName()
{
	if (!m_prefixParsed)
		m_prefixParsed = parsePrefix();
  
	return m_prefixParsed ? m_prefixComponents[1] : m_prefix;
}

std::string& IRCMessage::getPrefixHost()
{
	if (!m_prefixParsed)
		m_prefixParsed = parsePrefix();
  
	return m_prefixParsed ? m_prefixComponents[2] : m_prefix;
}

void IRCMessage::composeMessage(std::string& output)
{
	std::string o;

	if (m_prefix.size() > 0)
		o = std::string(":") + m_prefix + std::string(" ");

	o.append(m_command);

	for (int i=0; i < m_numParams; i++) {
		if (i == (m_numParams - 1))
			o.append(std::string(" :") + m_params[i]);
		else
			o.append(std::string(" ") + m_params[i]);
	}

	o.append(std::string("\r\n"));

	output = o;
}

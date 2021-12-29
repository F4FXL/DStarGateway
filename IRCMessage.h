/*
CIRCDDB - ircDDB client library in C++

Copyright (C) 2010   Michael Dirska, DL1BFF (dl1bff@mdx.de)
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

#include <string>
#include <vector>

class IRCMessage
{
public:
	IRCMessage();
	IRCMessage(const std::string& toNick, const std::string& msg);
	IRCMessage(const std::string& command);
	~IRCMessage();

	std::string prefix;
	std::string command;
	std::vector<std::string> params;

	int numParams;
	std::string& getPrefixNick();
	std::string& getPrefixName();
	std::string& getPrefixHost();
	void composeMessage(std::string& output);
	void addParam(const std::string& p);

	std::string getCommand();
	std::string getParam(int pos);
	int getParamCount();

private:
	bool parsePrefix();
	std::vector<std::string> prefixComponents;
	bool prefixParsed;
};

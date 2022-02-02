/*
 *   Copyright (C) 2010,2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include "Version.h"
#include "ProgramArgs.h"

const std::string NAME_OPTION("name");
const std::string REPEATER_PARAM("Callsign");
const std::string ACTION_PARAM("Action");
const std::string RECONNECT_PARAM("Param1");
const std::string REFLECTOR_PARAM("Param2");

int main(int argc, const char* argv[])
{
    std::string name;

    std::unordered_map<std::string, std::string> namedArgs;
    std::vector<std::string> positionalArgs;

    CProgramArgs::eatArguments(argc, argv, namedArgs, positionalArgs);

    if(namedArgs.size() == 0U && positionalArgs.size() == 0U) {
		::fprintf(stderr, "\ndgwremotecontrol v%s : invalid command line usage:\n\n", LONG_VERSION.c_str());
        ::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <repeater> link <reconnect> <reflector>\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <repeater> unlink\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <starnet> drop <user>\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <starnet> drop all\n\n");
        return 1;
    }


    return 0;
}
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

#include "optionparser.h"
#include "Version.h"

const std::string NAME_OPTION("name");
const std::string REPEATER_PARAM("Callsign");
const std::string ACTION_PARAM("Action");
const std::string RECONNECT_PARAM("Param1");
const std::string REFLECTOR_PARAM("Param2");

int main(int argc, const char* argv[])
{
    unsigned int paramsCount = 0U;
    optionparser::OptionParser p;
    p.exit_on_failure(false);
    p.throw_on_failure(false);

    p.add_option("--" + NAME_OPTION).default_value("").help("Gateway Name");
    p.add_option(REPEATER_PARAM);
    p.add_option(ACTION_PARAM);
    p.add_option(RECONNECT_PARAM);
    p.add_option(REFLECTOR_PARAM);

    p.eat_arguments(argc, argv);

    if(p.get_value(NAME_OPTION)) {
        std::cout << p.get_value<std::string>(NAME_OPTION) << std::endl;
        paramsCount++;
    }

    if(p.get_value(REPEATER_PARAM)) {
        std::cout << p.get_value<std::string>(REPEATER_PARAM) << std::endl;
        paramsCount++;
    }

    if(p.get_value(ACTION_PARAM)) {
        std::cout << p.get_value<std::string>(ACTION_PARAM) << std::endl;
        paramsCount++;
    }

    if(p.get_value(RECONNECT_PARAM)) {
        std::cout << p.get_value<std::string>(RECONNECT_PARAM) << std::endl;
        paramsCount++;
    }

    if(p.get_value(REFLECTOR_PARAM)) {
        std::cout << p.get_value<std::string>(REFLECTOR_PARAM) << std::endl;
        paramsCount++;
    }

    if(paramsCount < 2U) {
		::fprintf(stderr, "\ndgwremotecontrol %s : invalid command line usage:\n\n", LONG_VERSION.c_str());
        ::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <repeater> link <reconnect> <reflector>\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <repeater> unlink\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <starnet> drop <user>\n");
		::fprintf(stderr, "\t\tdgwremotecontrol [--name <name>] <starnet> drop all\n\n");
        return 1;
    }

    return 0;
}
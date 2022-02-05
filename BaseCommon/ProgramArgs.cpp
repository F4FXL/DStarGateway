/*
 *   Copyright (c) 2022 by Geoffrey Merck F4FXL / KC3FRA
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
#include <boost/algorithm/string.hpp>

#include "ProgramArgs.h"

void CProgramArgs::eatArguments(int argc, const char *argv[], std::unordered_map<std::string, std::string>& namedArgs,  std::vector<std::string>& positionalArgs)
{
    assert(argv != nullptr);

    namedArgs.clear();
    positionalArgs.clear();

    std::vector<std::string> programArgs;

    // Copy to a vector for easier handling, also skip program name
    for(int i = 1;i < argc; i++) {
        if(argv[i] != nullptr) {
            programArgs.push_back(std::string(argv[i]));
        }
    }

    // Consume Named args first
    for(auto it = programArgs.begin(); it != programArgs.end();) {
        if(boost::starts_with(*it, "-")) {
            std::string argName = boost::trim_left_copy_if(*it, [] (char c) { return c == '-'; });
            if(!argName.empty()) {
                namedArgs[argName] = "";
                it = programArgs.erase(it);
                if(it != programArgs.end()) {
                    namedArgs[argName] = *it;
                    it = programArgs.erase(it);
                }
            }
        }
        else {
            it++;
        }
    }

    //ProgramArgs now only contains pôsitional Args
    positionalArgs.assign(programArgs.begin(), programArgs.end());
}

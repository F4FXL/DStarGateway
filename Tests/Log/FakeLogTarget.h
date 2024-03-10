/*
 *   Copyright (C) 2021-2024 by Geoffrey Merck F4FXL / KC3FRA
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

#include <vector>
#include <iostream>

#include "LogTarget.h"

class CFakeLogTarget : public CLogTarget
{
public:
    CFakeLogTarget(LOG_SEVERITY logLevel) :
    CLogTarget(logLevel)
    {

    }

    virtual void printLogInt(const std::string& msg)
    {
        m_messages.push_back(msg);
        std::cout << msg;
    }

    std::vector<std::string> m_messages;
};
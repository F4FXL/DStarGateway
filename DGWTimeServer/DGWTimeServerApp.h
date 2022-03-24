/*
 *   Copyright (C) 2014 by Jonathan Naylor G4KLX
 *   Copyright (C) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

#pragma once

#include "TimeServerDefs.h"
#include "TimeServerConfig.h"
#include "TimeServerThread.h"

class CDGWTimeServerApp
{
public:
    CDGWTimeServerApp(const CTimeServerConfig * config);
    ~CDGWTimeServerApp();

    bool init();
    void run();

    static void sigHandler(int sig);
    static void sigHandlerFatal(int sig);
    static void terminateHandler();

private:
    bool createThread();

    static CDGWTimeServerApp * g_app;

    const CTimeServerConfig * m_config;
    CTimeServerThread * m_thread;
};

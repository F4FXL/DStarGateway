/*
 *   Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA
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

/*
 * A simple class to losely mimic wxthread
 */
#ifndef	Thread_H
#define	Thread_H

#include <thread>
#include <string>

class CThread {
public:
    CThread(const std::string& name);
    virtual ~CThread();
    void Create();
    void Run();
    void Wait();
    void Sleep(unsigned long milli);

protected:
    virtual void* Entry() = 0;

private:
    static void EntryRunner(CThread * thread);

    std::string m_name;
    std::thread m_thread;
};

#endif
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

#include <cassert>
#include "Thread.h"
#include "Log.h"

using namespace std;

CThread::CThread(const std::string& name) :
m_name(name)
{

}

CThread::~CThread()
{

}

void CThread::Create()
{
    //Actually does nothing, might be removed in the future
}

void CThread::Run()
{
    m_thread = thread(CThread::EntryRunner, this);
}

void CThread::Sleep(unsigned long millis)
{
    this_thread::sleep_for(chrono::milliseconds(millis));
}

void CThread::Wait()
{
    m_thread.join();
}

void CThread::EntryRunner(CThread * thread)
{
    assert(thread != nullptr);
    thread->Entry();
    CLog::logTrace("Exiting %s thread", thread->m_name.c_str());
}
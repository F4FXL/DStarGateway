/*
 *   Copyright (C) 2010,2012,2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2021-2022 by Geoffrey Merck F4FXL / KC3FRA
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

#include <string>

#include "SlowDataCollector.h"

class CSentenceCollector : public CSlowDataCollector
{
public:
    CSentenceCollector(unsigned char slowDataType, const std::string& sentenceIdentifier, unsigned char endMarker);

protected:
    bool addData(const unsigned char * data);

    virtual unsigned int getDataInt(unsigned char * data, unsigned int length) = 0;
    virtual bool getDataInt(std::string& data) = 0;
    static void dstarCallsignToAPRS(std::string& call);
    std::string getSentence();

private:
    virtual bool isValidSentence(const std::string& sentence) = 0;
    virtual void resetInt();

    std::string m_collector;
    std::string m_sentenceIdentifier;
    std::string m_sentence;
    unsigned char m_endMarker;
};
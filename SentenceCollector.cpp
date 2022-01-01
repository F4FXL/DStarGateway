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

#include <cassert>

#include "SentenceCollector.h"

CSentenceCollector::CSentenceCollector(unsigned char slowDataType, const std::string& sentenceIdentifier, unsigned char endMarker) :
CSlowDataCollector(slowDataType),
m_collector(),
m_sentenceIdentifier(sentenceIdentifier),
m_sentence(),
m_endMarker(endMarker)
{
    assert(!sentenceIdentifier.empty());
}

bool CSentenceCollector::addData(const unsigned char * data)
{
    m_collector.append(std::string((char*)data, 5U));

    std::string::size_type n2 = m_collector.find_last_of(m_endMarker);
	if (n2 == std::string::npos)
		return false;

    std::string::size_type n1 = m_collector.find(m_sentenceIdentifier);
	if (n1 == std::string::npos)
		return false;

    if(n2 < n1)
        return false;

    std::string sentence;
    for(unsigned int i = n1; i <= n2; i++) {
        sentence.push_back(m_collector[i] & 0x7FU);
    }

    bool ret = isValidSentence(sentence);
    if(ret) {
        m_sentence = sentence;
    } else {
        m_sentence.clear();
    }

    m_collector = m_collector.substr(n2);

    return ret;
}

std::string CSentenceCollector::getSentence()
{
    return m_sentence;
}

void CSentenceCollector::resetInt()
{
    m_collector.clear();
    m_sentence.clear();
}
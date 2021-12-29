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

#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>

#include "Utils.h"
#include "Config.h"
#include "Log.h"

CConfig::CConfig(const std::string filename) :
m_filename(filename)
{
}

CConfig::~CConfig()
{
    for(auto it = m_sections.begin(); it != m_sections.end(); it++) {
        delete it->second;
    }
    m_sections.clear();
}

bool CConfig::load()
{
    std::ifstream file;
    file.open(m_filename, std::ios::in);
    if(!file.is_open()) {
        CLog::logError("Failed to open configuration file %s", m_filename);
        return false;
    }

    CConfigSection * currentSection = nullptr;
    while(!file.eof()) {
        std::string line;
        std::getline(file, line);
        stripComment(line);

        if(isConfigSection(line)) {
            auto it = m_sections.find(line);
            currentSection = it != m_sections.end() ? it->second : nullptr;

            if(currentSection == nullptr) {
                currentSection = new CConfigSection(line);
                m_sections[currentSection->getName()] = currentSection;
            }
        } else if(isConfigValue(line) && currentSection != nullptr) {
            TConfigValue * configValue = readKeyAndValue(line);
            if(configValue != nullptr) {
                currentSection->getValues()[configValue->m_key] = configValue;
            }
        }
    }

    file.close();
    return true;
}

void CConfig::stripComment(std::string& s) const
{
    boost::trim(s);

    if(s.length() > 0 && s[0] == '#') {
        //we have a lien consisting only of comment, truncate it and leave
        s.resize(0);
        return;
    }

    char * sdup = strdup(s.c_str());
    char * uncommentedPart = strtok(sdup, "#");

    s = std::string(uncommentedPart != nullptr ? uncommentedPart : "");

    boost::trim(s);

    free(sdup);// could we use delete sdup here?
}

TConfigValue * CConfig::readKeyAndValue(const std::string s) const
{
    TConfigValue* res = nullptr;

    char * sdup = strdup(boost::trim_copy(s).c_str());

    char * keyPtr = strtok(sdup, "=");
    std::string key(keyPtr != nullptr ? keyPtr : "");

    boost::trim(key);

    if(!key.empty()) {
        char * valuePtr = strtok(nullptr, "=");
        std::string value(valuePtr != nullptr? valuePtr : "");

        res = new TConfigValue;
        res->m_key = key;
        res->m_value = boost::trim_copy(value);
    }

    free(sdup);// could we use delete sdup here?

    return res;
}

bool CConfig::getValue(const std::string &section, const std::string& key, bool &value, bool defaultValue) const
{
    std::string valueTemp;
    std::string dafaultValueStr = defaultValue ? "true" : "false";
    bool ret = getValue(section, key, valueTemp, dafaultValueStr, {"true", "1", "false", "0"});
    if(ret) {
        if(isSameNoCase(valueTemp, std::string("true")) || valueTemp == "1")
            value = true;
        else if(isSameNoCase(valueTemp, std::string("false")) || valueTemp == "0")
            value = false;
    }

    return ret;
}

bool CConfig::getValue(const std::string &section, const std::string& key, int &value, int min, int max, int defaultValue) const
{
    TConfigValue * val = lookupValue(section, key);
    if(val == nullptr || val->m_value.empty()) {
        value = defaultValue;
        return true;
    }

    if(!isDecimalInteger(val->m_value)){
        CLog::logError("Configuration error: %s.%s is not a valid number (%s)", section.c_str(), key.c_str(), val->m_value.c_str());
        return false;
    }

    int tmpValue = std::strtol(val->m_value.c_str(), nullptr, 10);

    if(tmpValue < min || tmpValue > max) {
        CLog::logError("Configuration error: %s.%s is out of range must be between %d and %d, actual %d", section.c_str(), key.c_str(), min, max, tmpValue);
        return false;
    }

    value = tmpValue;
    return true;
}

bool CConfig::getValue(const std::string &section, const std::string& key, double &value, double min, double max, double defaultValue) const
{
    TConfigValue * val = lookupValue(section, key);
    if(val == nullptr || val->m_value.empty()) {
        value = defaultValue;
        return true;
    }

    if(!isFloatingPoint(val->m_value)){
        CLog::logError("Configuration error: %s.%s is not a valid floating point number (%s)", section.c_str(), key.c_str(), val->m_value.c_str());
        return false;
    }

    double tmpValue = std::strtold(val->m_value.c_str(), nullptr);

    if(tmpValue < min || tmpValue > max) {
        CLog::logError("Configuration error: %s.%s is out of range must be between %f and %f, actual %f", section.c_str(), key.c_str(), min, max, tmpValue);
        return false;
    }

    value = tmpValue;
    return true;
}

bool CConfig::getValue(const std::string &section, const std::string& key, unsigned int &value, unsigned int min, unsigned int max, int defaultValue) const
{
    TConfigValue * val = lookupValue(section, key);
    if(val == nullptr || val->m_value.empty()) {
        value = defaultValue;
        return true;
    }

    if(!isDecimalInteger(val->m_value)){
        CLog::logError("Configuration error: %s.%s is not a valid number (%s)", section.c_str(), key.c_str(), val->m_value.c_str());
        return false;
    }

    unsigned int tmpValue = std::strtol(val->m_value.c_str(), nullptr, 10);

    if(tmpValue < min || tmpValue > max) {
        CLog::logError("Configuration error: %s.%s is out of range must be between %u and %u, actual %u", section.c_str(), key.c_str(), min, max, tmpValue);
        return false;
    }

    value = tmpValue;
    return true;
}

bool CConfig::getValue(const std::string &section, const std::string& key, std::string &value, unsigned int minLen, unsigned int maxLen, const std::string defaultValue) const
{
    TConfigValue * val = lookupValue(section, key);
    std::string valueTemp = val !=  nullptr ? val->m_value : defaultValue;

    if(valueTemp.empty() && minLen == 0U)
        valueTemp = defaultValue;

    if(valueTemp.length() < minLen || valueTemp.length() > maxLen) {
        CLog::logError("Configuration error: %s.%s has an invalid length, length must be between %u and %u, actual %u", section.c_str(), key.c_str(), minLen, maxLen, valueTemp.length());
        return false;
    }

    value = valueTemp;
    return true;
}

bool CConfig::getValue(const std::string &section, const std::string& key, unsigned char &value, unsigned char min, unsigned char max,unsigned char defaultValue) const
{
    unsigned int tempValue;
    bool ret = getValue(section, key, tempValue, (unsigned int)min, (unsigned int)max, (unsigned int) defaultValue);
    if(ret) {
        value = (unsigned char)tempValue;
    }

    return ret;
}

bool CConfig::getValue(const std::string &section, const std::string& key, std::string &value, const std::string defaultValue, const std::vector<std::string>& allowedValues) const
{
    std::string valueTemp;
    if(getValue(section, key, valueTemp, 0U, 2048, defaultValue)) {
        for(auto s : allowedValues) {
            if(isSameNoCase(s, valueTemp)) {
                value = CUtils::ToLower(valueTemp);
                return true;
            }
        }
    }

    // Build error message
    std::stringstream allowedValuesStreamStr;
    for(auto s : allowedValues) {
        allowedValuesStreamStr << s << ", ";
    }

    // We want nice looking error message, so get rid of trailing ", "
    std::string allowedValuesStr = allowedValuesStreamStr.str();
    allowedValuesStr.resize(allowedValuesStr.length() - 2);

    CLog::logError("Configuration error: %s.%s has an invalid value, valid values are: %s. Actual: %s", section.c_str(), key.c_str(), allowedValuesStr.c_str(), valueTemp.c_str());
    return false;
}

TConfigValue * CConfig::lookupValue(const std::string& sectionName, const std::string& key) const
{
    TConfigValue * res = nullptr;
    std::string sectionNameTemp = sectionName;

    // Make sure we have brackets around section names
    if(sectionNameTemp[0] != '[') sectionNameTemp = "[" + sectionNameTemp;
    if(sectionNameTemp[sectionNameTemp.length() - 1] != ']') sectionNameTemp.push_back(']');

    auto itSection = m_sections.find(sectionNameTemp);

    if(itSection != m_sections.end()) {
        auto itValue = itSection->second->getValues().find(key);
        if(itValue != itSection->second->getValues().end()) {
            res = itValue->second;
        }
    }

    return res;
}
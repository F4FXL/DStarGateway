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

#pragma once

#include <string>
#include <unordered_map>
#include <utility>
#include <cctype>
#include <cstring>
#include <cassert>
#include <vector>

#define isConfigSection(s)(!s.empty() && s[0] == '[' && s[s.length() - 1] == ']')
#define isConfigValue(s)(s.find('=') != std::string::npos && s.find('#') > s.find('='))
#define isSameNoCase(a,b)(strcasecmp(a.c_str(), b.c_str()) == 0)
#define isDecimalInteger(s)(!s.empty() && (s[0] == '-' || std::isdigit(s[0])) && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !(std::isdigit(c) || c == '-'); }) == s.end())
#define isFloatingPoint(s)(!s.empty() && (s[0] == '-' || std::isdigit(s[0])) && std::find_if(s.begin(), s.end(), [](unsigned char c) { return !(std::isdigit(c) || c == '-' || c == '.'); }) == s.end())

// https://www.programmingnotes.org/6504/c-how-to-make-map-unordered-map-keys-case-insensitive-using-c/
struct case_insensitive_unordered_map {
    struct comp {
        bool operator() (const std::string& lhs, const std::string& rhs) const {
            // On non Windows OS, use the function "strcasecmp" in #include <strings.h>
            return strcasecmp(lhs.c_str(), rhs.c_str()) == 0;
        }
    };
    struct hash {
        std::size_t operator() (std::string str) const {
            for (std::size_t index = 0; index < str.size(); ++index) {
                auto ch = static_cast<unsigned char>(str[index]);
                str[index] = static_cast<unsigned char>(std::tolower(ch));
            }
            return std::hash<std::string>{}(str);
        }
    };
};

typedef struct {
    std::string m_key;
    std::string m_value;
} TConfigValue;

typedef std::unordered_map<std::string, TConfigValue *, case_insensitive_unordered_map::hash, case_insensitive_unordered_map::comp> CConfigValuesMap;

class CConfigSection {
public:
    CConfigSection(const std::string name) :
    m_name(name)
    {
        assert(!name.empty());
    }

    ~CConfigSection()
    {
        for(auto it = m_values.begin();it != m_values.end();it++) {
            delete it->second;
        }
        m_values.clear();
    }

    std::string getName() const
    {
        return m_name;
    }
    CConfigValuesMap & getValues()
    {
        return m_values;
    }
private:
	std::string m_name;
    CConfigValuesMap m_values;
};

typedef std::unordered_map<std::string, CConfigSection *, case_insensitive_unordered_map::hash, case_insensitive_unordered_map::comp> CConfigSectionsMap;

class CConfig
{
public:
    CConfig(const std::string filename);
    ~CConfig();

    bool load();
    bool getValue(const std::string &section, const std::string& key, bool &value, bool defaultValue) const;
    bool getValue(const std::string &section, const std::string& key, int &value, int min, int max, int defaultValue) const;
    bool getValue(const std::string &section, const std::string& key, double &value, double min, double max, double defaultValue) const;
    bool getValue(const std::string &section, const std::string& key, unsigned int &value, unsigned int min, unsigned int max,int defaultValue) const;
    bool getValue(const std::string &section, const std::string& key, unsigned char &value, unsigned char min, unsigned char max,unsigned char defaultValue) const;
    bool getValue(const std::string &section, const std::string& key, std::string &value, unsigned int minLen, unsigned int maxLen, const std::string defaultValue) const;
    bool getValue(const std::string &section, const std::string& key, std::string &value, const std::string defaultValue, const std::vector<std::string>& allowedValues) const;

private:
    void stripComment(std::string& s) const;
    TConfigValue * readKeyAndValue(const std::string s) const;
    TConfigValue * lookupValue(const std::string& section, const std::string& key) const;

    std::string m_filename;
    CConfigSectionsMap m_sections;
};

/*
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

#pragma once

#include <memory>
#include <string>
#include <stdexcept>

#define string_right(s,l) (s.substr(s.length() - l, l))

class CStringUtils {
public:
    template<typename ... Args>
    static std::string string_format( const std::string& format, Args ... args )
    {
        std::string ret;
        string_format_in_place(ret, format, args...);
        return ret;
    }

    template<typename ... Args>
    static void string_format_in_place(std::string& output, const std::string& format, Args ... args )
    {
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }

        auto size = static_cast<size_t>( size_s );
        auto buf = std::make_unique<char[]>( size );
        std::snprintf( buf.get(), size, format.c_str(), args ... );

        output.reserve(size);
        output.assign(buf.get(), size - 1); // -1 because we do not need trailing '\0'
    }

    static size_t find_nth(const std::string& haystack, char needle, size_t nth);
};

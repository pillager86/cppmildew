/*
Copyright (C) 2021 pillager86.rf.gd

This program is free software: you can redistribute it and/or modify it under 
the terms of the GNU General Public License as published by the Free Software 
Foundation, either version 3 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with 
this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include "array.hpp"

namespace cppd
{
    using UTF8String = Array<char>;

    template<typename T>
    UTF8String ToUTF8String(const T& value)
    {
        if constexpr(std::is_same_v<decltype(nullptr), std::remove_cv_t<T>>)
        {
            return UTF8String("null");
        }
        else if constexpr(std::is_same_v<bool, std::remove_cv_t<T>>)
        {
            return UTF8String(value? "true" : "false");
        }
        else if constexpr(std::is_integral_v<T> || std::is_floating_point_v<T>)
        {
            std::ostringstream ss;
            ss << value;
            return UTF8String(ss.str());
        }
        else if constexpr(std::is_same_v<std::string, std::remove_cv_t<T>>)
        {
            return UTF8String(value);
        }
        else if constexpr(std::is_same_v<UTF8String, std::remove_cv_t<T>>)
        {
            return value;
        }
        return UTF8String("");
    }
}
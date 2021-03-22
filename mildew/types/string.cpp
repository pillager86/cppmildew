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

#include "string.hpp"

namespace mildew
{
    ScriptString::ScriptString()
    : ScriptObject("String", nullptr)
    {}

    ScriptString::ScriptString(const std::string& s)
    : ScriptObject("String", nullptr), str(s)
    {}

    size_t ScriptString::GetHash() const
    {
        constexpr size_t MOD = sizeof(size_t) * 8;
        size_t result = str.Length();
        for(size_t i = 0; i < str.Length(); ++i)
            result ^= str.At(i) << (i % MOD);
        return result;
    }

    bool ScriptString::operator<(const ScriptString& s) const 
    {
        return str < s.str;
    }

    bool ScriptString::operator==(const ScriptString& s) const
    {
        return str == s.str;
    }

    std::ostream& operator<<(std::ostream& os, const ScriptString& s)
    {
        os << s.str;
        return os;
    }
}
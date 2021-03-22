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
#include "array.hpp"

namespace mildew
{
    size_t ScriptArray::GetHash() const
    {
        // TODO use real hash function
        size_t result = array.Length();
        for(const auto& item : array)
            result ^= item.GetHash();
        return result;
    }

    bool ScriptArray::operator<(const ScriptArray& other)
    {
        return array < other.array;
    }

    bool ScriptArray::operator==(const ScriptArray& other)
    {
        return array == other.array;
    }

    std::ostream& operator<<(std::ostream& os, const ScriptArray& a)
    {
        os << '[';
        for(size_t i = 0; i < a.array.Length(); ++i)
        {
            os << a.array.At(i);
            if(i < a.array.Length() - 1)
                os << ", ";
        }
        os << ']';
        return os;
    }
}
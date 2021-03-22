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

#include <ostream>

#include "../../cppd/array.hpp"
#include "any.hpp"
#include "object.hpp"

namespace mildew
{
    class ScriptArray : public ScriptObject 
    {
    public:
        ScriptArray(std::initializer_list<ScriptAny> list)
        : ScriptObject("Array", nullptr), array(list) 
        {}

        size_t GetHash() const override;

        bool operator<(const ScriptArray& other);
        bool operator==(const ScriptArray& other);

        cppd::Array<ScriptAny> array;
    };

    std::ostream& operator<<(std::ostream& os, const ScriptArray& a);
}
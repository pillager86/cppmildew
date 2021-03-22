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

#include "../../cppd/utf8string.hpp"

#include <string>

#include "object.hpp"

namespace mildew
{
    class ScriptString : public ScriptObject
    {
    public:
        ScriptString();
        ScriptString(const std::string& s);

        size_t GetHash() const override;

        bool operator<(const ScriptString& s) const;
        bool operator==(const ScriptString& s) const;

        cppd::UTF8String str;
    };

    std::ostream& operator<<(std::ostream& os, const ScriptString& s);

}
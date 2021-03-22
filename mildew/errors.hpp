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

#include <sstream>
#include <stdexcept>
#include <string>

namespace mildew
{
    class ScriptCompileError : public std::logic_error
    {
    public:
        ScriptCompileError(const std::string& msg)
        : std::logic_error(msg) 
        {}
    };

    class UnimplementedError : public std::runtime_error
    {
    public:
        UnimplementedError(const std::string& feature)
        : std::runtime_error("This feature is unimplemented: " + feature)
        {}
    };

} // namespace mildew
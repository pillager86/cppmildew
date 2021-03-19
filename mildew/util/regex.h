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

#include <string>
#include <tuple>

namespace mildew 
{
    std::tuple<std::string, std::string> ExtractRegex(const std::string& slash_regex);
    bool IsValidRegex(const std::string& pattern, const std::string& flags);
}
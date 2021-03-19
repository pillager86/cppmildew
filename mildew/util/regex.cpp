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
#include "regex.h"

#include <regex>

namespace mildew
{

    std::tuple<std::string, std::string> ExtractRegex(const std::string& slash_regex)
    {
        if(slash_regex.length() < 2 || slash_regex[0] != '/')
            return std::tuple<std::string, std::string>("", "");
        const size_t kPatternStart = 1;
        size_t pattern_end = 0;
        for(size_t i = slash_regex.length(); i > 0; --i)
        {
            if(slash_regex[i-1] == '/')
            {
                pattern_end = i - 1;
                break;
            }
        }
        if(pattern_end == 0)
            return std::tuple<std::string, std::string>("", "");
        std::string pattern = slash_regex.substr(kPatternStart, pattern_end - kPatternStart);
        std::string flags = slash_regex.substr(pattern_end+1, slash_regex.length() - (pattern_end + 1));
        return std::tuple<std::string, std::string>(pattern, flags);
    }

    bool IsValidRegex(const std::string& pattern, const std::string& flags)
    {
        try 
        {
            auto reg = std::basic_regex(pattern);
            // TODO check flags
            return true;
        }
        catch(const std::regex_error&)
        {
            return false;
        }
    }
}
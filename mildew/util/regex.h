#pragma once

#include <regex>
#include <string>
#include <tuple>

namespace mildew 
{
    std::tuple<std::string, std::string> ExtractRegex(const std::string& slash_regex);
    bool IsValidRegex(const std::string& pattern, const std::string& flags);
}
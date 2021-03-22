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
#include <vector>

#include "types/any.hpp"

namespace mildew
{

    class Interpreter
    {
    public:
        Interpreter() {}
        Interpreter(const Interpreter& i) = delete;
        ~Interpreter() {}

        ScriptAny Evaluate(const std::string& code, const std::string& name = "<program>");
        bool HasErrors() const { return errors_.size() != 0; }

        Interpreter& operator=(const Interpreter& i) = delete;

        const std::vector<std::string>& errors() const { return errors_; }
    private:
        std::vector<std::string> errors_;
    };

} // namespace mildew
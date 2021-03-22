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
#include <iostream>
#include "mildew/interpreter.hpp"

/**
 * Implements a basic REPL that lists tokens of script input
 */
int main()
{
    std::string input;
    mildew::Interpreter interpreter;
    while(true)
    {
        std::cout << "mildew> ";
        std::getline(std::cin, input);
        if(input == "" || input == "#exit")
            break;
        while(input.length() > 0 && input[input.length() - 1] == '\\')
        {
            input.resize(input.length() - 1);
            std::string line = "";
            std::cout << ">>> ";
            std::getline(std::cin, line);
            input += '\n' + line;
        }
        interpreter.Evaluate(input, "<repl>");
        if(interpreter.HasErrors())
        {
            for(const auto& error : interpreter.errors())
                std::cerr << error << std::endl;
            continue;
        }
        std::cout << "Successful parse" << std::endl;
    }
    return 0;
}
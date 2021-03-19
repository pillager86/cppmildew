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
#include "mildew/lexer.h"

/**
 * Implements a basic REPL that lists tokens of script input
 */
int main()
{
    std::string input;
    while(true)
    {
        std::cout << "mildew> ";
        std::getline(std::cin, input);
        if(input == "" || input == "#exit")
            break;
        auto lexer = mildew::Lexer(input);
        auto tokens = lexer.Tokenize();
        if(lexer.HasErrors())
        {
            for(const auto& error : lexer.errors())
                std::cerr << "Lexer Error: " << error << std::endl;
            continue;
        }
        for(const auto& token : tokens)
            std::cout << token << " ";
        std::cout << std::endl;
    }
    return 0;
}
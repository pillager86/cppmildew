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
#include "interpreter.h"

#include <iostream>

#include "errors.h"
#include "lexer.h"
#include "parser.h"

namespace mildew
{
    ScriptAny Interpreter::Evaluate(const std::string& code, const std::string& name)
    {
        errors_.clear();
        auto lexer = Lexer(code);
        auto tokens = lexer.Tokenize();
        if(lexer.HasErrors())
        {
            errors_.emplace_back("Lexer Errors");
            errors_.insert(std::end(errors_), std::begin(lexer.errors()), std::end(lexer.errors()));
            return ScriptAny();
        }
        std::cout << "Tokens for program " << name << std::endl;
        for(const auto& token : tokens)
            std::cout << token << " ";
        std::cout << std::endl;
        auto parser = Parser(tokens);
        try 
        {
            auto expression = parser.ParseExpression();
            std::cout << "Parsed expression success" << std::endl;
            std::cout << expression->to_string() << std::endl;
        }
        catch(const ScriptCompileError& compile_error)
        {
            errors_.emplace_back(compile_error.what());
            return ScriptAny();
        }
        return ScriptAny();
    }

} // namespace mildew
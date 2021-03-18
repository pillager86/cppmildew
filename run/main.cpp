#include <iostream>
#include "mildew/lexer.h"

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
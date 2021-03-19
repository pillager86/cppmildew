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

#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace mildew
{
    struct Position final
    {
        int line=0, column=0;
        void Advance(const char ch);
    };

    std::ostream& operator<<(std::ostream& os, const Position& pos);

    struct Token final
    {
        enum class Type 
        {
            EOF_, KEYWORD, INTEGER, DOUBLE, STRING, IDENTIFIER, REGEX,
            NOT, AND, OR, GT, GE, LT, LE,
            EQUALS, NEQUALS, STRICT_EQUALS, STRICT_NEQUALS,

            ASSIGN, 
            POW_ASSIGN, STAR_ASSIGN, FSLASH_ASSIGN, PERCENT_ASSIGN,
            PLUS_ASSIGN, DASH_ASSIGN,
            BAND_ASSIGN, BXOR_ASSIGN, BOR_ASSIGN, BLS_ASSIGN, BRS_ASSIGN, BURS_ASSIGN,

            PLUS, DASH, STAR, FSLASH, PERCENT, POW, DOT, TDOT,
            INC, DEC, // ++ and --
            BIT_AND, BIT_XOR, BIT_OR, BIT_NOT, BIT_LSHIFT, BIT_RSHIFT, BIT_URSHIFT,
            LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET, 
            SEMICOLON, COMMA, LABEL, QUESTION, COLON, ARROW, 
            NULLC, // null coalesce
            
            INVALID
        };

        enum class LiteralFlag
        {
            NONE, BINARY, OCTAL, HEXADECIMAL, TEMPLATE_STRING
        };

        Token(const Token::Type t = Token::Type::EOF_, const Position& p = {0,0}, const std::string& txt = "", const LiteralFlag lflag = LiteralFlag::NONE)
        : type{t}, position{p}, text{txt}, literal_flag{lflag} {}

        bool IsAssignmentOperator() const;
        bool IsIdentifier(const std::string& id) const;
        bool IsKeyword(const std::string& keyword) const;
        std::string Symbol() const;

        static Token CreateFakeToken(const Type type, const std::string& text);
        static Token CreateInvalidToken(const Position& pos, const std::string& text = "");

        Type type = Type::EOF_;
        Position position = {0, 0};
        std::string text;
        LiteralFlag literal_flag = LiteralFlag::NONE;
    };

    std::ostream& operator<<(std::ostream& os, const Token::Type token_type);
    std::ostream& operator<<(std::ostream& os, const Token& token);

    struct Lexer final
    {
        Lexer(const std::string& text) : text_(text) {}
        
        bool HasErrors() { return errors_.size() != 0; }
        const std::vector<std::string>& errors() const { return errors_; }
        std::vector<Token> Tokenize();

        static const std::unordered_set<std::string> kKeywords;
        static const std::unordered_map<char, char> kEscapeChars;

    private:
        template <typename ...Args>
        void AddError(Args... args)
        {
            std::ostringstream ss;
            ((ss << std::forward<Args>(args)), ...);
            errors_.emplace_back(ss.str());
        }
        char AdvanceChar();
        bool CanMakeRegex(const std::vector<Token>& tokens) const;
        char CurrentChar() const;
        void HandleFSlash(std::vector<Token>& tokens);
        Token MakeAndToken();
        Token MakeDashToken();
        std::vector<Token> MakeDotTokens();
        Token MakeEqualToken();
        Token MakeIdKwOrLabel(const std::vector<Token>& tokens);
        Token MakeIntOrDoubleToken();
        Token MakeLAngleBracketToken();
        Token MakeNotToken();
        Token MakeOrToken();
        Token MakePercentToken();
        Token MakePlusToken();
        Token MakeQuestionToken();
        Token MakeRAngleBracketToken();
        Token MakeStarToken();
        Token MakeStringToken(std::vector<Token>& previous);
        Token MakeXorToken();
        bool Match(const char ch);
        char PeekChar() const;

        Position pos_ = {1, 1};
        std::string text_;
        size_t index_ = 0;
        std::vector<std::string> errors_;
    };
}
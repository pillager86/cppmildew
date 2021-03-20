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
#include "lexer.h"

#include <cctype>

#include "../cpp/utf.h"
#include "util/regex.h"

namespace mildew
{
    inline static bool IsAlpha(const char c)
    {
        return std::isalpha(static_cast<unsigned char>(c));
    }

    inline static bool IsAlphaNumeric(const char c)
    {
        return std::isalnum(static_cast<unsigned char>(c));
    }

    inline static bool IsDigit(const char c)
    {
        return std::isdigit(static_cast<unsigned char>(c));
    }

    inline static bool IsWhiteSpace(const char c)
    {
        return std::iswspace(static_cast<unsigned char>(c));
    }

    inline static char ToLower(const char c)
    {
        return std::tolower(static_cast<unsigned char>(c));
    }

    static bool IsValidDigit(const char c, const Token::LiteralFlag lflag)
    {
        switch(lflag)
        {
        case Token::LiteralFlag::NONE:
            return IsDigit(c) || c == '.' || c == 'e';
        case Token::LiteralFlag::HEXADECIMAL:
            return IsDigit(c) || (ToLower(c) >= 'a' && ToLower(c) <= 'f');
        case Token::LiteralFlag::OCTAL:
            return c >= '0' && c <= '7';
        case Token::LiteralFlag::BINARY:
            return c == '0' || c == '1';
        case Token::LiteralFlag::TEMPLATE_STRING:
            return true;
        }
        return false; // compiler warning
    }

    inline static bool ContinuesKWorID(const char c)
    {
        return IsAlphaNumeric(c) || c == '_' || c == '$';
    }

    inline static bool StartsKWorID(const char c)
    {
        return IsAlpha(c) || c == '_' || c == '$';
    }

    void Position::Advance(const char ch)
    {
        if(ch == '\0')
        {
            return;
        }
        else if(ch == '\n')
        {
            ++line;
            column = 1;
        }
        else
        {
            ++column;
        }
    }

    std::ostream& operator<<(std::ostream& os, const Position& pos)
    {
        os << "line " << pos.line << ", column " << pos.column;
        return os;
    }

    bool Token::IsAssignmentOperator() const
    {
        return (type == Type::ASSIGN || 
                type == Type::POW_ASSIGN ||
                type == Type::STAR_ASSIGN ||
                type == Type::FSLASH_ASSIGN ||
                type == Type::PERCENT_ASSIGN ||
                type == Type::PLUS_ASSIGN || 
                type == Type::DASH_ASSIGN ||
                type == Type::BAND_ASSIGN ||
                type == Type::BXOR_ASSIGN ||
                type == Type::BOR_ASSIGN ||
                type == Type::BLS_ASSIGN ||
                type == Type::BRS_ASSIGN ||
                type == Type::BURS_ASSIGN
        );
    }

    bool Token::IsIdentifier(const std::string& id) const
    {
        return type == Type::IDENTIFIER && text == id;
    }

    bool Token::IsKeyword(const std::string& keyword) const
    {
        return type == Type::KEYWORD && text == keyword;
    }

    std::string Token::Symbol() const
    {
        switch(type)
        {
        case Type::EOF_:
            return "\0";
        case Type::KEYWORD: case Type::INTEGER: case Type::DOUBLE: case Type::STRING: case Type::IDENTIFIER: case Type::REGEX:
            return text;
        case Type::NOT: return "!";
        case Type::AND: return "&&";
        case Type::OR: return "||";
        case Type::GT: return ">";
        case Type::GE: return ">=";
        case Type::LT: return "<";
        case Type::LE: return "<=";
        case Type::EQUALS: return "==";
        case Type::NEQUALS: return "!=";
        case Type::STRICT_EQUALS: return "===";
        case Type::STRICT_NEQUALS: return "!==";
        case Type::ASSIGN: return "=";
        case Type::POW_ASSIGN: return "**=";
        case Type::STAR_ASSIGN: return "*=";
        case Type::FSLASH_ASSIGN: return "/=";
        case Type::PERCENT_ASSIGN: return "%=";
        case Type::PLUS_ASSIGN: return "+=";
        case Type::DASH_ASSIGN: return "-=";
        case Type::BAND_ASSIGN: return "&=";
        case Type::BXOR_ASSIGN: return "^=";
        case Type::BOR_ASSIGN: return "|=";
        case Type::BLS_ASSIGN: return "<<=";
        case Type::BRS_ASSIGN: return ">>=";
        case Type::BURS_ASSIGN: return ">>>=";
        case Type::PLUS: return "+";
        case Type::DASH: return "-";
        case Type::STAR: return "*";
        case Type::FSLASH: return "/";
        case Type::PERCENT: return "%";
        case Type::POW: return "**";
        case Type::DOT: return ".";
        case Type::TDOT: return "...";
        case Type::INC: return "++";
        case Type::DEC: return "--"; 
        case Type::BIT_AND: return "&";
        case Type::BIT_XOR: return "^";
        case Type::BIT_OR: return "|";
        case Type::BIT_NOT: return "~";
        case Type::BIT_LSHIFT: return "<<";
        case Type::BIT_RSHIFT: return ">>";
        case Type::BIT_URSHIFT: return ">>>";
        case Type::LPAREN: return "(";
        case Type::RPAREN: return ")";
        case Type::LBRACE: return "{";
        case Type::RBRACE: return "}";
        case Type::LBRACKET: return "[";
        case Type::RBRACKET: return "]";
        case Type::SEMICOLON: return ";";
        case Type::COMMA: return ",";
        case Type::LABEL: return text + ":";
        case Type::QUESTION: return "?";
        case Type::COLON: return ":";
        case Type::ARROW: return "=>";
        case Type::NULLC: return "??";
        case Type::INVALID: return "#";
        }
        return "<invalid symbol>";
    }

    Token Token::CreateFakeToken(const Type type, const std::string& text)
    {
        return Token{type, {0, 0}, text, LiteralFlag::NONE};
    }

    Token Token::CreateInvalidToken(const Position& pos, const std::string& text)
    {
        return Token{Type::INVALID, pos, text, LiteralFlag::NONE};
    }

    std::ostream& operator<<(std::ostream& os, const Token::Type token_type)
    {
        switch(token_type)
        {
        case Token::Type::EOF_: os << "EOF"; break;
        case Token::Type::KEYWORD: os << "KEYWORD"; break;
        case Token::Type::INTEGER: os << "INTEGER"; break;
        case Token::Type::DOUBLE:  os << "DOUBLE"; break;
        case Token::Type::STRING: os << "STRING"; break;
        case Token::Type::IDENTIFIER: os << "IDENTIFIER"; break;
        case Token::Type::REGEX: os << "REGEX"; break;
        case Token::Type::NOT: os << "NOT"; break;
        case Token::Type::AND: os << "AND"; break;
        case Token::Type::OR: os << "OR"; break;
        case Token::Type::GT: os << "GT"; break;
        case Token::Type::GE: os << "GE"; break;
        case Token::Type::LT: os << "LT"; break;
        case Token::Type::LE: os << "LE"; break;
        case Token::Type::EQUALS: os << "EQUALS"; break;
        case Token::Type::NEQUALS: os << "NEQUALS"; break;
        case Token::Type::STRICT_EQUALS: os << "STRICT_EQUALS"; break;
        case Token::Type::STRICT_NEQUALS: os << "STRICT_NEQUALS"; break;
        case Token::Type::ASSIGN: os << "ASSIGN"; break;
        case Token::Type::POW_ASSIGN: os << "POW_ASSIGN"; break;
        case Token::Type::STAR_ASSIGN: os << "STAR_ASSIGN"; break;
        case Token::Type::FSLASH_ASSIGN: os << "FSLASH_ASSIGN"; break;
        case Token::Type::PERCENT_ASSIGN: os << "PERCENT_ASSIGN"; break;
        case Token::Type::PLUS_ASSIGN: os << "PLUS_ASSIGN"; break;
        case Token::Type::DASH_ASSIGN: os << "DASH_ASSIGN"; break;
        case Token::Type::BAND_ASSIGN: os << "BAND_ASSIGN"; break;
        case Token::Type::BXOR_ASSIGN: os << "BXOR_ASSIGN"; break;
        case Token::Type::BOR_ASSIGN: os << "BOX_ASSIGN"; break;
        case Token::Type::BLS_ASSIGN: os << "BLS_ASSIGN"; break;
        case Token::Type::BRS_ASSIGN: os << "BRS_ASSIGN"; break;
        case Token::Type::BURS_ASSIGN: os << "BURS_ASSIGN"; break;
        case Token::Type::PLUS: os << "PLUS"; break;
        case Token::Type::DASH: os << "DASH"; break;
        case Token::Type::STAR: os << "STAR"; break;
        case Token::Type::FSLASH: os << "FSLASH"; break;
        case Token::Type::PERCENT: os << "PERCENT"; break;
        case Token::Type::POW: os << "POW"; break;
        case Token::Type::DOT: os << "DOT"; break;
        case Token::Type::TDOT: os << "TDOT"; break;
        case Token::Type::INC: os << "INC"; break;
        case Token::Type::DEC: os << "DEC"; break; 
        case Token::Type::BIT_AND: os << "BIT_AND"; break;
        case Token::Type::BIT_XOR: os << "BIT_XOR"; break;
        case Token::Type::BIT_OR: os << "BIT_OR"; break;
        case Token::Type::BIT_NOT: os << "BIT_NOT"; break;
        case Token::Type::BIT_LSHIFT: os << "BIT_LSHIFT"; break;
        case Token::Type::BIT_RSHIFT: os << "BIT_RSHIFT"; break;
        case Token::Type::BIT_URSHIFT: os << "BIT_URSHIFT"; break;
        case Token::Type::LPAREN: os << "LPAREN"; break;
        case Token::Type::RPAREN: os << "RPAREN"; break;
        case Token::Type::LBRACE: os << "LBRACE"; break;
        case Token::Type::RBRACE: os << "RBRACE"; break;
        case Token::Type::LBRACKET: os << "LBRACKET"; break;
        case Token::Type::RBRACKET: os << "RBRACKET"; break;
        case Token::Type::SEMICOLON: os << "SEMICOLON"; break;
        case Token::Type::COMMA: os << "COMMA"; break;
        case Token::Type::LABEL: os << "LABEL"; break;
        case Token::Type::QUESTION: os << "QUESTION"; break;
        case Token::Type::COLON: os << "COLON"; break;
        case Token::Type::ARROW: os << "ARROW"; break;
        case Token::Type::NULLC: os << "NULLC"; break;
        case Token::Type::INVALID: os << "INVALID"; break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        os << "[" << token.type;
        if(token.text.size() != 0)
            os << "|" << token.text;
        os << "]";
        return os;
    }

    const std::unordered_set<std::string> Lexer::kKeywords = 
    {
        "true", "false", "undefined", "null",
        "var", "let", "const", 
        "if", "else", "while", "do", "for", "in",
        "switch", "case", "default",
        "break", "continue", "return", 
        "function", "class", "super", "extends",
        "new", "delete", "typeof", "instanceof",
        "throw", "try", "catch", "finally", 
        "yield"
    };

    const std::unordered_map<char, char> Lexer::kEscapeChars = 
    {
        {'b', '\b'}, {'f', '\f'}, {'n', '\n'}, {'r', '\r'}, {'t', '\t'}, {'v', '\v'}, 
        {'0', '\0'}, {'\'', '\''}, {'"', '"'}, {'\\', '\\'}
    };

    std::vector<Token> Lexer::Tokenize()
    {
        std::vector<Token> tokens;
        if(text_ == "")
            return tokens;
        while(index_ < text_.length())
        {
            while(IsWhiteSpace(CurrentChar()))
                AdvanceChar();
            char c = CurrentChar();
            switch(c)
            {
            case '\'': case '"': case '`': tokens.emplace_back(MakeStringToken(tokens)); break;
            case '>': tokens.emplace_back(MakeRAngleBracketToken()); break;
            case '<': tokens.emplace_back(MakeLAngleBracketToken()); break;
            case '=': tokens.emplace_back(MakeEqualToken()); break;
            case '!': tokens.emplace_back(MakeNotToken()); break;
            case '&': tokens.emplace_back(MakeAndToken()); break;
            case '|': tokens.emplace_back(MakeOrToken()); break;
            case '+': tokens.emplace_back(MakePlusToken()); break;
            case '-': tokens.emplace_back(MakeDashToken()); break;
            case '*': tokens.emplace_back(MakeStarToken()); break;
            case '/': HandleFSlash(tokens); break;
            case '%': tokens.emplace_back(MakePercentToken()); break;
            case '^': tokens.emplace_back(MakeXorToken()); break;
            case '~': tokens.emplace_back(Token(Token::Type::BIT_NOT, pos_)); break;
            case '(': tokens.emplace_back(Token(Token::Type::LPAREN, pos_)); break;
            case ')': tokens.emplace_back(Token(Token::Type::RPAREN, pos_)); break;
            case '{': tokens.emplace_back(Token(Token::Type::LBRACE, pos_)); break;
            case '}': tokens.emplace_back(Token(Token::Type::RBRACE, pos_)); break;
            case '[': tokens.emplace_back(Token(Token::Type::LBRACKET, pos_)); break;
            case ']': tokens.emplace_back(Token(Token::Type::RBRACKET, pos_)); break;
            case ';': tokens.emplace_back(Token(Token::Type::SEMICOLON, pos_)); break;
            case ',': tokens.emplace_back(Token(Token::Type::COMMA, pos_)); break;
            case '.': {
                auto dots = MakeDotTokens();
                tokens.insert(std::end(tokens), std::begin(dots), std::end(dots));
                break;
            }
            case ':': tokens.emplace_back(Token(Token::Type::COLON, pos_)); break;
            case '?': tokens.emplace_back(MakeQuestionToken()); break;
            case '\0': tokens.emplace_back(Token(Token::Type::EOF_, pos_)); break;
            default:
                if(StartsKWorID(c))
                    tokens.emplace_back(MakeIdKwOrLabel(tokens));
                else if(IsDigit(c))
                    tokens.emplace_back(MakeIntOrDoubleToken());
                else 
                    AddError("Invalid character ", c, " at ", pos_);
            }
            AdvanceChar();
        }
        tokens.emplace_back(Token(Token::Type::EOF_));
        return tokens;
    }

    char Lexer::AdvanceChar()
    {
        auto ret = CurrentChar();
        ++index_;
        pos_.Advance(CurrentChar());
        return ret;
    }

    bool Lexer::CanMakeRegex(const std::vector<Token>& tokens) const
    {
        if(tokens.size() == 0)
            return true;
        switch((tokens.end() - 1)->type)
        {
        case Token::Type::IDENTIFIER:
        case Token::Type::INTEGER:
        case Token::Type::DOUBLE:
        case Token::Type::STRING:
        case Token::Type::RBRACKET:
        case Token::Type::RPAREN:
        case Token::Type::INC:
        case Token::Type::DEC:
            return false;
        case Token::Type::KEYWORD: {
            const std::string& text = (tokens.end() - 1)->text;
            if(text == "null" || text == "true" || text == "false")
                return false;
            return true;
        }
        default:
            return true;
        }
    }

    char Lexer::CurrentChar() const
    {
        if(index_ < text_.length())
            return text_[index_];
        return '\0';
    }

    void Lexer::HandleFSlash(std::vector<Token>& tokens)
    {
        if(Match('*')) // block comment
        {
            while(PeekChar() != '\0')
            {
                if(Match('*'))
                {
                    if(PeekChar() == '/')
                        break;
                }
                AdvanceChar();
            }
            AdvanceChar();
        }
        else if(Match('/')) // single line comment
        {
            while(PeekChar() != '\n' && PeekChar() != '\0')
            {
                AdvanceChar();
            }
        }
        else if(CanMakeRegex(tokens))
        {
            std::string accum = "";
            auto start_pos = pos_;
            accum += CurrentChar();
            bool getting_flags = false;
            while(CurrentChar())
            {
                if(!getting_flags)
                {
                    if(CurrentChar() == '\\')
                    {
                        accum += AdvanceChar();
                        if(CurrentChar())
                        {
                            accum += AdvanceChar();
                        }
                    }
                    else if(CurrentChar() == '/')
                    {
                        accum += AdvanceChar();
                        getting_flags = false;
                    }
                    else 
                    {
                        accum += AdvanceChar();
                    }
                }
                else 
                {
                    if(IsAlpha(CurrentChar()))
                        break;
                    accum += AdvanceChar();
                }
            }
            --index_;
            auto extracted = ExtractRegex(accum);
            auto pattern = std::get<0>(extracted);
            auto flags = std::get<1>(extracted);
            if((pattern == "" && flags == "")
             || !IsValidRegex(pattern, flags))
            {
                AddError("Malformed/invalid regex literal at position ",
                    start_pos);
                return;
            }
            tokens.emplace_back(Token(Token::Type::REGEX, start_pos, accum));
        }
        else if(PeekChar() == '=')
        {
            const auto start_pos = pos_;
            AdvanceChar();
            tokens.emplace_back(Token(Token::Type::FSLASH_ASSIGN, start_pos));
        }
        else 
        {
            tokens.emplace_back(Token(Token::Type::FSLASH, pos_));
        }
    }

    Token Lexer::MakeAndToken()
    {
        auto start_pos = pos_;
        if(Match('&'))
            return Token(Token::Type::AND, start_pos);
        else if(Match('='))
            return Token(Token::Type::BAND_ASSIGN, start_pos);
        else 
            return Token(Token::Type::BIT_AND, start_pos);
    }

    Token Lexer::MakeDashToken()
    {
        auto start_pos = pos_;
        if(Match('-'))
            return Token(Token::Type::DEC, start_pos);
        else if(Match('='))
            return Token(Token::Type::DASH_ASSIGN, start_pos);
        else 
            return Token(Token::Type::DASH, start_pos);
    }

    std::vector<Token> Lexer::MakeDotTokens()
    {
        auto start_pos = pos_;
        if(Match('.'))
        {
            if(Match('.'))
                return std::vector<Token>({Token(Token::Type::TDOT, start_pos)});
            else 
                return std::vector<Token>({Token(Token::Type::DOT, start_pos),
                    Token(Token::Type::DOT, pos_)});
        }
        else 
        {
            return std::vector<Token>({Token(Token::Type::DOT, pos_)});
        }
    }

    Token Lexer::MakeEqualToken()
    {
        auto start_pos = pos_;
        if(Match('='))
        {
            if(Match('='))
                return Token(Token::Type::STRICT_EQUALS, start_pos);
            else 
                return Token(Token::Type::EQUALS, start_pos);
        }
        else if(Match('>'))
        {
            return Token(Token::Type::ARROW, start_pos);
        }
        else 
        {
            return Token(Token::Type::ASSIGN, start_pos);
        }
    }

    Token Lexer::MakeIdKwOrLabel(const std::vector<Token>& tokens)
    {
        const auto start = index_;
        auto start_pos = pos_;
        AdvanceChar();
        while(ContinuesKWorID(CurrentChar()))
            AdvanceChar();
        auto text = text_.substr(start, index_ - start);
        --index_;
        if(text == "return" || text == "throw" || text == "delete"
          || text == "catch" || text == "finally")
        {
            if(tokens.size() > 0 && (tokens.end() -1)->type == Token::Type::DOT)
                return Token(Token::Type::IDENTIFIER, start_pos, text);
        }

        if(kKeywords.count(text) > 0)
        {
            return Token(Token::Type::KEYWORD, start_pos, text);
        }
        else if(Match(':'))
        {
            return Token(Token::Type::LABEL, start_pos, text);
        }
        else 
        {
            return Token(Token::Type::IDENTIFIER, start_pos, text);
        }
    }

    Token Lexer::MakeIntOrDoubleToken()
    {
        const auto start = index_;
        auto start_pos = pos_;
        int dot_counter = 0;
        int e_counter = 0;
        Token::LiteralFlag lflag = Token::LiteralFlag::NONE;
        if(Match('x'))
            lflag = Token::LiteralFlag::HEXADECIMAL;
        else if(Match('o'))
            lflag = Token::LiteralFlag::OCTAL;
        else if(Match('b'))
            lflag = Token::LiteralFlag::BINARY;

        if(lflag != Token::LiteralFlag::NONE && text_[start] != '0')
        {
            AddError("Malformed integer literal at ", start_pos);
            return Token::CreateInvalidToken(start_pos, "");
        }

        while(IsValidDigit(PeekChar(), lflag))
        {
            AdvanceChar();
            if(lflag == Token::LiteralFlag::NONE)
            {
                if(CurrentChar() == '.')
                {
                    if(++dot_counter > 1)
                    {
                        AddError("Too many decimals in integer literal at ", start_pos);
                        return Token::CreateInvalidToken(start_pos, "");
                    }
                }
                else if(ToLower(CurrentChar()) == 'e')
                {
                    if(++e_counter > 1)
                    {
                        AddError("Numbers may only have one exponent specifier at ", start_pos);
                        return Token::CreateInvalidToken(start_pos, "");
                    }
                    if(PeekChar() == '+' || PeekChar() == '-')
                        AdvanceChar();
                    if(!IsDigit(PeekChar()))
                    {
                        AddError("Exponent specifier must be followed by number at ", start_pos);
                    }
                }
            }
        }
        auto text = text_.substr(start, index_ + 1 - start);
        if(lflag != Token::LiteralFlag::NONE && text.length() <= 2)
        {
            AddError("Malformed hex/octal/binary integer at ", start_pos);
            return Token::CreateInvalidToken(start_pos, "");
        }
        if(dot_counter == 0 && e_counter == 0)
            return Token(Token::Type::INTEGER, start_pos, text, lflag);
        return Token(Token::Type::DOUBLE, start_pos, text);
    }

    Token Lexer::MakeLAngleBracketToken()
    {
        auto start_pos = pos_;
        if(Match('='))
        {
            return Token(Token::Type::LE, start_pos);
        }
        else if(Match('<'))
        {
            if(Match('='))
                return Token(Token::Type::BLS_ASSIGN, start_pos);
            else 
                return Token(Token::Type::BIT_LSHIFT, start_pos);
        }
        else 
        {
            return Token(Token::Type::LT, start_pos);
        }
    }

    Token Lexer::MakeNotToken()
    {
        auto start_pos = pos_;
        if(Match('='))
        {
            if(Match('='))
                return Token(Token::Type::STRICT_NEQUALS, start_pos);
            else 
                return Token(Token::Type::NEQUALS, start_pos);
        }
        else 
        {
            return Token(Token::Type::NOT, start_pos);
        }
    }

    Token Lexer::MakeOrToken()
    {
        auto start_pos = pos_;
        if(Match('|'))
            return Token(Token::Type::OR, start_pos);
        else if(Match('='))
            return Token(Token::Type::BOR_ASSIGN, start_pos);
        else 
            return Token(Token::Type::BIT_OR, start_pos);
    }

    Token Lexer::MakePercentToken()
    {
        auto start_pos = pos_;
        if(Match('='))
            return Token(Token::Type::PERCENT_ASSIGN, start_pos);
        else 
            return Token(Token::Type::PERCENT, pos_);
    }

    Token Lexer::MakePlusToken()
    {
        auto start_pos = pos_;
        if(Match('+'))
            return Token(Token::Type::INC, start_pos);
        else if(Match('='))
            return Token(Token::Type::PLUS_ASSIGN, start_pos);
        else 
            return Token(Token::Type::PLUS, pos_);
    }

    Token Lexer::MakeQuestionToken()
    {
        auto start_pos = pos_;
        if(Match('?'))
            return Token(Token::Type::NULLC, start_pos);
        else 
            return Token(Token::Type::QUESTION, pos_);
    }

    Token Lexer::MakeRAngleBracketToken()
    {
        auto start_pos = pos_;
        if(Match('='))
        {
            return Token(Token::Type::GE, start_pos);
        }
        else if(Match('>'))
        {
            if(Match('>'))
            {
                if(Match('='))
                    return Token(Token::Type::BURS_ASSIGN, start_pos);
                else 
                    return Token(Token::Type::BIT_URSHIFT, start_pos);
            }
            else if(Match('='))
            {
                return Token(Token::Type::BRS_ASSIGN, start_pos);
            }
            else 
            {
                return Token(Token::Type::BIT_RSHIFT, start_pos);
            }
        }
        else 
        {
            return Token(Token::Type::GT, pos_);
        }
    }

    Token Lexer::MakeStarToken()
    {
        auto start_pos = pos_;
        if(Match('*'))
        {
            if(Match('='))
                return Token(Token::Type::POW_ASSIGN, start_pos);
            else 
                return Token(Token::Type::POW, start_pos);
        }
        else if(Match('='))
        {
            return Token(Token::Type::STAR_ASSIGN, start_pos);
        }
        else 
        {
            return Token(Token::Type::STAR, pos_);
        }
    }

    Token Lexer::MakeStringToken(std::vector<Token>& previous)
    {
        const char kCloseQuote = CurrentChar();
        auto start_pos = pos_;
        AdvanceChar();
        std::string text = "";
        bool escape_chars = true;
        if(previous.size() >= 3)
        {
            if((previous.end()-1)->IsIdentifier("raw") &&
               (previous.end()-2)->type == Token::Type::DOT &&
               (previous.end()-3)->IsIdentifier("String"))
            {
                escape_chars = false;
                previous.resize(previous.size() - 3);
            }
        }
        const Token::LiteralFlag kLflag = kCloseQuote == '`' ? Token::LiteralFlag::TEMPLATE_STRING : Token::LiteralFlag::NONE;
        while(CurrentChar() != kCloseQuote)
        {
            if(CurrentChar() == '\0')
            {
                AddError("Missing close quote at ", pos_);
                return Token::CreateInvalidToken(pos_, text);
            }
            else if(CurrentChar() == '\n' && kLflag != Token::LiteralFlag::TEMPLATE_STRING)
            {
                AddError("Line breaks inside regular string literals are not allowed at", pos_);
                return Token::CreateInvalidToken(pos_, text);
            }
            else if(CurrentChar() == '\\' && escape_chars)
            {
                AdvanceChar();
                if(kEscapeChars.count(CurrentChar()) > 0)
                {
                    text += kEscapeChars.at(CurrentChar());
                }
                else if(CurrentChar() == 'u')
                {
                    AdvanceChar();
                    std::string accum = "";
                    bool using_braces = false;
                    int limit_counter = 0;
                    static const int kLimit = 4;
                    if(CurrentChar() == '{')
                    {
                        AdvanceChar();
                        using_braces = true;
                    }
                    while(IsValidDigit(CurrentChar(), Token::LiteralFlag::HEXADECIMAL))
                    {
                        if(limit_counter >= kLimit && !using_braces)
                            break;
                        accum += CurrentChar();
                        AdvanceChar();
                        if(!using_braces)
                            ++limit_counter;
                    }
                    if(CurrentChar() == '}' && using_braces)
                        AdvanceChar();
                    --index_;
                    try 
                    {
                        text += cpp::EncodeChar32(std::stoi(accum, nullptr, 16));
                    }
                    catch(const std::exception&)
                    {
                        AddError("Invalid UTF32 at ", pos_);
                        return Token::CreateInvalidToken(pos_, accum);
                    }
                }
                else if(CurrentChar() == 'x')
                {
                    AdvanceChar();
                    std::string accum = "";
                    accum += CurrentChar();
                    AdvanceChar();
                    accum += CurrentChar();
                    try 
                    {
                        const char result = std::stoi(accum, nullptr, 16);
                        text += result;
                    }
                    catch(const std::exception&)
                    {
                        AddError("Invalid hexadecimal number at ", pos_);
                        return Token::CreateInvalidToken(pos_, accum);
                    }
                }
                else 
                {
                    AddError("Unknown escape character ", CurrentChar(), " at ", pos_);
                    return Token::CreateInvalidToken(start_pos, text);
                }
            }
            else 
            {
                text += CurrentChar();
            }
            AdvanceChar();
        }
        return Token(Token::Type::STRING, start_pos, text, kLflag);
    }

    Token Lexer::MakeXorToken()
    {
        auto start_pos = pos_;
        if(Match('='))
            return Token(Token::Type::BXOR_ASSIGN, start_pos);
        else
            return Token(Token::Type::BIT_XOR, pos_);
    }

    bool Lexer::Match(const char ch)
    {
        if(PeekChar() == ch)
        {
            AdvanceChar();
            return true;
        }
        return false;
    }

    char Lexer::PeekChar() const
    {
        if(index_ + 1 < text_.length())
            return text_[index_ + 1];
        return '\0';
    }
}
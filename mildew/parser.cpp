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
#include "parser.hpp"

#include <algorithm>
#include <unordered_map>

#include "errors.hpp"
#include "lexer.hpp"
#include "util/sfmt.hpp"

namespace mildew
{
    static int UnaryOpPrecedence(const Token& op_token, bool is_post = false)
    {
        if(op_token.IsKeyword("typeof") && !is_post)
        {
            return 17;
        }

        switch(op_token.type)
        {
        case Token::Type::BIT_NOT:
        case Token::Type::NOT:
        case Token::Type::PLUS:
        case Token::Type::DASH:
            return is_post ? 0 : 17;
        case Token::Type::INC:
        case Token::Type::DEC:
            return is_post ? 18 : 17;
        default: return 0;
        }
    }

    static int BinaryOpPrecedence(const Token& op_token)
    {
        if(op_token.IsKeyword("instanceof"))
            return 12;
        switch(op_token.type)
        {
        case Token::Type::LBRACKET:
        case Token::Type::DOT:
        case Token::Type::LPAREN:
            return 20;
        case Token::Type::POW:
            return 16;
        case Token::Type::STAR:
        case Token::Type::FSLASH:
        case Token::Type::PERCENT:
            return 15;
        case Token::Type::PLUS:
        case Token::Type::DASH:
            return 14;
        case Token::Type::BIT_LSHIFT:
        case Token::Type::BIT_RSHIFT:
        case Token::Type::BIT_URSHIFT:
            return 13;
        case Token::Type::LT:
        case Token::Type::LE:
        case Token::Type::GT:
        case Token::Type::GE:
            return 12;
        case Token::Type::EQUALS:
        case Token::Type::NEQUALS:
        case Token::Type::STRICT_EQUALS:
        case Token::Type::STRICT_NEQUALS:
            return 11;
        case Token::Type::BIT_AND:
            return 10;
        case Token::Type::BIT_XOR:
            return 9;
        case Token::Type::BIT_OR:
            return 8;
        case Token::Type::AND:
            return 7;
        case Token::Type::OR:
            return 6;
        case Token::Type::NULLC:
            return 5;
        case Token::Type::QUESTION:
            return 4;
        case Token::Type::ASSIGN:
        case Token::Type::POW_ASSIGN:
        case Token::Type::STAR_ASSIGN:
        case Token::Type::FSLASH_ASSIGN:
        case Token::Type::PERCENT_ASSIGN:
        case Token::Type::PLUS_ASSIGN:
        case Token::Type::DASH_ASSIGN:
        case Token::Type::BAND_ASSIGN:
        case Token::Type::BXOR_ASSIGN:
        case Token::Type::BOR_ASSIGN:
        case Token::Type::BLS_ASSIGN:
        case Token::Type::BRS_ASSIGN:
        case Token::Type::BURS_ASSIGN:
            return 3;
        default:
            return 0;
        }
    }

    static bool IsBinaryOpLeftAssoc(const Token& op_token)
    {
        if(op_token.IsKeyword("instanceof"))
            return true;
        switch(op_token.type)
        {
        case Token::Type::LBRACKET:
        case Token::Type::DOT:
        case Token::Type::LPAREN:
            return true;
        case Token::Type::POW:
            return false;
        case Token::Type::STAR:
        case Token::Type::FSLASH:
        case Token::Type::PERCENT:
            return true;
        case Token::Type::PLUS:
        case Token::Type::DASH:
            return true;
        case Token::Type::BIT_LSHIFT:
        case Token::Type::BIT_RSHIFT:
        case Token::Type::BIT_URSHIFT:
            return true;
        case Token::Type::LT:
        case Token::Type::LE:
        case Token::Type::GT:
        case Token::Type::GE:
            return true;
        case Token::Type::EQUALS:
        case Token::Type::NEQUALS:
        case Token::Type::STRICT_EQUALS:
        case Token::Type::STRICT_NEQUALS:
            return true;
        case Token::Type::BIT_AND:
            return true;
        case Token::Type::BIT_XOR:
            return true;
        case Token::Type::BIT_OR:
            return true;
        case Token::Type::AND:
            return true;
        case Token::Type::OR:
            return true;
        case Token::Type::NULLC:
            return true;
        case Token::Type::QUESTION:
            return false;
        case Token::Type::ASSIGN:
        case Token::Type::POW_ASSIGN:
        case Token::Type::STAR_ASSIGN:
        case Token::Type::FSLASH_ASSIGN:
        case Token::Type::PERCENT_ASSIGN:
        case Token::Type::PLUS_ASSIGN:
        case Token::Type::DASH_ASSIGN:
        case Token::Type::BAND_ASSIGN:
        case Token::Type::BXOR_ASSIGN:
        case Token::Type::BOR_ASSIGN:
        case Token::Type::BLS_ASSIGN:
        case Token::Type::BRS_ASSIGN:
        case Token::Type::BURS_ASSIGN:
            return false;
        default:
            return false;
        }
    }

    static bool TokenBeginsLoop(const Token& token)
    {
        return token.type == Token::Type::LABEL
            || token.IsKeyword("while")
            || token.IsKeyword("do")
            || token.IsKeyword("for");
    }

    std::shared_ptr<ExpressionNode> Parser::ParseExpression(int min_prec)
    {
        std::shared_ptr<ExpressionNode> primary_left = nullptr;
        CheckEOF("expression");
        const int un_op_prec = UnaryOpPrecedence(*current_token_);
        if(un_op_prec > min_prec)
        {
            const auto& op_token = *current_token_;
            NextToken();
            primary_left = ParsePrimaryExpression();
            primary_left = std::make_shared<UnaryOpNode>(op_token, primary_left);
        }
        else 
        {
            primary_left = ParsePrimaryExpression();
        }

        while(BinaryOpPrecedence(*current_token_) >= min_prec || 
          UnaryOpPrecedence(*current_token_, true) >= min_prec)
        {
            if(UnaryOpPrecedence(*current_token_, true) >= min_prec)
            {
                if(auto uop_node = std::dynamic_pointer_cast<UnaryOpNode>(primary_left))
                    primary_left = std::make_shared<UnaryOpNode>(uop_node->op_token, 
                        std::make_shared<UnaryOpNode>(*current_token_, uop_node->operand_node, true)
                    );
                else 
                    primary_left = std::make_shared<UnaryOpNode>(*current_token_, primary_left, true);
                NextToken();
            }
            else 
            {
                const auto& op_token = *current_token_;
                const auto prec = BinaryOpPrecedence(op_token);
                const auto is_left_assoc = IsBinaryOpLeftAssoc(op_token);
                const auto next_min_prec = is_left_assoc? (prec + 1) : prec;
                NextToken();
                if(op_token.type == Token::Type::QUESTION)
                {
                    auto on_true = ParseExpression();
                    Consume(Token::Type::COLON, "terniary expression");
                    auto on_false = ParseExpression();
                    primary_left = std::make_shared<TerniaryOpNode>(primary_left, on_true, on_false);
                }
                else if(op_token.type == Token::Type::DOT)
                {
                    auto right = ParsePrimaryExpression();
                    if(std::dynamic_pointer_cast<VarAccessNode>(right) == nullptr)
                        throw ScriptCompileError(MakeString("Right hand side of `.` operator must be identifier at ", 
                            current_token_->position));
                    if(un_op_prec != 0 && prec > un_op_prec)
                    {
                        auto uon = std::dynamic_pointer_cast<UnaryOpNode>(primary_left);
                        primary_left = std::make_shared<UnaryOpNode>(uon->op_token, 
                            std::make_shared<MemberAccessNode>(uon->operand_node, op_token, right));
                    }
                    else 
                    {
                        primary_left = std::make_shared<MemberAccessNode>(primary_left, op_token, right);
                    }
                }
                else if(op_token.type == Token::Type::LBRACKET)
                {
                    auto index = ParseExpression();
                    Consume(Token::Type::RBRACKET, "index expression");
                    if(un_op_prec != 0 && prec > un_op_prec)
                    {
                        auto uon = std::dynamic_pointer_cast<UnaryOpNode>(primary_left);
                        primary_left = std::make_shared<UnaryOpNode>(uon->op_token, 
                            std::make_shared<ArrayIndexNode>(uon->operand_node, index));
                    }
                    else 
                    {
                        primary_left = std::make_shared<ArrayIndexNode>(primary_left, index);
                    }
                }
                else if(op_token.type == Token::Type::LPAREN)
                {
                    auto params = ParseCommaSeparatedExpressions(Token::Type::RPAREN);
                    NextToken(); // consume )
                    if(un_op_prec != 0 && prec > un_op_prec)
                    {
                        auto uon = std::dynamic_pointer_cast<UnaryOpNode>(primary_left);
                        primary_left = std::make_shared<UnaryOpNode>(uon->op_token, 
                            std::make_shared<FunctionCallNode>(uon->operand_node, params));
                    }
                    else 
                    {
                        primary_left = std::make_shared<FunctionCallNode>(primary_left, params);
                    }
                }
                else 
                {
                    auto primary_right = ParseExpression(next_min_prec);
                    if(op_token.IsAssignmentOperator())
                    {
                        if(!(std::dynamic_pointer_cast<VarAccessNode>(primary_left) ||
                          std::dynamic_pointer_cast<MemberAccessNode>(primary_left) ||
                          std::dynamic_pointer_cast<ArrayIndexNode>(primary_left)))
                        {
                            throw ScriptCompileError(MakeString("Invalid left hand operand for assignment ",
                                primary_left->to_string(), " at ", op_token.position));
                        }
                    }
                    primary_left = std::make_shared<BinaryOpNode>(op_token, primary_left, primary_right);
                }
            }
        }
        return primary_left;
    }

    std::shared_ptr<BlockStatementNode> Parser::ParseProgram()
    {
        CheckEOF("parse program");
        const auto kLineNo = current_token_->position.line;
        function_context_stack_.push({FunctionContext::Type::NORMAL, 0, 0, {}});
        auto statements = ParseStatements(Token::Type::EOF_);
        function_context_stack_.pop();
        return std::make_shared<BlockStatementNode>(kLineNo, statements);
    }

    void Parser::CheckEOF(const std::string& where) const
    {
        if(current_token_ == nullptr)
        {
            if(where != "")
                throw ScriptCompileError("Unexpected EOF in " + where);
            else 
                throw ScriptCompileError("Unexpected EOF");
        }
    }

    void Parser::Consume(const Token::Type token_type, const std::string& where)
    {
        std::string end = (where == "") ? "" : (" in " + where);
        if(current_token_ == nullptr)
            throw ScriptCompileError(MakeString("Unexpected EOF, expected ", token_type, end));
        if(current_token_->type != token_type)
            throw ScriptCompileError(MakeString("Unexpected token ", current_token_->type, " expected ", 
                token_type, end, " at ", current_token_->position));
        NextToken();
    }

    void Parser::ConsumeText(const Token::Type token_type, const std::string& text, const std::string& where)
    {
        std::string end = (where == "") ? "" : (" in " + where);
        if(current_token_ == nullptr)
            throw ScriptCompileError(MakeString("Unexpected EOF, expected ", token_type, " ", text, end));
        if(current_token_->type != token_type || current_token_->text != text)
            throw ScriptCompileError(MakeString("Unexpected token ", *current_token_, " expected ", 
                token_type, " ", text, end, " at ", current_token_->position));
        NextToken();
    }

    ScriptAny Parser::EvaluateCTFE(const std::shared_ptr<ExpressionNode>& expr)
    {
        // for now just accept literals
        auto literal_node = std::dynamic_pointer_cast<LiteralNode>(expr);
        if(literal_node == nullptr)
            return ScriptAny();
        
        if(literal_node->literal_token.IsKeyword("true"))
            return ScriptAny(true);
        else if(literal_node->literal_token.IsKeyword("false"))
            return ScriptAny(false);
        else if(literal_node->literal_token.IsKeyword("null"))
            return ScriptAny(nullptr);
        else if(literal_node->literal_token.IsKeyword("undefined"))
            return ScriptAny();
        else if(literal_node->literal_token.type == Token::Type::DOUBLE)
            return ScriptAny(std::stod(literal_node->literal_token.text, nullptr));
        else if(literal_node->literal_token.type == Token::Type::STRING)
            return ScriptAny(literal_node->literal_token.text);
        else if(literal_node->literal_token.type == Token::Type::INTEGER)
        {
            switch(literal_node->literal_token.literal_flag)
            {
            case Token::LiteralFlag::BINARY:
                return ScriptAny(std::stoi(literal_node->literal_token.text.substr(2), nullptr, 2));
            case Token::LiteralFlag::HEXADECIMAL:
                return ScriptAny(std::stoi(literal_node->literal_token.text.substr(2), nullptr, 16));
            case Token::LiteralFlag::OCTAL:
                return ScriptAny(std::stoi(literal_node->literal_token.text.substr(2), nullptr, 8));
            default:
                return ScriptAny(std::stoi(literal_node->literal_token.text, nullptr, 10));
            }
        }
        return ScriptAny();
    }

    void Parser::NextToken()
    {
        if(token_index_ >= tokens_.size())
            current_token_ = nullptr;
        else
            current_token_ = &tokens_[token_index_++];
    }

    std::tuple<std::vector<std::string>, std::vector<std::shared_ptr<ExpressionNode>>> Parser::ParseArgumentList()
    {
        std::vector<std::string> arg_list;
        std::vector<std::shared_ptr<ExpressionNode>> def_args;
        while(current_token_->type != Token::Type::RPAREN && current_token_->type != Token::Type::EOF_)
        {
            arg_list.emplace_back(current_token_->text);
            Consume(Token::Type::IDENTIFIER, "argument list");
            if(current_token_->type == Token::Type::ASSIGN)
            {
                NextToken(); // consume =
                def_args.emplace_back(ParseExpression());
            }
            else if(def_args.size() != 0)
            {
                throw ScriptCompileError(MakeString("Default arguments must be last at ", current_token_->position));
            }

            if(current_token_->type == Token::Type::COMMA)
                NextToken();
            else if(current_token_->type != Token::Type::RPAREN)
                throw ScriptCompileError(MakeString("Arguments must be separated by comma not ",
                    *current_token_, " at ", current_token_->position));
        }
        return std::tuple(arg_list, def_args);
    }

    std::shared_ptr<ClassDeclarationStatementNode> Parser::ParseClassDeclarationStatement()
    {
        const auto kLineNumber = current_token_->position.line;
        const auto& kClassToken = *current_token_;
        NextToken();
        const std::string& kClassName = current_token_->text;
        Consume(Token::Type::IDENTIFIER, "class declaration");
        std::shared_ptr<ExpressionNode> base_class = nullptr;
        if(current_token_->IsKeyword("extends"))
        {
            NextToken();
            base_class = ParseExpression();
            base_class_stack_.push(base_class);
        }
        auto class_def = ParseClassDefinition(kClassToken, kClassName, base_class);
        return std::make_shared<ClassDeclarationStatementNode>(kLineNumber, kClassToken, class_def);
    }

    enum class PropertyType { NONE, GET, SET, STATIC };

    std::shared_ptr<ClassDefinition> Parser::ParseClassDefinition(const Token& class_token, 
            const std::string& class_name, const std::shared_ptr<ExpressionNode>& base_class)
    {
        Consume(Token::Type::LBRACE, "class definition");
        std::shared_ptr<FunctionLiteralNode> constructor = nullptr;
        std::vector<std::string> method_names;
        std::vector<std::shared_ptr<FunctionLiteralNode>> methods;
        std::vector<std::string> get_method_names;
        std::vector<std::shared_ptr<FunctionLiteralNode>> get_methods;
        std::vector<std::string> set_method_names;
        std::vector<std::shared_ptr<FunctionLiteralNode>> set_methods;
        std::vector<std::string> static_method_names;
        std::vector<std::shared_ptr<FunctionLiteralNode>> static_methods;
        while(current_token_->type != Token::Type::RBRACE && current_token_->type != Token::Type::EOF_)
        {
            PropertyType ptype = PropertyType::NONE;
            std::string current_method_name = "";
            if(current_token_->IsIdentifier("get"))
            {
                ptype = PropertyType::GET;
                NextToken();
            }
            else if(current_token_->IsIdentifier("set"))
            {
                ptype = PropertyType::SET;
                NextToken();
            }
            else if(current_token_->IsIdentifier("static"))
            {
                ptype = PropertyType::STATIC;
                NextToken();
            }
            const auto& kIdToken = *current_token_;
            Consume(Token::Type::IDENTIFIER, "class definition");
            current_method_name = kIdToken.text;
            Consume(Token::Type::LPAREN, "class definition");
            auto [arg_names, def_args] = ParseArgumentList();
            NextToken(); // consume )
            Consume(Token::Type::LBRACE, "class definition");
            if(current_method_name != "constructor")
                function_context_stack_.push({FunctionContext::Type::METHOD, 0, 0, {}});
            else 
                function_context_stack_.push({FunctionContext::Type::CONSTRUCTOR, 0, 0, {}});
            auto statements = ParseStatements(Token::Type::RBRACE);
            function_context_stack_.pop();
            NextToken(); // consume }
            if(current_method_name == "constructor")
            {
                if(ptype != PropertyType::NONE)
                    throw ScriptCompileError(MakeString("Get, set, or static not allowed for constructor at ",
                        class_token.position));
                if(constructor != nullptr)
                    throw ScriptCompileError(MakeString("Classes may only have one constructor at ",
                        class_token.position));
                if(base_class != nullptr)
                {
                    int num_supers = 0;
                    for(const auto& stmt : statements)
                    {
                        if(auto expr_stmt = std::dynamic_pointer_cast<ExpressionStatementNode>(stmt))
                        {
                            if(auto fcn = std::dynamic_pointer_cast<FunctionCallNode>(expr_stmt->expression_node))
                            {
                                if(auto supernode = std::dynamic_pointer_cast<SuperNode>(fcn->function_to_call))
                                    num_supers++;
                            }
                        }
                    }
                    if(num_supers != 1)
                        throw ScriptCompileError(MakeString("Derived class constructors must have one super call at ",
                            class_token.position));
                    constructor = std::make_shared<FunctionLiteralNode>(kIdToken, arg_names, def_args, statements, 
                        class_name, true);
                }
                else 
                {
                    switch(ptype)
                    {
                    case PropertyType::NONE: {
                        const auto kTrueName = (class_name != "<anonymous class>" && class_name != "") ?
                            (class_name + ".prototype." + current_method_name) :
                            current_method_name;
                        methods.emplace_back(std::make_shared<FunctionLiteralNode>(kIdToken, arg_names, def_args,
                            statements, kTrueName));
                        method_names.emplace_back(current_method_name);
                        break;
                    }
                    case PropertyType::GET:
                        get_methods.emplace_back(std::make_shared<FunctionLiteralNode>(kIdToken, arg_names,
                            def_args, statements));
                        get_method_names.emplace_back(current_method_name);
                        break;
                    case PropertyType::SET:
                        set_methods.emplace_back(std::make_shared<FunctionLiteralNode>(kIdToken, arg_names,
                            def_args, statements));
                        set_method_names.emplace_back(current_method_name);
                        break;
                    case PropertyType::STATIC: {
                        const auto kTrueName = (class_name != "<anonymous class>" && class_name != "") ?
                            (class_name + "." + current_method_name) : current_method_name;
                        static_methods.emplace_back(std::make_shared<FunctionLiteralNode>(kIdToken, arg_names,
                            def_args, statements, kTrueName));
                        static_method_names.emplace_back(current_method_name);
                        break;
                    }
                    }
                }
            }
        }
        NextToken(); // eat } of class body

        std::unordered_map<std::string, bool> mname_map;
        for(const auto& mname : method_names)
        {
            if(mname_map.count(mname) > 0)
                throw ScriptCompileError(MakeString("Duplicate methods are not allowed at ", 
                    class_token.position));
            mname_map[mname] = true;
        }

        // TODO check setters, getters, and static

        if(constructor == nullptr)
        {
            constructor = std::make_shared<FunctionLiteralNode>(class_token, std::vector<std::string>(), 
                std::vector<std::shared_ptr<ExpressionNode>>(), std::vector<std::shared_ptr<StatementNode>>(), 
                class_name, true);
        }
        if(base_class != nullptr)
            base_class_stack_.pop();
        return std::make_shared<ClassDefinition>(class_name, constructor, method_names, methods, get_method_names,
            get_methods, set_method_names, set_methods, static_method_names, static_methods, base_class);
    }

    std::shared_ptr<ClassLiteralNode> Parser::ParseClassExpression()
    {
        const auto& class_token = *current_token_;
        NextToken();
        std::string class_name = "<anonymous class>";
        if(current_token_->type == Token::Type::IDENTIFIER)
        {
            class_name = current_token_->text;
            NextToken();
        }
        std::shared_ptr<ExpressionNode> base_class = nullptr;
        if(current_token_->IsKeyword("extends"))
        {
            NextToken();
            base_class = ParseExpression();
            base_class_stack_.push(base_class);
        }
        auto class_def = ParseClassDefinition(class_token, class_name, base_class);
        return std::make_shared<ClassLiteralNode>(class_token, class_def);
    }

    std::vector<std::shared_ptr<ExpressionNode>> Parser::ParseCommaSeparatedExpressions(const Token::Type stop)
    {
        std::vector<std::shared_ptr<ExpressionNode>> expressions;
        while(current_token_->type != stop && current_token_->type != Token::Type::EOF_ 
          && !current_token_->IsIdentifier("of") && !current_token_->IsIdentifier("in"))
        {
            expressions.emplace_back(ParseExpression());
            if(current_token_->type == Token::Type::COMMA)
                NextToken();
            else if(current_token_->type != stop
              && !current_token_->IsIdentifier("of")
              && !current_token_->IsKeyword("in"))
                throw ScriptCompileError(MakeString("Comma separated list items must be separated by ',' not ",
                    *current_token_, " or missing ", stop, " at ", current_token_->position));
        }

        return expressions;
    }

    std::shared_ptr<DoWhileStatementNode> Parser::ParseDoWhileStatement(const std::string& label)
    {
        const auto kLineNumber = current_token_->position.line;
        NextToken();
        auto loop_body = ParseStatement();
        ConsumeText(Token::Type::KEYWORD, "while", "do while statement");
        Consume(Token::Type::LPAREN, "do while statement");
        auto condition = ParseExpression();
        Consume(Token::Type::RPAREN, "do while statement");
        Consume(Token::Type::SEMICOLON, "do while statement");
        return std::make_shared<DoWhileStatementNode>(kLineNumber, loop_body, condition, label);
    }

    std::shared_ptr<StatementNode> Parser::ParseForStatement(const std::string& label)
    {
        const auto kLineNumber = current_token_->position.line;
        NextToken();
        Consume(Token::Type::LPAREN, "for statement");
        std::shared_ptr<VarDeclarationStatementNode> decl = nullptr;
        if(current_token_->type != Token::Type::SEMICOLON)
            decl = ParseVarDeclarationStatement(false);
        if(current_token_->IsKeyword("in") || current_token_->IsIdentifier("of"))
        {
            const auto& kOfInToken = *current_token_;
            if(decl == nullptr)
                throw ScriptCompileError(MakeString("Invalid for in/of statement at ",
                    current_token_->position));
            const auto& qualifier = decl->qualifier_token;
            std::vector<std::shared_ptr<VarAccessNode>> vans;
            if(decl->qualifier_token.text != "const" && decl->qualifier_token.text != "let")
                throw ScriptCompileError(MakeString("For of/in loop declaration must be local at ",
                    decl->qualifier_token.position));
            int van_count = 0;
            for(const auto& va : decl->assignment_nodes)
            {
                auto valid = std::dynamic_pointer_cast<VarAccessNode>(va);
                if(valid == nullptr)
                    throw ScriptCompileError(MakeString("Invalid variable declaration in for of/in statement",
                        " at ", qualifier.position));
                vans.emplace_back(valid);
                ++van_count;
            }
            if(van_count > 2)
                throw ScriptCompileError(MakeString("For of/in loops may only have up to two declarations ",
                    " at ", qualifier.position));
            NextToken();
            auto obj_to_iterate = ParseExpression();
            Consume(Token::Type::RPAREN, "for " + kOfInToken.text + " loop");
            auto body_statement = ParseStatement();
            return std::make_shared<ForOfStatementNode>(kLineNumber, qualifier, kOfInToken, vans, obj_to_iterate,
                body_statement, label);
        }
        else if(current_token_->type == Token::Type::SEMICOLON)
        {
            NextToken();
            std::shared_ptr<ExpressionNode> condition = nullptr;
            if(current_token_->type != Token::Type::SEMICOLON)
            {
                condition = ParseExpression();
                if(current_token_->type != Token::Type::SEMICOLON)
                    throw ScriptCompileError(MakeString("Expected ';' after for condition at ", 
                        current_token_->position));
            }
            else 
            {
                condition = std::make_shared<LiteralNode>(Token::CreateFakeToken(Token::Type::KEYWORD, "true"));
            }
            NextToken();
            std::shared_ptr<ExpressionNode> increment = nullptr;
            if(current_token_->type != Token::Type::RPAREN)
            {
                increment = ParseExpression();
            }
            else 
            {
                increment = std::make_shared<LiteralNode>(Token::CreateFakeToken(Token::Type::KEYWORD, "true"));
            }
            Consume(Token::Type::RPAREN, "for statement");
            auto body_node = ParseStatement();
            return std::make_shared<ForStatementNode>(kLineNumber, decl, condition, increment, body_node, label);
        }
        else 
            throw ScriptCompileError(MakeString("Invalid for statement at ", current_token_->position));
    }

    std::shared_ptr<FunctionDeclarationStatementNode> Parser::ParseFunctionDeclarationStatement()
    {
        const auto kLineNumber = current_token_->position.line;
        bool is_generator = false;
        NextToken();
        if(current_token_->type == Token::Type::STAR)
        {
            is_generator = true;
            NextToken();
        }
        std::string name = current_token_->text;
        Consume(Token::Type::IDENTIFIER, "function declaration statement");
        Consume(Token::Type::LPAREN, " function declaration statement");
        auto [arg_names, def_args] = ParseArgumentList();
        NextToken(); // consume )
        // TODO implement unique util function
        /*std::vector<std::string> uniq;
        std::unique_copy(arg_names.begin(), arg_names.end(), uniq);
        if(uniq.size() != arg_names.size())
            throw ScriptCompileError(MakeString("Function argument names must be unique at ",
                current_token_->position));*/
        Consume(Token::Type::LBRACE, "function declaration statement");
        function_context_stack_.push({(is_generator? FunctionContext::Type::GENERATOR: FunctionContext::Type::NORMAL),
            0, 0, {}});
        auto statements = ParseStatements(Token::Type::RBRACE);
        function_context_stack_.pop();
        NextToken(); // consume }
        return std::make_shared<FunctionDeclarationStatementNode>(kLineNumber, name, arg_names, def_args, 
            statements, is_generator);
    }

    std::shared_ptr<FunctionLiteralNode> Parser::ParseFunctionLiteral()
    {
        bool is_g = false;
        const auto& token = *current_token_;
        NextToken();
        if(current_token_->type == Token::Type::STAR)
        {
            is_g = true;
            NextToken();
        }
        std::string opt_name = "";
        if(current_token_->type == Token::Type::IDENTIFIER)
        {
            opt_name = current_token_->text;
            NextToken();
        }

        Consume(Token::Type::LPAREN, "function literal");
        auto [arg_names, def_args] = ParseArgumentList();
        NextToken(); // consume )
        Consume(Token::Type::LBRACE, "function literal");
        NextToken();
        function_context_stack_.push({
            is_g ? FunctionContext::Type::GENERATOR : FunctionContext::Type::NORMAL,
            0, 0, std::vector<std::string>()}
        );
        auto statements = ParseStatements(Token::Type::RBRACE);
        function_context_stack_.pop();
        NextToken(); // consume }
        return std::make_shared<FunctionLiteralNode>(token, arg_names, def_args, statements, opt_name, false, is_g);
    }

    std::shared_ptr<IfStatementNode> Parser::ParseIfStatement()
    {
        const auto kLineNumber = current_token_->position.line;
        NextToken();
        Consume(Token::Type::LPAREN, "if statement");
        auto condition = ParseExpression();
        Consume(Token::Type::RPAREN, "if statement");
        auto true_statement = ParseStatement();
        std::shared_ptr<StatementNode> else_statement = nullptr;
        if(current_token_->IsKeyword("else"))
        {
            NextToken();
            else_statement = ParseStatement();
        }
        return std::make_shared<IfStatementNode>(kLineNumber, condition, true_statement, else_statement);
    }

    std::shared_ptr<LambdaNode> Parser::ParseLambda(bool has_parentheses)
    {
        std::vector<std::string> arg_list;
        std::vector<std::shared_ptr<ExpressionNode>> default_args;
        if(has_parentheses)
        {
            NextToken(); // consume (
            auto result = ParseArgumentList();
            arg_list = std::get<0>(result);
            default_args = std::get<1>(result);
            NextToken(); // consume )
        }
        else 
        {
            arg_list.emplace_back(current_token_->text);
            Consume(Token::Type::IDENTIFIER, "lambda expression");
        }
        const auto& arrow = *current_token_;
        Consume(Token::Type::ARROW, "lambda expression");
        if(current_token_->type == Token::Type::LBRACE)
        {
            NextToken(); // consume {
            auto stmts = ParseStatements(Token::Type::RBRACE);
            NextToken(); // consume }
            return std::make_shared<LambdaNode>(arrow, arg_list, default_args, stmts);
        }
        else 
        {
            auto expr = ParseExpression();
            return std::make_shared<LambdaNode>(arrow, arg_list, default_args, expr);
        }
    }

    std::shared_ptr<StatementNode> Parser::ParseLoopStatement()
    {
        std::string label = "";
        if(current_token_->type == Token::Type::LABEL)
        {
            label = current_token_->text;
            function_context_stack_.top().label_stack.emplace_back(label);
            NextToken();
        }
        std::shared_ptr<StatementNode> statement;
        if(current_token_->IsKeyword("while"))
        {
            function_context_stack_.top().loop_stack++;
            statement = ParseWhileStatement(label);
            function_context_stack_.top().loop_stack--;
        }
        else if(current_token_->IsKeyword("do"))
        {
            function_context_stack_.top().loop_stack++;
            statement = ParseDoWhileStatement(label);
            function_context_stack_.top().loop_stack--;
        }
        else if(current_token_->IsKeyword("for"))
        {
            function_context_stack_.top().loop_stack++;
            statement = ParseForStatement(label);
            function_context_stack_.top().loop_stack--;
        }
        else 
        {
            throw ScriptCompileError(MakeString("Labels may only be used before loops at ",
                current_token_->position));
        }
        if(label != "")
            function_context_stack_.top().label_stack.resize(
                function_context_stack_.top().label_stack.size() - 1);
        return statement;
    }

    std::shared_ptr<NewExpressionNode> Parser::ParseNewExpression()
    {
        NextToken();
        auto expression = ParseExpression();
        auto fcn = std::dynamic_pointer_cast<FunctionCallNode>(expression);
        if(fcn == nullptr)
        {
            fcn = std::make_shared<FunctionCallNode>(expression, std::vector<std::shared_ptr<ExpressionNode>>(), 
                true);
        }
        else 
        {
            fcn = std::make_shared<FunctionCallNode>(fcn->function_to_call, fcn->argument_nodes, true);
        }
        return std::make_shared<NewExpressionNode>(fcn);
    }

    std::shared_ptr<ObjectLiteralNode> Parser::ParseObjectLiteral()
    {
        const auto& start_token = *current_token_;
        NextToken(); // consume {
        std::vector<std::string> keys;
        std::vector<std::shared_ptr<ExpressionNode>> value_expressions;
        while(current_token_->type != Token::Type::RBRACE)
        {
            const auto& id_token = *current_token_;
            if(current_token_->type != Token::Type::IDENTIFIER && current_token_->type != Token::Type::STRING
              && current_token_->type != Token::Type::LABEL)
                throw ScriptCompileError(MakeString("Invalid key for object literal ", *current_token_, " at ",
                    current_token_->position));
            keys.push_back(current_token_->text);
            NextToken();
            if(id_token.type != Token::Type::LABEL)
                Consume(Token::Type::COLON, "object literal");

            value_expressions.emplace_back(ParseExpression());
            if(current_token_->type == Token::Type::COMMA)
                NextToken();
            else if(current_token_->type != Token::Type::RBRACE)
                throw ScriptCompileError(MakeString("Key value pairs must be separated by ',' not ", 
                    *current_token_, " at ", current_token_->position));
        }
        NextToken(); // consume }
        if(keys.size() != value_expressions.size())
            throw ScriptCompileError(MakeString("Malformed object literal at ", start_token.position));
        return std::make_shared<ObjectLiteralNode>(keys, value_expressions);
    }

    std::shared_ptr<ExpressionNode> Parser::ParsePrimaryExpression()
    {
        std::shared_ptr<ExpressionNode> left = nullptr;
        CheckEOF("primary expression");
        switch(current_token_->type)
        {
        case Token::Type::LPAREN: {
            auto lookahead = PeekTokens(3);
            if((lookahead[1].type == Token::Type::COMMA ||
                lookahead[1].type == Token::Type::ARROW ||
                lookahead[2].type == Token::Type::ARROW) && // TODO handle lambdas in math expression although invalid
               lookahead[0].type != Token::Type::LPAREN)
            {
                left = ParseLambda(true);
            }
            else 
            {
                NextToken();
                left = ParseExpression();
                CheckEOF("parenthesis expression");
                Consume(Token::Type::RPAREN, "primary expression");
            }
            break;
        }
        case Token::Type::LBRACE:
            left = ParseObjectLiteral();
            break;
        case Token::Type::DOUBLE: // compiler handles values in this version
        case Token::Type::INTEGER:
        case Token::Type::REGEX:
            left = std::make_shared<LiteralNode>(*current_token_);
            NextToken();
            break;
        case Token::Type::STRING:
            if(current_token_->literal_flag == Token::LiteralFlag::TEMPLATE_STRING)
                left = ParseTemplateString();
            else 
                left = std::make_shared<LiteralNode>(*current_token_);
            NextToken();
            break;
        case Token::Type::KEYWORD:
            if(current_token_->text == "true" || current_token_->text == "false" || 
              current_token_->text == "null" || current_token_->text == "undefined")
            {
                left = std::make_shared<LiteralNode>(*current_token_);
                NextToken();
            }
            else if(current_token_->text == "function")
                left = ParseFunctionLiteral();
            else if(current_token_->text == "class")
                left = ParseClassExpression();
            else if(current_token_->text == "new")
                left = ParseNewExpression();
            else if(current_token_->text == "super")
                left = ParseSuper();
            else if(current_token_->text == "yield")
                left = ParseYield();
            else 
                throw new ScriptCompileError(MakeString("Unexpected keyword ", current_token_->text, 
                    " in primary expression at ", current_token_->position));
            break;
        case Token::Type::IDENTIFIER: {
            auto lookahead = PeekToken();
            if(lookahead.type == Token::Type::ARROW)
            {
                left = ParseLambda(false);
            }
            else 
            {
                left = std::make_shared<VarAccessNode>(*current_token_);
                NextToken();
            }
            break;
        }
        case Token::Type::LBRACKET: {
            NextToken(); // consume [
            auto values = ParseCommaSeparatedExpressions(Token::Type::RBRACKET);
            NextToken(); // consume ]
            left = std::make_shared<ArrayLiteralNode>(values);
            break;
        }
        default:
            throw ScriptCompileError(MakeString("Unexpected token ", *current_token_, " in primary expression at ",
                current_token_->position));
        }
        return left;
    }

    std::shared_ptr<StatementNode> Parser::ParseStatement()
    {
        CheckEOF("statement");
        const auto kLineNumber = current_token_->position.line;
        if(current_token_->IsKeyword("var") 
          || current_token_->IsKeyword("let")
          || current_token_->IsKeyword("const"))
        {
            return ParseVarDeclarationStatement();
        }
        else if(current_token_->type == Token::Type::LBRACE)
        {
            NextToken(); // consume {
            auto statements = ParseStatements(Token::Type::RBRACE);
            NextToken(); // consume }
            return std::make_shared<BlockStatementNode>(kLineNumber, statements);
        }
        else if(current_token_->IsKeyword("if"))
        {
            return ParseIfStatement();
        }
        else if(current_token_->IsKeyword("swtich"))
        {
            return ParseSwitchStatement();
        }
        else if(TokenBeginsLoop(*current_token_))
        {
            return ParseLoopStatement();
        }
        else if(current_token_->IsKeyword("break"))
        {
            if(function_context_stack_.top().loop_stack == 0 
              && function_context_stack_.top().switch_stack == 0 )
            {
                throw ScriptCompileError(MakeString("Break statement only allowed in loops or switch body at ",
                    current_token_->position));
            }
            const auto& break_token = *current_token_;
            NextToken();
            std::string label = "";
            if(current_token_->type == Token::Type::IDENTIFIER)
            {
                label = current_token_->text;
                bool valid = false;
                for(size_t i = function_context_stack_.top().label_stack.size(); i > 0; --i)
                {
                    if(function_context_stack_.top().label_stack[i - 1] == label)
                    {
                        valid = true;
                        break;
                    }
                }
                if(!valid)
                    throw ScriptCompileError(MakeString("Break label ", label, " does not refer to valid label at ",
                        current_token_->position));
                NextToken();
            }
            Consume(Token::Type::SEMICOLON, "break statement");
            return std::make_shared<BreakOrContinueStatementNode>(break_token, label);
        }
        else if(current_token_->IsKeyword("continue"))
        {
            if(function_context_stack_.top().loop_stack == 0)
                throw ScriptCompileError(MakeString("Continue statement only allowed in loops at ",
                    current_token_->position));
            const auto& continue_token = *current_token_;
            NextToken();
            std::string label = "";
            if(current_token_->type == Token::Type::IDENTIFIER)
            {
                label = current_token_->text;
                bool valid = false;
                for(size_t i = function_context_stack_.top().label_stack.size(); i > 0; --i)
                {
                    if(function_context_stack_.top().label_stack[i - 1] == label)
                    {
                        valid = true;
                        break;
                    }
                }
                if(!valid)
                    throw ScriptCompileError(MakeString("Continue label ", label, " does not refer to valid label at ",
                        current_token_->position));
                NextToken();
            }
            Consume(Token::Type::SEMICOLON, "continue statement");
            return std::make_shared<BreakOrContinueStatementNode>(continue_token, label);
        }
        else if(current_token_->IsKeyword("return"))
        {
            NextToken();
            std::shared_ptr<ExpressionNode> expression = nullptr;
            if(current_token_->type != Token::Type::SEMICOLON)
                expression = ParseExpression();
            Consume(Token::Type::SEMICOLON, "return statement");
            return std::make_shared<ReturnStatementNode>(kLineNumber, expression);
        }
        else if(current_token_->IsKeyword("function"))
        {
            return ParseFunctionDeclarationStatement();
        }
        else if(current_token_->IsKeyword("throw"))
        {
            NextToken();
            auto expr = ParseExpression();
            Consume(Token::Type::SEMICOLON, "throw statement");
            return std::make_shared<ThrowStatementNode>(kLineNumber, expr);
        }
        else if(current_token_->IsKeyword("try"))
        {
            return ParseTryBlockStatement();
        }
        else if(current_token_->IsKeyword("delete"))
        {
            const auto& kDelToken = *current_token_;
            NextToken();
            auto expression = ParseExpression();
            if(!(std::dynamic_pointer_cast<MemberAccessNode>(expression)
              || std::dynamic_pointer_cast<ArrayIndexNode>(expression)))
                throw ScriptCompileError(MakeString("Invalid operand for delete: ", expression->to_string(), 
                    " at ", kDelToken.position));
            return std::make_shared<DeleteStatementNode>(kDelToken, expression);
        }
        else if(current_token_->IsKeyword("class"))
        {
            return ParseClassDeclarationStatement();
        }
        else 
        {
            if(current_token_->type == Token::Type::SEMICOLON)
            {
                NextToken();
                return std::make_shared<ExpressionStatementNode>(kLineNumber, nullptr);
            }
            else 
            {
                auto expression = ParseExpression();
                if(current_token_->type != Token::Type::SEMICOLON && current_token_->type != Token::Type::EOF_)
                    throw ScriptCompileError(MakeString("Expected semicolon after expression statement at ",
                        current_token_->position));
                NextToken();
                return std::make_shared<ExpressionStatementNode>(kLineNumber, expression);
            }
        }
    }

    std::vector<std::shared_ptr<StatementNode>> Parser::ParseStatements(const Token::Type stop)
    {
        std::vector<std::shared_ptr<StatementNode>> statements;
        while(current_token_->type != stop && current_token_->type != Token::Type::EOF_)
        {
            statements.emplace_back(ParseStatement());
        }
        return statements;
    }

    std::shared_ptr<SuperNode> Parser::ParseSuper()
    {
        const auto& stoken = *current_token_;
        if(base_class_stack_.size() < 1)
            throw ScriptCompileError(MakeString("Super expression only allowed in derived classes at ", 
                stoken.position));
        NextToken(); // consume 'super'
        return std::make_shared<SuperNode>(stoken, base_class_stack_.top());
    }

    std::shared_ptr<SwitchStatementNode> Parser::ParseSwitchStatement()
    {
        function_context_stack_.top().switch_stack++;
        const auto kLineNumber = current_token_->position.line;
        const auto& switch_token = *current_token_;
        NextToken();
        Consume(Token::Type::LPAREN, "switch statement");
        auto expression = ParseExpression();
        Consume(Token::Type::RPAREN, "switch statement");
        Consume(Token::Type::LBRACE, "switch statement");
        bool case_started = false;
        size_t statement_counter = 0;
        std::vector<std::shared_ptr<StatementNode>> statement_nodes;
        size_t default_statement_id = static_cast<size_t>(-1);
        std::unordered_map<ScriptAny, size_t> jump_table;
        while(current_token_->type != Token::Type::RBRACE)
        {
            if(current_token_->IsKeyword("case"))
            {
                NextToken();
                case_started = true;
                auto case_expression = ParseExpression();
                auto result = EvaluateCTFE(case_expression);
                if(result.type() == ScriptAny::Type::UNDEFINED)
                    throw ScriptCompileError(MakeString("Case expressions must be known at compile time at ",
                        switch_token.position));
                Consume(Token::Type::COLON, "switch statement");
                if(jump_table.count(result) > 0)
                    throw ScriptCompileError(MakeString("Duplicate case entries not allowed at ",
                        switch_token.position));
                jump_table[result] = statement_counter;
            }
            else if(current_token_->IsKeyword("default"))
            {
                case_started = true;
                NextToken();
                Consume(Token::Type::COLON, "switch statement");
                default_statement_id = statement_counter;
            }
            else 
            {
                if(!case_started)
                    throw ScriptCompileError(MakeString("Case condition required before any statements at ",
                        current_token_->position));
                statement_nodes.emplace_back(ParseStatement());
                ++statement_counter;
            }
        }
        NextToken(); // consume }
        function_context_stack_.top().switch_stack--;
        return std::make_shared<SwitchStatementNode>(kLineNumber, expression, statement_nodes, 
            default_statement_id, jump_table);
    }

    static std::string PeekTwo(const std::string& text, const size_t i)
    {
        std::string result;
        result += ((i >= text.length()) ? '\0' : text[i]);
        result += ((i + 1>= text.length()) ? '\0' : text[i+1]);
        return result;
    }

    std::shared_ptr<TemplateStringNode> Parser::ParseTemplateString()
    {
        bool lit_state = true;
        size_t text_index;
        std::string current_expr;
        std::string current_lit;
        std::vector<std::shared_ptr<ExpressionNode>> nodes;
        int bracket_stack = 0;

        while(text_index < current_token_->text.length())
        {
            if(lit_state)
            {
                if(PeekTwo(current_token_->text, text_index) == "${")
                {
                    current_expr = "";
                    text_index += 2;
                    lit_state = false;
                    if(current_lit.length() > 0)
                        nodes.emplace_back(std::make_shared<LiteralNode>(
                            Token::CreateFakeToken(Token::Type::STRING, current_lit)));
                }
                else 
                {
                    current_lit += current_token_->text[text_index++];
                }
            }
            else 
            {
                if(current_token_->text[text_index] == '}')
                {
                    if(bracket_stack)
                    {
                        current_expr += current_token_->text[text_index++];
                        --bracket_stack;
                    }
                    else 
                    {
                        current_lit = "";
                        text_index++;
                        lit_state = true;
                        if(current_expr.length() > 0)
                        {
                            auto lexer = Lexer(current_expr);
                            auto tokens = lexer.Tokenize();
                            if(lexer.HasErrors())
                            {
                                throw ScriptCompileError(MakeString("Invalid characters in template expression at ",
                                    current_token_->position));
                            }
                            auto parser = Parser(tokens);
                            nodes.emplace_back(parser.ParseExpression());
                            if(parser.current_token_->type != Token::Type::EOF_)
                            {
                                throw ScriptCompileError(MakeString("Unexpected token in template expression: ",
                                    parser.current_token_->type, " at ", current_token_->position));
                            }
                        }
                    }
                }
                else 
                {
                    if(current_token_->text[text_index] == '{')
                        ++bracket_stack;
                    current_expr += current_token_->text[text_index++];
                }
            }
        }
        if(lit_state == false)
            throw ScriptCompileError(MakeString("Unclosed template expression at ", current_token_->position));
        if(current_lit.length() > 0)
            nodes.emplace_back(std::make_shared<LiteralNode>(
                Token::CreateFakeToken(Token::Type::STRING, current_lit)));
        return std::make_shared<TemplateStringNode>(nodes);
    }

    std::shared_ptr<TryBlockStatementNode> Parser::ParseTryBlockStatement()
    {
        const auto kLineNumber = current_token_->position.line;
        const auto& kTryToken = *current_token_;
        NextToken();
        auto try_block = ParseStatement();
        std::shared_ptr<StatementNode> catch_block = nullptr, finally_block = nullptr;
        std::string name = "";
        if(current_token_->IsKeyword("catch"))
        {
            NextToken();
            if(current_token_->type == Token::Type::LPAREN)
            {
                NextToken();
                name = current_token_->text;
                Consume(Token::Type::IDENTIFIER, "try statement catch block");
                Consume(Token::Type::RPAREN, "try statement catch block");
            }
            catch_block = ParseStatement();
        }
        if(current_token_->IsKeyword("finally"))
        {
            NextToken();
            finally_block = ParseStatement();
        }
        if(catch_block == nullptr && finally_block == nullptr)
            throw ScriptCompileError(MakeString("Try statements must have catch and/or finally block",
                " at ", kTryToken.position));
        return std::make_shared<TryBlockStatementNode>(kLineNumber, try_block, name, catch_block, finally_block);
    }

    std::shared_ptr<VarDeclarationStatementNode> Parser::ParseVarDeclarationStatement(const bool consume_semicolon)
    {
        const auto& kSpecifier = *current_token_;
        NextToken();
        std::vector<std::shared_ptr<ExpressionNode>> expressions;
        while(current_token_->type != Token::Type::SEMICOLON
          && current_token_->type != Token::Type::EOF_
          && !current_token_->IsIdentifier("of")
          && !current_token_->IsKeyword("in"))
        {
            std::string var_name = "";
            if(current_token_->type == Token::Type::IDENTIFIER)
            {
                var_name = current_token_->text;
                NextToken();
            }
            else if(current_token_->type == Token::Type::LBRACE || current_token_->type == Token::Type::LBRACKET)
            {
                var_name += current_token_->Symbol();
                const auto kEndTokenType = (current_token_->type == Token::Type::LBRACE) ? 
                    Token::Type::RBRACE : Token::Type::RBRACKET;
                NextToken();
                bool spread_listed = false;
                while(current_token_->type != kEndTokenType && current_token_->type != Token::Type::EOF_)
                {
                    if(current_token_->type == Token::Type::TDOT)
                    {
                        var_name += '.';
                        NextToken();
                        var_name += current_token_->text;
                        Consume(Token::Type::IDENTIFIER, "destructure var declaration");
                        if(spread_listed)
                            throw ScriptCompileError(MakeString("Only one spread variable allowed at ",
                                current_token_->position));
                        spread_listed = true;
                    }
                    else
                    {
                        var_name += current_token_->text;
                        Consume(Token::Type::IDENTIFIER, "destructure var declaration");
                    }
                    if(current_token_->type == Token::Type::COMMA)
                    {
                        var_name += ',';
                        NextToken();
                    }
                    else if(current_token_->type != kEndTokenType)
                    {
                        throw ScriptCompileError(MakeString("Destructure variable names must be separated by comma",
                            " at ", current_token_->position));
                    }
                }
                if(var_name.length() < 2)
                    throw ScriptCompileError(MakeString("Destructure declaration cannot be empty at ",
                        current_token_->position));
                NextToken(); // consume } or ]
            }
            if(current_token_->type == Token::Type::ASSIGN)
            {
                const auto& kAssignToken = *current_token_;
                NextToken();
                auto expression = ParseExpression();
                expressions.emplace_back(std::make_shared<BinaryOpNode>(kAssignToken, 
                    std::make_shared<VarAccessNode>(Token::CreateFakeToken(Token::Type::IDENTIFIER, var_name)),
                    expression));
            }
            else
            {
                expressions.emplace_back(std::make_shared<VarAccessNode>(
                    Token::CreateFakeToken(Token::Type::IDENTIFIER, var_name)));
            }
            if(current_token_->type == Token::Type::COMMA)
                NextToken();
            else if(current_token_->type != Token::Type::SEMICOLON && current_token_->type != Token::Type::EOF_)
                throw ScriptCompileError(MakeString("Expected ',' between variable declarations ",
                    "(or missing ';') at ", current_token_->position));
        }

        for(const auto& expr : expressions)
        {
            if(auto node = std::dynamic_pointer_cast<BinaryOpNode>(expr) )
            {
                if(!std::dynamic_pointer_cast<VarAccessNode>(node->left_node))
                    throw ScriptCompileError(MakeString("Invalid assignment node at ", node->op_token.position));
            }
            else if(!std::dynamic_pointer_cast<VarAccessNode>(expr) )
            {
                throw ScriptCompileError(MakeString("Invalid variable name in declaration: ", expr->to_string(), 
                    " at ", kSpecifier.position));
            }
        }
        if(consume_semicolon)
            NextToken();
        return std::make_shared<VarDeclarationStatementNode>(kSpecifier, expressions);
    }

    std::shared_ptr<WhileStatementNode> Parser::ParseWhileStatement(const std::string& label)
    {
        const auto kLineNumber = current_token_->position.line;
        NextToken();
        Consume(Token::Type::LPAREN, "while statement");
        auto condition = ParseExpression();
        Consume(Token::Type::RPAREN, "while statement");
        auto loop_body = ParseStatement();
        return std::make_shared<WhileStatementNode>(kLineNumber, condition, loop_body, label);
    }

    std::shared_ptr<YieldNode> Parser::ParseYield()
    {
        if(function_context_stack_.size() == 0 
          || function_context_stack_.top().fct != FunctionContext::Type::GENERATOR)
            throw ScriptCompileError(MakeString("Yield may only be used in Generator functions at ",
                current_token_->position));
        const auto& ytoken = *current_token_;
        NextToken();
        std::shared_ptr<ExpressionNode> expr = nullptr;
        if(current_token_->type != Token::Type::RBRACE && current_token_->type != Token::Type::SEMICOLON)
            expr = ParseExpression();
        return std::make_shared<YieldNode>(ytoken, expr);
    }

    Token Parser::PeekToken()
    {
        if(token_index_ < tokens_.size())
            return tokens_[token_index_];
        return Token();
    }

    std::vector<Token> Parser::PeekTokens(int num)
    {
        std::vector<Token> list;
        for(size_t i = token_index_; i < token_index_ + num; ++i)
        {
            if(i < tokens_.size())
                list.emplace_back(tokens_[i]);
            else 
                list.emplace_back(Token());
        }
        return list;
    }

    void Parser::PutbackToken()
    {
        if(token_index_ > 0)
            current_token_ = &tokens_[--token_index_];
    }

} // namespace mildew
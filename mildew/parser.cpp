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
#include "parser.h"

#include "errors.h"
#include "lexer.h"
#include "util/sfmt.h"

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
        // else if(literal_node->literal_token.type == Token::Type::STRING)
        //    return ScriptAny(literal_node->literal_token.text);
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
        throw UnimplementedError("class declaration");
    }

    std::shared_ptr<ClassDefinition> Parser::ParseClassDefinition(const Token& class_token, 
            const std::string& class_name, const std::shared_ptr<ExpressionNode>& base_class)
    {
        throw UnimplementedError("class definition");
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
        throw UnimplementedError("do while");
    }

    std::shared_ptr<StatementNode> Parser::ParseForStatement(const std::string& label)
    {
        throw UnimplementedError("for");
    }

    std::shared_ptr<FunctionDeclarationStatementNode> Parser::ParseFunctionDeclarationStatement()
    {
        throw UnimplementedError("function declaration");
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
        throw UnimplementedError("if");
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
        throw UnimplementedError("loop statement");
    }

    std::shared_ptr<NewExpressionNode> Parser::ParseNewExpression()
    {
        const auto& new_token = *current_token_;
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
        throw UnimplementedError("statement");
    }

    std::vector<std::shared_ptr<StatementNode>> Parser::ParseStatements(const Token::Type stop)
    {
        throw UnimplementedError("statements");
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
        throw UnimplementedError("switch");
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
        throw UnimplementedError("try block");
    }

    std::shared_ptr<VarDeclarationStatementNode> Parser::ParseVarDeclarationStatement(const bool consume_semicolon)
    {
        throw UnimplementedError("var declaration");
    }

    std::shared_ptr<WhileStatementNode> Parser::ParseWhileStatement(const std::string& label)
    {
        throw UnimplementedError("while");
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
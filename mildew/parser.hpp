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

#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <tuple>
#include <vector>

#include "lexer.hpp"
#include "nodes.hpp"
#include "types/any.hpp"

namespace mildew
{
    struct Parser
    {
        Parser(const std::vector<Token>& tokens) : tokens_(tokens) { NextToken(); }
        // TODO Parser that accepts Compiler reference to use CTFE properly

        std::shared_ptr<BlockStatementNode> ParseProgram();
        std::shared_ptr<ExpressionNode> ParseExpression(int min_prec = 1);

    private:
        void CheckEOF(const std::string& where="") const;
        void Consume(const Token::Type token_type, const std::string& where="");
        void ConsumeText(const Token::Type token_type, const std::string& text, const std::string& where="");
        ScriptAny EvaluateCTFE(const std::shared_ptr<ExpressionNode>& expr);
        void NextToken();
        std::tuple<std::vector<std::string>, std::vector<std::shared_ptr<ExpressionNode>>> ParseArgumentList();
        std::shared_ptr<ClassDeclarationStatementNode> ParseClassDeclarationStatement();
        std::shared_ptr<ClassDefinition> ParseClassDefinition(const Token& class_token, const std::string& class_name,
            const std::shared_ptr<ExpressionNode>& base_class);
        std::shared_ptr<ClassLiteralNode> ParseClassExpression();
        std::vector<std::shared_ptr<ExpressionNode>> ParseCommaSeparatedExpressions(const Token::Type stop);
        std::shared_ptr<DoWhileStatementNode> ParseDoWhileStatement(const std::string& label = "");
        std::shared_ptr<StatementNode> ParseForStatement(const std::string& label = "");
        std::shared_ptr<FunctionDeclarationStatementNode> ParseFunctionDeclarationStatement();
        std::shared_ptr<FunctionLiteralNode> ParseFunctionLiteral();
        std::shared_ptr<IfStatementNode> ParseIfStatement();
        std::shared_ptr<LambdaNode> ParseLambda(bool has_parentheses);
        std::shared_ptr<StatementNode> ParseLoopStatement();
        std::shared_ptr<NewExpressionNode> ParseNewExpression();
        std::shared_ptr<ObjectLiteralNode> ParseObjectLiteral();
        std::shared_ptr<ExpressionNode> ParsePrimaryExpression();
        std::shared_ptr<StatementNode> ParseStatement();
        std::vector<std::shared_ptr<StatementNode>> ParseStatements(const Token::Type stop);
        std::shared_ptr<SuperNode> ParseSuper();
        std::shared_ptr<SwitchStatementNode> ParseSwitchStatement();
        std::shared_ptr<TemplateStringNode> ParseTemplateString();
        std::shared_ptr<TryBlockStatementNode> ParseTryBlockStatement();
        std::shared_ptr<VarDeclarationStatementNode> ParseVarDeclarationStatement(const bool consume_semicolon = true);
        std::shared_ptr<WhileStatementNode> ParseWhileStatement(const std::string& label = "");
        std::shared_ptr<YieldNode> ParseYield();
        Token PeekToken();
        std::vector<Token> PeekTokens(int num);
        void PutbackToken();

        struct FunctionContext
        {
            enum class Type { NORMAL, CONSTRUCTOR, METHOD, GENERATOR };
            Type fct;
            int loop_stack;
            int switch_stack;
            std::vector<std::string> label_stack;
        };

        const std::vector<Token> tokens_;
        size_t token_index_ = 0;
        Token const* current_token_;
        std::stack<FunctionContext> function_context_stack_;
        std::stack<std::shared_ptr<ExpressionNode>> base_class_stack_;
    };
} // namespace mildew
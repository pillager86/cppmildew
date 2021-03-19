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

#include <any>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "lexer.h"
#include "types/any.h"

namespace mildew
{
    // forward declarations
    class IExpressionVisitor;
    class IStatementVisitor;
    class StatementNode;

    class ExpressionNode
    {
    public:
        virtual ~ExpressionNode() {}
        virtual std::any Accept(IExpressionVisitor& visitor) const = 0;
        virtual std::string to_string() const = 0;
    };

    std::ostream& operator<<(std::ostream& os, const ExpressionNode& node);

    class LiteralNode : public ExpressionNode
    {
    public:
        LiteralNode(const Token& token) : literal_token(token) {}
        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token literal_token;
    };

    class FunctionLiteralNode : public ExpressionNode
    {
    public:
        FunctionLiteralNode(
            const Token& t, 
            const std::vector<std::string>& args, 
            const std::vector<std::shared_ptr<ExpressionNode>>& defargs,
            const std::vector<std::shared_ptr<StatementNode>>& stmts,
            const std::string& optname = "",
            const bool is_c = false,
            const bool is_g = false)
        : token(t), arg_list(args), default_arguments(defargs), statements(stmts), optional_name(optname),
          is_class(is_c), is_generator(is_g)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token token;
        const std::vector<std::string> arg_list;
        const std::vector<std::shared_ptr<ExpressionNode>> default_arguments;
        const std::vector<std::shared_ptr<StatementNode>> statements;
        const std::string optional_name;
        const bool is_class;
        const bool is_generator;
    };

    class LambdaNode : public ExpressionNode
    {
    public:
        LambdaNode(const Token& arrow, const std::vector<std::string>& args,
            const std::vector<std::shared_ptr<ExpressionNode>>& defargs, 
            const std::vector<std::shared_ptr<StatementNode>>& stmts)
        : arrow_token(arrow), argument_list(args), default_arguments(defargs), 
          statements(stmts), return_expression(nullptr)
        {}

        LambdaNode(const Token& arrow, const std::vector<std::string>& args,
            const std::vector<std::shared_ptr<ExpressionNode>>& defargs,
            const std::shared_ptr<ExpressionNode>& ret)
        : arrow_token(arrow), argument_list(args), default_arguments(defargs),
          statements(std::vector<std::shared_ptr<StatementNode>>()), return_expression(ret)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token arrow_token;
        const std::vector<std::string> argument_list;
        const std::vector<std::shared_ptr<ExpressionNode>> default_arguments;
        // only one of these may be active at a time:
        const std::vector<std::shared_ptr<StatementNode>> statements;
        std::shared_ptr<ExpressionNode> return_expression;
    };

    class TemplateStringNode : public ExpressionNode
    {
    public:
        TemplateStringNode(const std::vector<std::shared_ptr<ExpressionNode>>& ns)
        : nodes(ns)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::vector<std::shared_ptr<ExpressionNode>> nodes;
    };

    class ArrayLiteralNode : public ExpressionNode
    {
    public:
        ArrayLiteralNode(const std::vector<std::shared_ptr<ExpressionNode>>& values)
        : value_nodes(values)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::vector<std::shared_ptr<ExpressionNode>> value_nodes;
    };

    class ObjectLiteralNode : public ExpressionNode
    {
    public:
        ObjectLiteralNode(const std::vector<std::string>& ks, const std::vector<std::shared_ptr<ExpressionNode>>& vs)
        : keys(ks), value_nodes(vs)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::vector<std::string> keys;
        const std::vector<std::shared_ptr<ExpressionNode>> value_nodes;
    };

    class ClassDefinition
    {
    public:
        ClassDefinition(const std::string& clsname, const std::shared_ptr<FunctionLiteralNode>& ctor,
            const std::vector<std::string>& mnames, const std::vector<std::shared_ptr<FunctionLiteralNode>>& ms,
            const std::vector<std::string>& gmnames, const std::vector<std::shared_ptr<FunctionLiteralNode>>& gms,
            const std::vector<std::string>& smnames, const std::vector<std::shared_ptr<FunctionLiteralNode>>& sms,
            const std::vector<std::string>& stmnames, const std::vector<std::shared_ptr<FunctionLiteralNode>>& stms,
            const std::shared_ptr<ExpressionNode>& base = nullptr)
        : class_name(clsname), constructor(ctor), method_names(mnames), methods(ms),
          get_method_names(gmnames), get_methods(gms), set_method_names(smnames), set_methods(sms),
          static_method_names(stmnames), static_methods(stms), base_class(base)
        {}

        std::string to_string() const;

        const std::string class_name;
        const std::shared_ptr<FunctionLiteralNode> constructor;
        const std::vector<std::string> method_names;
        const std::vector<std::shared_ptr<FunctionLiteralNode>> methods;
        const std::vector<std::string> get_method_names;
        const std::vector<std::shared_ptr<FunctionLiteralNode>> get_methods;
        const std::vector<std::string> set_method_names;
        const std::vector<std::shared_ptr<FunctionLiteralNode>> set_methods;
        const std::vector<std::string> static_method_names;
        const std::vector<std::shared_ptr<FunctionLiteralNode>> static_methods;
        const std::shared_ptr<ExpressionNode> base_class; // hopefully a function
    };

    class ClassLiteralNode : public ExpressionNode
    {
    public:
        ClassLiteralNode(const Token& ctoken, const std::shared_ptr<ClassDefinition>& cdef)
        : class_token(ctoken), class_definition(cdef)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token class_token; // should be class keyword
        const std::shared_ptr<ClassDefinition> class_definition;
    };

    class BinaryOpNode : public ExpressionNode
    {
    public:
        BinaryOpNode(const Token& op, const std::shared_ptr<ExpressionNode>& left, 
            const std::shared_ptr<ExpressionNode>& right)
        : op_token(op), left_node(left), right_node(right)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token op_token;
        const std::shared_ptr<ExpressionNode> left_node;
        const std::shared_ptr<ExpressionNode> right_node;
    };

    class UnaryOpNode : public ExpressionNode
    {
    public:
        UnaryOpNode(const Token& op, const std::shared_ptr<ExpressionNode>& operand, const bool is_post = false)
        : op_token(op), operand_node(operand), is_postfix(is_post)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token op_token;
        const std::shared_ptr<ExpressionNode> operand_node;
        const bool is_postfix;
    };

    class TerniaryOpNode : public ExpressionNode
    {
    public:
        TerniaryOpNode(const std::shared_ptr<ExpressionNode>& cond,
            const std::shared_ptr<ExpressionNode>& on_true,
            const std::shared_ptr<ExpressionNode>& on_false)
        : condition_node(cond), on_true_node(on_true), on_false_node(on_false)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> condition_node;
        const std::shared_ptr<ExpressionNode> on_true_node;
        const std::shared_ptr<ExpressionNode> on_false_node;
    };

    class VarAccessNode : public ExpressionNode
    {
    public:
        VarAccessNode(const Token& token)
        : var_token(token)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token var_token;
    };

    class FunctionCallNode : public ExpressionNode
    {
    public:
        FunctionCallNode(const std::shared_ptr<ExpressionNode>& fn, 
            const std::vector<std::shared_ptr<ExpressionNode>>& args,
            const bool ret = false)
        : function_to_call(fn), argument_nodes(args), return_this(ret)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> function_to_call;
        const std::vector<std::shared_ptr<ExpressionNode>>& argument_nodes;
        const bool return_this;
    };

    class ArrayIndexNode : public ExpressionNode
    {
    public:
        ArrayIndexNode(const std::shared_ptr<ExpressionNode>& obj, 
            const std::shared_ptr<ExpressionNode>& index)
        : object_node(obj), index_node(index)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> object_node;
        const std::shared_ptr<ExpressionNode> index_node;
    };

    class MemberAccessNode : public ExpressionNode
    {
    public:
        MemberAccessNode(const std::shared_ptr<ExpressionNode>& obj,
            const Token& dot, const std::shared_ptr<ExpressionNode>& member)
        : object_node(obj), dot_token(dot), member_node(member)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> object_node;
        const Token dot_token;
        const std::shared_ptr<ExpressionNode> member_node;
    };

    class NewExpressionNode : public ExpressionNode
    {
    public:
        NewExpressionNode(const std::shared_ptr<FunctionCallNode>& fcn)
        : function_call_node(fcn)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<FunctionCallNode> function_call_node;
    };

    class SuperNode : public ExpressionNode
    {
    public:
        SuperNode(const Token& stoken, const std::shared_ptr<ExpressionNode>& base)
        : super_token(stoken), base_class(base)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token super_token;
        const std::shared_ptr<ExpressionNode> base_class;
    };

    class YieldNode : public ExpressionNode
    {
    public:
        YieldNode(const Token& ytoken, const std::shared_ptr<ExpressionNode>& expr)
        : yield_token(ytoken), yield_expression_node(expr)
        {}

        std::any Accept(IExpressionVisitor& visitor) const override;
        std::string to_string() const override;

        const Token yield_token;
        const std::shared_ptr<ExpressionNode> yield_expression_node;
    };

// Statements /////////////////////////////////////////////////////////////////

    class StatementNode
    {
    public:
        StatementNode(const size_t line_no)
        : line(line_no) {}
        virtual ~StatementNode() {}

        virtual std::any Accept(IStatementVisitor& visitor) const = 0;
        virtual std::string to_string() const = 0;

        const size_t line;
    };

    class VarDeclarationStatementNode : public StatementNode
    {
    public:
        VarDeclarationStatementNode(const Token& qual, const std::vector<std::shared_ptr<ExpressionNode>>& nodes)
        : StatementNode(qual.position.line), qualifier_token(qual), assignment_nodes(nodes)
        {}
        VarDeclarationStatementNode(const size_t line_no, const Token& qual,
            const std::vector<std::shared_ptr<ExpressionNode>>& nodes)
        : StatementNode(line_no), qualifier_token(qual), assignment_nodes(nodes)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        // must be var, let, or const
        const Token qualifier_token;
        // must be VarAccessNode or BinaryOpNode, validated by parser
        const std::vector<std::shared_ptr<ExpressionNode>> assignment_nodes;
    };

    class BlockStatementNode: public StatementNode
    {
    public:
        BlockStatementNode(const size_t line_no, const std::vector<std::shared_ptr<StatementNode>>& stmts)
        : StatementNode(line_no), statement_nodes(stmts)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::vector<std::shared_ptr<StatementNode>> statement_nodes;
    };

    class IfStatementNode : public StatementNode
    {
    public:
        IfStatementNode(const size_t line_no, const std::shared_ptr<ExpressionNode>& condition,
            const std::shared_ptr<StatementNode>& on_true, const std::shared_ptr<StatementNode>& on_false=nullptr)
        : StatementNode(line_no), condition_node(condition), on_true_statement(on_true), on_false_statement(on_false)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> condition_node;
        const std::shared_ptr<StatementNode> on_true_statement;
        const std::shared_ptr<StatementNode> on_false_statement;
    };

    class SwitchStatementNode : public StatementNode
    {
    public:
        SwitchStatementNode(const size_t line_no, const std::shared_ptr<ExpressionNode>& expr, 
            const std::vector<std::shared_ptr<StatementNode>>& stmts, const size_t def_id,
            const std::unordered_map<ScriptAny, size_t>& jmptbl)
        : StatementNode(line_no), expression_node(expr), statement_nodes(stmts), 
          default_statement_id(def_id), jump_table(jmptbl)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> expression_node;
        const std::vector<std::shared_ptr<StatementNode>> statement_nodes;
        const size_t default_statement_id; // index into statement_nodes
        const std::unordered_map<ScriptAny, size_t> jump_table;
    };

    class WhileStatementNode : public StatementNode
    {
    public:
        WhileStatementNode(const size_t line_no, const std::shared_ptr<ExpressionNode>& cond,
            const std::shared_ptr<StatementNode>& body, const std::string& lbl = "")
        : StatementNode(line_no), condition_node(cond), body_node(body), label(lbl)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> condition_node;
        const std::shared_ptr<StatementNode> body_node;
        const std::string label;
    };

    class DoWhileStatementNode : public StatementNode 
    {
    public:
        DoWhileStatementNode(const size_t line_no, const std::shared_ptr<StatementNode>& body,
            const std::shared_ptr<ExpressionNode>& cond, const std::string lbl="")
        : StatementNode(line_no), body_node(body), condition_node(cond), label(lbl)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<StatementNode> body_node;
        const std::shared_ptr<ExpressionNode> condition_node;
        const std::string label;
    };

    class ForStatementNode : public StatementNode
    {
    public:
        ForStatementNode(const size_t line_no, const std::shared_ptr<StatementNode>& init, 
            const std::shared_ptr<ExpressionNode>& cond, const std::shared_ptr<ExpressionNode>& inc,
            const std::shared_ptr<StatementNode>& body, const std::string lbl="")
        : StatementNode(line_no), init_statement(init), condition_node(cond), increment_node(inc),
          body_node(body), label(lbl)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<StatementNode> init_statement; // can be any expression statement but usually var decl
        const std::shared_ptr<ExpressionNode> condition_node;
        const std::shared_ptr<ExpressionNode> increment_node;
        const std::shared_ptr<StatementNode> body_node;
        const std::string label;
    };

    class ForOfStatementNode : public StatementNode
    {
    public:
        ForOfStatementNode(const size_t line_no, const Token& qual, const Token& of_in,
            const std::vector<std::shared_ptr<VarAccessNode>>& vars, 
            const std::shared_ptr<ExpressionNode>& obj,
            const std::shared_ptr<StatementNode>& body, const std::string& lbl = "")
        : StatementNode(line_no), qualifier_token(qual), of_in_token(of_in), var_access_nodes(vars),
          object_to_iterate(obj), body_node(body), label(lbl)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const Token qualifier_token;
        const Token of_in_token;
        const std::vector<std::shared_ptr<VarAccessNode>> var_access_nodes;
        const std::shared_ptr<ExpressionNode> object_to_iterate;
        const std::shared_ptr<StatementNode> body_node;
        const std::string label;
    };

    class BreakOrContinueStatementNode : public StatementNode
    {
    public:
        BreakOrContinueStatementNode(const Token& bc, const std::string& lbl="")
        : StatementNode(bc.position.line), break_or_continue(bc), label(lbl)
        {}

        BreakOrContinueStatementNode(const size_t line_no, const Token& bc, const std::string& lbl="")
        : StatementNode(line_no), break_or_continue(bc), label(lbl)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const Token break_or_continue;
        const std::string label;
    };

    class ReturnStatementNode : public StatementNode
    {
    public:
        ReturnStatementNode(const size_t line_no, const std::shared_ptr<ExpressionNode>& expr)
        : StatementNode(line_no), expression_node(expr)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> expression_node;
    };

    class FunctionDeclarationStatementNode : public StatementNode
    {
    public:
        FunctionDeclarationStatementNode(const size_t line_no, const std::string& fname, 
            const std::vector<std::string>& args, const std::vector<std::shared_ptr<ExpressionNode>>& defargs,
            const std::vector<std::shared_ptr<StatementNode>>& stmts, const bool is_g = false)
        : StatementNode(line_no), name(fname), argument_names(args), default_arguments(defargs),
          statement_nodes(stmts), is_generator(is_g)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::string name;
        const std::vector<std::string> argument_names;
        const std::vector<std::shared_ptr<ExpressionNode>> default_arguments;
        const std::vector<std::shared_ptr<StatementNode>> statement_nodes;
        const bool is_generator;
    };

    class ThrowStatementNode : public StatementNode
    {
    public:
        ThrowStatementNode(const size_t line_no, const std::shared_ptr<ExpressionNode>& expr)
        : StatementNode(line_no), expression_node(expr)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> expression_node;
    };

    class TryBlockStatementNode : StatementNode
    {
    public:
        TryBlockStatementNode(const size_t line_no, const std::shared_ptr<StatementNode>& tryb, 
            const std::string& exname, const std::shared_ptr<StatementNode>& catchb,
            const std::shared_ptr<StatementNode>& finb)
        : StatementNode(line_no), try_block_node(tryb), exception_name(exname),
          catch_block_node(catchb), finally_block_node(finb)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<StatementNode> try_block_node;
        const std::string exception_name;
        const std::shared_ptr<StatementNode> catch_block_node;
        const std::shared_ptr<StatementNode> finally_block_node;
    };

    class DeleteStatementNode : public StatementNode
    {
    public:
        DeleteStatementNode(const Token& dtoken, const std::shared_ptr<ExpressionNode>& access)
        : StatementNode(dtoken.position.line), delete_token(dtoken), access_node(access)
        {}

        DeleteStatementNode(const size_t line_no, const Token& dtoken, 
            const std::shared_ptr<ExpressionNode>& access)
        : StatementNode(line_no), delete_token(dtoken), access_node(access)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const Token delete_token;
        const std::shared_ptr<ExpressionNode> access_node; // either member access or array access, validated by parser
    };

    class ClassDeclarationStatementNode : public StatementNode
    {
    public:
        ClassDeclarationStatementNode(const Token& ctoken, const std::shared_ptr<ClassDefinition>& cdef)
        : StatementNode(ctoken.position.line), class_token(ctoken), class_definition(cdef)
        {}

        ClassDeclarationStatementNode(const size_t line_no, const Token& ctoken, 
            const std::shared_ptr<ClassDefinition>& cdef)
        : StatementNode(line_no), class_token(ctoken), class_definition(cdef)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const Token class_token;
        const std::shared_ptr<ClassDefinition> class_definition;
    };

    class ExpressionStatementNode : public StatementNode
    {
    public:
        ExpressionStatementNode(const size_t line_no, const std::shared_ptr<ExpressionNode>& expr)
        : StatementNode(line_no), expression_node(expr)
        {}

        std::any Accept(IStatementVisitor& visitor) const override;
        std::string to_string() const override;

        const std::shared_ptr<ExpressionNode> expression_node;
    };

} // namespace mildew
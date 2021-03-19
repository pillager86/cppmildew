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
#include "nodes.h"

#include <sstream>

#include "visitors.h"

namespace mildew
{
    std::ostream& operator<<(std::ostream& os, const ExpressionNode& node)
    {
        os << node.to_string();
        return os;
    }

    std::any LiteralNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitLiteralNode(*this);
    }

    std::string LiteralNode::to_string() const
    {
        std::string result = literal_token.text;
        if(literal_token.type == Token::Type::STRING)
            result = '"' + result + '"';
        return result;
    }

    std::any FunctionLiteralNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitFunctionLiteralNode(*this);
    }

    std::string FunctionLiteralNode::to_string() const
    {
        std::string output = "function(";
        for(size_t i = 0; i < arg_list.size(); ++i)
        {
            output += arg_list[i];
            if(i < arg_list.size() - 1)
                output += ", ";
        }
        output += "){\n";
        for(const auto& stmt : statements)
            output += "\t" + stmt->to_string();
        output += "\n}";
        return output;
    }

    std::any LambdaNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitLambdaNode(*this);
    }

    std::string LambdaNode::to_string() const
    {
        std::string result = "(";
        for(size_t i = 0; i < argument_list.size(); ++i)
        {
            result += argument_list[i];
            if(i < argument_list.size() - 1)
                result += ", ";
        }
        result += ") => ";
        if(return_expression)
        {
            result += return_expression->to_string();
        }
        else 
        {
            result += '{';
            for(const auto& stmt : statements)
                result += stmt->to_string() + " ";
            result += '}';
        }
        return result;
    }

    std::any TemplateStringNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitTemplateStringNode(*this);
    }

    std::string TemplateStringNode::to_string() const
    {
        std::ostringstream ss;
        ss << '`';
        for(const auto& node : nodes)
        {
            if(const auto lit = std::dynamic_pointer_cast<LiteralNode>(node))
                ss << lit->literal_token.text;
            else
                ss << "${" << node->to_string() << '}';
        }
        ss << '`';
        return ss.str();
    }

    std::any ArrayLiteralNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitArrayLiteralNode(*this);
    }

    std::string ArrayLiteralNode::to_string() const
    {
        std::string result = "[";
        for(size_t i = 0; i < value_nodes.size(); ++i)
        {
            result += value_nodes[i]->to_string();
            if(i < value_nodes.size() - 1)
                result += ", ";
        }
        result += ']';
        return result;
    }

    std::any ObjectLiteralNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitObjectLiteralNode(*this);
    }

    std::string ObjectLiteralNode::to_string() const
    {
        if(keys.size() != value_nodes.size())
            return "<malformed object literal node>";
        std::string result = "{";
        for(size_t i = 0; i < keys.size(); ++i)
        {
            result += keys[i] + ':' + value_nodes[i]->to_string();
            if(i < keys.size() - 1)
                result += ", ";
        }
        result += '}';
        return result;
    }

    std::string ClassDefinition::to_string() const
    {
        std::string result = "class ";
        if(class_name != "")
            result += class_name + ' ';
        if(base_class)
            result += "extends " + base_class->to_string();
        result += "{ <class definition...> }";
        return result;
    }

    std::any ClassLiteralNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitClassLiteralNode(*this);
    }

    std::string ClassLiteralNode::to_string() const
    {
        return class_definition->to_string();
    }

    std::any BinaryOpNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitBinaryOpNode(*this);
    }

    std::string BinaryOpNode::to_string() const
    {
        return "(" + left_node->to_string() + op_token.Symbol() + right_node->to_string() + ")";
    }

    std::any UnaryOpNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitUnaryOpNode(*this);
    }

    std::string UnaryOpNode::to_string() const 
    {
        if(is_postfix)
            return "(" + operand_node->to_string() + op_token.Symbol() + ")";
        else 
            return "(" + op_token.Symbol() + operand_node->to_string() + ")";
    }

    std::any TerniaryOpNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitTerniaryOpNode(*this);
    }

    std::string TerniaryOpNode::to_string() const
    {
        return "(" + condition_node->to_string() + " ? " + on_true_node->to_string() + " : "
            + on_false_node->to_string() + ")";
    }

    std::any VarAccessNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitVarAccessNode(*this);
    }

    std::string VarAccessNode::to_string() const
    {
        return var_token.text;
    }

    std::any FunctionCallNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitFunctionCallNode(*this);
    }

    std::string FunctionCallNode::to_string() const
    {
        std::string result = function_to_call->to_string() + "(";
        for(size_t i = 0; i < argument_nodes.size(); ++i)
        {
            result += argument_nodes[i]->to_string();
            if(i < argument_nodes.size() - 1)
                result += ", ";
        }
        result += ')';
        return result;
    }

    std::any ArrayIndexNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitArrayIndexNode(*this);
    }

    std::string ArrayIndexNode::to_string() const
    {
        return object_node->to_string() + '[' + index_node->to_string() + ']';
    }

    std::any MemberAccessNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitMemberAccessNode(*this);
    }

    std::string MemberAccessNode::to_string() const
    {
        return object_node->to_string() + '.' + member_node->to_string();
    }

    std::any NewExpressionNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitNewExpressionNode(*this);
    }

    std::string NewExpressionNode::to_string() const
    {
        return "new " + function_call_node->to_string();
    }

    std::any SuperNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitSuperNode(*this);
    }

    std::string SuperNode::to_string() const
    {
        return "super";
    }

    std::any YieldNode::Accept(IExpressionVisitor& visitor) const
    {
        return visitor.VisitYieldNode(*this);
    }

    std::string YieldNode::to_string() const
    {
        return "yield " + (yield_expression_node ? yield_expression_node->to_string() : "");
    }
}
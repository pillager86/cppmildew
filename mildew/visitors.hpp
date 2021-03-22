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

#include "nodes.hpp"

namespace mildew
{
    class IExpressionVisitor
    {
    public:
        virtual ~IExpressionVisitor() {}

        virtual std::any VisitLiteralNode(const LiteralNode& lnode) = 0;
        virtual std::any VisitFunctionLiteralNode(const FunctionLiteralNode& flnode) = 0;
        virtual std::any VisitLambdaNode(const LambdaNode& lnode) = 0;
        virtual std::any VisitTemplateStringNode(const TemplateStringNode& tsnode) = 0;
        virtual std::any VisitArrayLiteralNode(const ArrayLiteralNode& alnode) = 0;
        virtual std::any VisitObjectLiteralNode(const ObjectLiteralNode& olnode) = 0;
        virtual std::any VisitClassLiteralNode(const ClassLiteralNode& clnode) = 0;
        virtual std::any VisitBinaryOpNode(const BinaryOpNode& bonode) = 0;
        virtual std::any VisitUnaryOpNode(const UnaryOpNode& uonode) = 0;
        virtual std::any VisitTerniaryOpNode(const TerniaryOpNode& tonode) = 0;
        virtual std::any VisitVarAccessNode(const VarAccessNode& vanode) = 0;
        virtual std::any VisitFunctionCallNode(const FunctionCallNode& fcnode) = 0;
        virtual std::any VisitArrayIndexNode(const ArrayIndexNode& ainode) = 0;
        virtual std::any VisitMemberAccessNode(const MemberAccessNode& manode) = 0;
        virtual std::any VisitNewExpressionNode(const NewExpressionNode& nenode) = 0;
        virtual std::any VisitSuperNode(const SuperNode& snode) = 0;
        virtual std::any VisitYieldNode(const YieldNode& ynode) = 0;
    };

    class IStatementVisitor
    {
    public:
        virtual ~IStatementVisitor() {}

        virtual std::any VisitVarDeclarationStatementNode(const VarDeclarationStatementNode& vdsnode) = 0;
        virtual std::any VisitBlockStatementNode(const BlockStatementNode& bsnode) = 0;
        virtual std::any VisitIfStatementNode(const IfStatementNode& isnode) = 0;
        virtual std::any VisitSwitchStatementNode(const SwitchStatementNode& ssnode) = 0;
        virtual std::any VisitWhileStatementNode(const WhileStatementNode& wsnode) = 0;
        virtual std::any VisitDoWhileStatementNode(const DoWhileStatementNode& dwsnode) = 0;
        virtual std::any VisitForStatementNode(const ForStatementNode& fsnode) = 0;
        virtual std::any VisitForOfStatementNode(const ForOfStatementNode& fosnode) = 0;
        virtual std::any VisitBreakOrContinueStatementNode(const BreakOrContinueStatementNode& bocsnode) = 0;
        virtual std::any VisitReturnStatementNode(const ReturnStatementNode& rsnode) = 0;
        virtual std::any VisitFunctionDeclarationStatementNode(const FunctionDeclarationStatementNode& fdsnode) = 0;
        virtual std::any VisitThrowStatementNode(const ThrowStatementNode& tsnode) = 0;
        virtual std::any VisitTryBlockStatementNode(const TryBlockStatementNode& tbsnode) = 0;
        virtual std::any VisitDeleteStatementNode(const DeleteStatementNode& dsnode) = 0;
        virtual std::any VisitClassDeclarationStatementNode(const ClassDeclarationStatementNode& cdsnode) = 0;
        virtual std::any VisitExpressionStatementNode(const ExpressionStatementNode& esnode) = 0;
    };
}
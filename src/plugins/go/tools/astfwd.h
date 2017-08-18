/****************************************************************************
**
** Copyright (C) 2016 Alexander Kudryavtsev.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

namespace GoTools {

enum PositionRelation { Before, Contain, After };

class LookupContext;
class PackageType;
class ResolveContext;

template <typename _Tp> class List;

class AST;
class ASTVisitor;

class FileAST;
class CommentAST;
class SpecAST;
class FieldAST;
class FieldGroupAST;
class ImportSpecAST;
class VarSpecWithTypeAST;
class VarSpecWithValuesAST;
class ConstSpecAST;
class TypeSpecAST;
class DeclAST;
class BadDeclAST;
class GenDeclAST;
class FuncDeclAST;
class ExprAST;
class RhsExprListAST;
class RangeExpAST;
class BadExprAST;
class ParenExprAST;
class IdentAST;
class DeclIdentAST;
class TypeIdentAST;
class PackageTypeAST;
class SelectorExprAST;
class IndexExprAST;
class SliceExprAST;
class TypeAssertExprAST;
class EllipsisAST;
class EllipsisTypeAST;
class CallExprAST;
class StarTypeAST;
class StarExprAST;
class NewExprAST;
class MakeExprAST;
class UnaryExprAST;
class ArrowUnaryExprAST;
class RefUnaryExprAST;
class BinaryExprAST;
class BinaryPlusExprAST;
class KeyValueExprAST;
class TypeAST;
class TupleTypeAST;
class BadTypeAST;
class ParenTypeAST;
class ArrayTypeAST;
class StructTypeAST;
class ChanTypeAST;
class FuncTypeAST;
class InterfaceTypeAST;
class MapTypeAST;
class LitAST;
class BasicLitAST;
class StringLitAST;
class FuncLitAST;
class CompositeLitAST;
class StmtAST;
class EmptyStmtAST;
class BadStmtAST;
class DeclStmtAST;
class LabeledStmtAST;
class ExprStmtAST;
class SendStmtAST;
class IncDecStmtAST;
class BlockStmtAST;
class AssignStmtAST;
class DefineStmtAST;
class GoStmtAST;
class DeferStmtAST;
class ReturnStmtAST;
class BranchStmtAST;
class IfStmtAST;
class CaseClauseAST;
class SwitchStmtAST;
class TypeSwitchStmtAST;
class CommClauseAST;
class SelectStmtAST;
class ForStmtAST;
class RangeStmtAST;

typedef List<CommentAST *> CommentGroupAST;
typedef List<IdentAST *> IdentListAST;
typedef List<DeclIdentAST *> DeclIdentListAST;
typedef List<ImportSpecAST *> ImportSpecListAST;
typedef List<DeclAST *> DeclListAST;
typedef List<SpecAST *> SpecListAST;
typedef List<ExprAST *> ExprListAST;
typedef List<TypeAST *> TypeListAST;
typedef List<FieldAST *> FieldListAST;
typedef List<StmtAST *> StmtListAST;

}   // namespace GoTools

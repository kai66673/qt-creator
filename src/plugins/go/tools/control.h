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

#include "diagnosticclient.h"

namespace GoTools {

class TranslationUnit;
class Comment;
class StringLiteral;
class Identifier;
class Scope;
class StructScope;
class FileScope;
class Type;
class TupleType;
class FuncDeclAST;
class FuncTypeAST;
class TypeIdentAST;
class TypeSpecAST;
class ExprAST;
class TypeAST;
class RhsExprListAST;
class RangeExpAST;
class Symbol;
class GoSource;

class Control
{
public:
    Control();
    ~Control();

    DiagnosticClient *diagnosticClient() const;
    void setDiagnosticClient(DiagnosticClient *diagnosticClient);

    TranslationUnit *translationUnit() const;
    TranslationUnit *switchTranslationUnit(TranslationUnit *unit);

    const Comment *comment(const char *chars, unsigned size);
    const StringLiteral *string(const char *chars, unsigned size);
    const Identifier *identifier(const char *chars, unsigned size);
    Scope *newScope(Scope *outer);
    Scope *newStructScope(Scope *outer);
    FileScope *newFileScope(GoSource *source);
    Symbol *newMethod(TypeIdentAST *recv, FuncDeclAST *funcDecl, Scope *owner);
    Symbol *newFuncDecl(unsigned tokenIndex, const Identifier *identifier, FuncTypeAST *decl, Scope *owner);
    Symbol *newVarDecl(unsigned tokenIndex, const Identifier *identifier, TypeAST *decl, Scope *owner);
    Symbol *newFieldDecl(unsigned tokenIndex, const Identifier *identifier, TypeAST *decl, Scope *owner);
    Symbol *newTypeDecl(unsigned tokenIndex, const Identifier *identifier, TypeSpecAST *decl, Scope *owner);
    Symbol *newConstDecl(unsigned tokenIndex, const Identifier *identifier, ExprAST *value, Scope *owner);
    Symbol *newShortVarDecl(unsigned tokenIndex, const Identifier *identifier, RhsExprListAST *decl, unsigned index, Scope *owner);
    Symbol *newRangeKeyDecl(unsigned tokenIndex, const Identifier *identifier, RangeExpAST *decl, Scope *owner);
    Symbol *newRangeValueDecl(unsigned tokenIndex, const Identifier *identifier, RangeExpAST *decl, Scope *owner);

    static const Identifier *dotIdentifier();
    static const Identifier *underscoreIdentifier();
    static const Identifier *builtinErrorIdentifier();

    static const Identifier *builtinIntIdentifier();
    static const Identifier *builtinByteIdentifier();
    static const Identifier *builtinInt8Identifier();
    static const Identifier *builtinRuneIdentifier();
    static const Identifier *builtinUintIdentifier();
    static const Identifier *builtinInt16Identifier();
    static const Identifier *builtinInt32Identifier();
    static const Identifier *builtinInt64Identifier();
    static const Identifier *builtinUint8Identifier();
    static const Identifier *builtinStringIdentifier();
    static const Identifier *builtinUint16Identifier();
    static const Identifier *builtinUint32Identifier();
    static const Identifier *builtinUint64Identifier();
    static const Identifier *builtinFloat32Identifier();
    static const Identifier *builtinFloat64Identifier();
    static const Identifier *builtinUintptrIdentifier();
    static const Identifier *builtinComplex64Identifier();
    static const Identifier *builtinComplex128Identifier();

    static Type *errorBuiltinType();
    static Type *voidType();
    static Type *unresolvedType();
    static Type *unresolvedTupleType();
    static Type *intBuiltinType();
    static Type *integralBuiltinType();
    static Type *stringBuiltingType();

    static const Identifier *newIdentifier();
    static const Identifier *makeIdentifier();

    static Symbol *errorErrorMethod();

private:
    class Data;
    friend class Data;
    Data *d;
};

}   // namespace GoTools

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

#include "lookupcontext.h"

namespace GoTools {

class TypeSpecAST;
class ExprType;

class NamedType
{
public:
    virtual ~NamedType() { }

    virtual const TypeSpecAST *typeSpec(ResolveContext *resolver) const = 0;
};

class Type: public LookupContext
{
public:
    virtual ~Type() { }

    virtual const Type *indexType(ResolveContext *resolver) const = 0;                // key:     S[K] -> type of K
    virtual const Type *elementsType(ResolveContext *resolver) const = 0;             // value:   S[K] -> type of S[...]
    virtual ExprType call(ResolveContext *) const;
    virtual const Type *chanValueType() const = 0;
    virtual const Type *derefType() const { return 0; }

    virtual const Type *baseType() const { return this; }
    virtual int refLevel() const { return 0; }
    virtual const Type *unstar() const { return 0; }
    virtual int countInTurple() const { return 1; }

    virtual QString describe() const  = 0;

    virtual const NamedType *asNamedType() const { return 0; }
    virtual bool isVoid() const { return false; }
    virtual bool isString(ResolveContext *resolver) const = 0;
    virtual bool isIntegral(ResolveContext *resolver) const = 0;
};

class BuiltinType: public Type
{
public:
    virtual ~BuiltinType() { }

    virtual Symbol *lookupMember(const IdentAST *, ResolveContext *) const override { return 0; }
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &,
                                       ResolveContext *, Predicate) const override { }

    virtual const Type *indexType(ResolveContext *) const override { return 0; }
    virtual const Type *elementsType(ResolveContext *) const override { return 0; }
    virtual const Type *chanValueType() const override { return 0; }
};

class VoidType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`void`"); }
    virtual bool isVoid() const override { return true; }
    virtual bool isString(ResolveContext *) const override { return false; }
    virtual bool isIntegral(ResolveContext *) const override { return false; }
};

class UnresolvedType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`Unresolved`"); }
    virtual bool isString(ResolveContext *) const override { return false; }
    virtual bool isIntegral(ResolveContext *) const override { return false; }
};

class UnresolvedTupleType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`Unresolved`"); }
    virtual bool isString(ResolveContext *) const override { return false; }
    virtual bool isIntegral(ResolveContext *) const override { return false; }
};

class IntBuiltinType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("int"); }
    virtual bool isString(ResolveContext *) const override { return false; }
    virtual bool isIntegral(ResolveContext *) const override { return true; }
};

class IntegralBuiltinType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`Integral`"); }
    virtual bool isString(ResolveContext *) const override { return false; }
    virtual bool isIntegral(ResolveContext *) const override { return true; }
};

class StringBuiltinType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("string"); }
    virtual bool isString(ResolveContext *) const override { return true; }
    virtual bool isIntegral(ResolveContext *) const override { return false; }
};

}   // namespace GoTools

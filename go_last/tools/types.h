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
class ResolvedType;

class NamedType
{
public:
    virtual ~NamedType() { }

    virtual const TypeSpecAST *typeSpec(ResolveContext *resolver) const = 0;
};

class Type: public LookupContext
{
public:
    enum BuiltingKind {
        String = 0,
        Integral,
        Void,
        Other
    };

    virtual ~Type() { }

    virtual ResolvedType indexType(ResolveContext *resolver, int refLvl = 0) const = 0;                // key:     S[K] -> type of K
    virtual ResolvedType elementsType(ResolveContext *resolver, int refLvl = 0) const = 0;             // value:   S[K] -> type of S[...]
    virtual ResolvedType callType(ResolveContext *, int = 0) const;
    virtual ResolvedType chanValueType(ResolveContext *, int refLvl = 0) const = 0;

    virtual int countInTurple() const { return 1; }
    virtual ResolvedType extractFromTuple(int index, int refLvl = 0) const;

    virtual QString describe() const  = 0;

    virtual const NamedType *asNamedType() const { return 0; }
    virtual BuiltingKind builtinKind(ResolveContext *resolver, int refLvl = 0) const = 0;
};

class BuiltinType: public Type
{
public:
    virtual ~BuiltinType() { }

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *, ResolveContext *, int) const override { return 0; }
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &,
                                       ResolveContext *, int, Predicate) const override { }

    virtual ResolvedType indexType(ResolveContext *, int = 0) const override;
    virtual ResolvedType elementsType(ResolveContext *, int = 0) const override;
    virtual ResolvedType chanValueType(ResolveContext *, int = 0) const override;
};

class ErrorType: public BuiltinType
{
public:
    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ident, ResolveContext *, int refLvl) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *, int refLvl, Predicate) const override;

    virtual QString describe() const override { return QLatin1String("error"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }
};

class VoidType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`void`"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int refLvl = 0) const override { return refLvl ? Other : Void; }
};

class UnresolvedType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`Unresolved`"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }
};

class UnresolvedTupleType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`Unresolved`"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }
};

class IntBuiltinType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("int"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int refLvl = 0) const override
    { return refLvl ? Other : Integral; }
};

class IntegralBuiltinType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("`Integral`"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int refLvl = 0) const override
    { return refLvl ? Other : Integral; }
};

class StringBuiltinType: public BuiltinType
{
public:
    virtual QString describe() const override { return QLatin1String("string"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int refLvl = 0) const override
    { return refLvl ? Other : String; }
};

}   // namespace GoTools

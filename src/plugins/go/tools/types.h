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

class TurpleType;

class Type: public LookupContext
{
public:
    virtual ~Type() { }

    virtual Type *indexType(ExprTypeResolver *resolver) = 0;                      // key:     S[K] -> type of K
    virtual Type *elementsType(ExprTypeResolver *resolver) = 0;                   // value:   S[K] -> type of S[...]
    virtual Type *calleeType(int index, ExprTypeResolver *resolver) const = 0;    // type of function call returns
    virtual Type *chanValueType() const = 0;
    virtual void fillTurple(TurpleType *turple, ExprTypeResolver *) const;
    virtual Type *derefType() const { return 0; }

    virtual Type *baseType() { return this; }
    virtual int refLevel() const { return 0; }

    virtual QString describe() const  = 0;
};

class BuiltinType: public Type
{
public:
    virtual ~BuiltinType() { }

    virtual Symbol *lookupMember(const IdentAST *, ExprTypeResolver *) override { return 0; }
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &,
                                       ExprTypeResolver *, Predicate) override { }

    virtual Type *indexType(ExprTypeResolver *) override { return 0; }
    virtual Type *elementsType(ExprTypeResolver *) override { return 0; }
    virtual Type *calleeType(int, ExprTypeResolver *) const override { return 0; }
    virtual Type *chanValueType() const override { return 0; }

    virtual QString describe() const override { return QLatin1String("builtin"); }
};

class TypeWithDerefLevel: public Type
{
public:
    TypeWithDerefLevel(int derefLevel, Type *type)
        : _derefLevel(derefLevel)
        , _baseType(type)
    { }

    virtual ~TypeWithDerefLevel() { }

    virtual Symbol *lookupMember(const IdentAST *ident, ExprTypeResolver *resolver) override
    {
        if (_baseType) {
            if (Type *baseTyp = _baseType->baseType()) {
                switch (_derefLevel + _baseType->refLevel()) {
                    case 0:
                    case -1:
                        return baseTyp->lookupMember(ident, resolver);
                }
            }
        }

        return 0;
    }

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate p = 0) override
    {
        if (_baseType) {
            if (Type *baseTyp = _baseType->baseType()) {
                switch (_derefLevel + _baseType->refLevel()) {
                    case 0:
                    case -1:
                        baseTyp->fillMemberCompletions(completions, resolver, p);
                        break;
                }
            }
        }
    }

    virtual Type *indexType(ExprTypeResolver *resolver) override
    {
        if (_baseType)
            if (_derefLevel + _baseType->refLevel() == 0)
                if (Type *typ = _baseType->baseType())
                    return typ->indexType(resolver);
        return 0;
    }

    virtual Type *elementsType(ExprTypeResolver *resolver) override
    {
        if (_baseType)
            if (_derefLevel + _baseType->refLevel() == 0)
                if (Type *typ = _baseType->baseType())
                    return typ->elementsType(resolver);
        return 0;
    }

    virtual Type *calleeType(int index, ExprTypeResolver *resolver) const override
    {
        if (_baseType)
            if (_derefLevel + _baseType->refLevel() == 0)
                if (Type *typ = _baseType->baseType())
                    return typ->calleeType(index, resolver);
        return 0;
    }

    virtual Type *chanValueType() const override
    {
        if (_baseType)
            if (_derefLevel + _baseType->refLevel() == 0)
                if (Type *typ = _baseType->baseType())
                    return typ->chanValueType();
        return 0;
    }

    virtual QString describe() const override
    {
        switch (_derefLevel) {
            case 0: return _baseType ? _baseType->describe() : QString();
            case -1: return _baseType ? QLatin1String("*") + _baseType->describe() : QString();
        }
        return QString();
    }

    virtual Type *baseType() override { return _baseType; }
    virtual int refLevel() const override { return _derefLevel; }

private:
    int _derefLevel;
    Type *_baseType;
};

class TurpleType
{
public:
    virtual ~TurpleType() { }

    Type *type(int index);
    void appendType(TypeWithDerefLevel *typ);

private:
    QList<TypeWithDerefLevel *> _types;
};

}   // namespace GoTools

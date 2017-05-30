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

#include "methodsscope.h"
#include "ast.h"

namespace GoTools {

MethodSymbol::MethodSymbol(const Identifier *id)
    : _index(0)
    , _next(0)
    , m_funcDecl(0)
    , m_identifier(id)
{ }

void MethodSymbol::setFuncDecl(FuncDeclAST *funcDecl)
{ m_funcDecl = funcDecl; }

FuncDeclAST *MethodSymbol::funcDecl() const
{ return m_funcDecl; }

TypeSymbol::TypeSymbol(const Identifier *identifier)
    : _index(0)
    , _next(0)
    , m_identifier(identifier)
{ }

void TypeSymbol::addMethod(FuncDeclAST *funcDecl)
{ m_methods.enterSymbol(funcDecl->name->ident)->setFuncDecl(funcDecl); }

MethodSymbol *TypeSymbol::lookup(const Identifier *funcId)
{ return m_methods.lookat(funcId); }

QStringList TypeSymbol::typeMethods()
{
    QStringList result;
    for (unsigned i = 0; i < m_methods.symbolCount(); i++) {
        if (FuncDeclAST *func = m_methods.symbolAt(i)->funcDecl()) {
            if (DeclIdentAST *funcIdent = func->name)
                result << funcIdent->ident->toString();
        }
    }
    return result;
}

void MethodsScope::addMethod(const Identifier *typeId, FuncDeclAST *funcDecl)
{ m_typeToMethods.enterSymbol(typeId)->addMethod(funcDecl); }

FuncDeclAST *MethodsScope::lookup(const Identifier *typeId, const Identifier *funcId)
{
    if (TypeSymbol *typeSymbol = m_typeToMethods.lookat(typeId))
        if (MethodSymbol *methodSymbol = typeSymbol->lookup(funcId))
            return methodSymbol->funcDecl();
    return 0;
}

QStringList MethodsScope::typeMethods(const Identifier *typeId)
{
    if (TypeSymbol *typeSymbol = m_typeToMethods.lookat(typeId))
        return typeSymbol->typeMethods();

    return QStringList();
}

}   // namespave GoTools

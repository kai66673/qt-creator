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

#include "scopevisitor.h"

#include <texteditor/texteditor.h>

#include <stack>

namespace GoTools {

class SymbolUnderCursor: public ScopePositionVisitor
{
public:
    SymbolUnderCursor(GoSource::Ptr doc, bool protectCache = true);

protected:
    virtual bool visit(StructTypeAST *ast);
    virtual void endVisit(StructTypeAST *);

    virtual bool visit(DeclIdentAST *ast);
    virtual bool visit(IdentAST *ast);
    virtual bool visit(TypeIdentAST *ast);
    virtual bool visit(PackageTypeAST *ast);
    virtual bool visit(SelectorExprAST *ast);

    virtual bool visit(CompositeLitAST *ast);
    virtual bool visit(KeyValueExprAST *ast);

protected:
    GoSource::Ptr m_doc;

    std::stack<const Type *> m_nestedCompositLitType;
    std::stack<const StructTypeAST *> m_structures;

    Symbol *m_symbol;
    const Token *m_token;
};

}   // namespace GoTools

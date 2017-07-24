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
#include "ast.h"
#include "packagetype.h"
#include "symbolundercursordescriber.h"

namespace GoTools {

SymbolUnderCursorDescriber::SymbolUnderCursorDescriber(GoSource::Ptr doc, bool protectCache)
    : SymbolUnderCursor(doc, protectCache)
{ }

QString SymbolUnderCursorDescriber::description(unsigned pos)
{
    m_pos = pos;
    defineDescription();
    return m_symbolTypeDescription;
}

bool SymbolUnderCursorDescriber::visit(ImportSpecAST *ast)
{
    if (ast->t_path) {
        unsigned startPos = ast->name ? ast->name->t_identifier : ast->t_path;
        const Token &firstToken = _tokens->at(startPos);
        const Token &lastToken = _tokens->at(ast->t_path);

        if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
            if (ast->name) {
                m_packageAlias = ast->name->ident->toString();
            } else {
                QString path = lastToken.string->unquoted();
                if (!path.isEmpty()) {
                    int pos = path.lastIndexOf('/') + 1;
                    m_packageAlias = path.mid(pos);
                }
            }
            _traverseFinished = true;
        }

        else if (m_pos <= lastToken.end())
            _traverseFinished = true;
    }

    return false;
}

bool SymbolUnderCursorDescriber::visit(IdentAST *ast)
{
    const Token &tk = _tokens->at(ast->t_identifier);
    m_token = &tk;

    if (m_pos >= tk.begin() && m_pos <= tk.end()) {
        _traverseFinished = true;
        if (ast->isLookable()) {
            m_symbol = m_currentScope->lookupMember(ast, this);
            if (!m_symbol) { // may be a package
                QString packageAlias(ast->ident->toString());
                if (packageTypeForAlias(packageAlias))
                    m_packageAlias = packageAlias;
            }
        }
    }

    else if (m_pos <= tk.end())
        _traverseFinished = true;

    return false;
}

bool SymbolUnderCursorDescriber::visit(PackageTypeAST *ast)
{
    const Token &packageToken = _tokens->at(ast->packageAlias->t_identifier);
    if (m_pos >= packageToken.begin() && m_pos <= packageToken.end()) {
        _traverseFinished = true;
        QString packageAlias(ast->packageAlias->ident->toString());
        if (ast->packageAlias->isLookable())
            if (packageTypeForAlias(packageAlias))
                m_packageAlias = packageAlias;
        return false;
    }

    const Token &typeToken = _tokens->at(ast->typeName->t_identifier);
    m_token = &typeToken;

    if (m_pos >= typeToken.begin() && m_pos <= typeToken.end()) {
        _traverseFinished = true;
        if (ast->typeName->isLookable()) {
            QString packageAlias(ast->packageAlias->ident->toString());
            if (PackageType *context = packageTypeForAlias(packageAlias))
                m_symbol = context->lookupMember(ast->typeName, this);
        }
    }

    else if (m_pos <= typeToken.end())
        _traverseFinished = true;

    return false;
}

void SymbolUnderCursorDescriber::defineDescription()
{
    m_symbol = 0;
    m_token = 0;
    m_symbolTypeDescription.clear();
    m_packageAlias.clear();

    if (m_doc && isValidResolveContext()) {
        acceptForPosition(m_initialFileAst->importDecls);
        if (!_traverseFinished)
            acceptForPosition(m_initialFileAst->decls);

        if (m_symbol) {
            m_symbolTypeDescription = m_symbol->describeType(this);
        } else if (!m_packageAlias.isEmpty()) {
            for (const GoSource::Import &import: m_doc->imports()) {
                if (import.alias == m_packageAlias) {
                    m_symbolTypeDescription = QLatin1String("package: ") + import.resolvedDir;
                    break;
                }
            }
        }
    }
}

}   // namespace GoTools

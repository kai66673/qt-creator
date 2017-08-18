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
#include "gocompletionassistvisitor.h"
#include "packagetype.h"
#include "ast.h"

namespace GoTools {

GoCompletionAssistVisitor::GoCompletionAssistVisitor(GoSource::Ptr doc, QList<TextEditor::AssistProposalItemInterface *> &completions)
    : ScopePositionVisitor(doc.data())
    , m_doc(doc)
    , m_completions(completions)
{ }

void GoCompletionAssistVisitor::fillCompletions(bool isGlobalCompletion, unsigned pos)
{
    m_isGlobalCompletion = isGlobalCompletion;
    m_pos = pos;

    if (m_doc && isValidResolveContext()) {
        /// TODO: imports autocompletions
        m_inImportSection = false;
        if (m_initialFileAst->importDecls) {
            const Token &firstToken = _tokens->at(m_initialFileAst->importDecls->firstToken());
            const Token &lastToken = _tokens->at(m_initialFileAst->importDecls->lastToken());
            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                m_inImportSection = true;
                return;
            }
        }

        for (DeclListAST *it = m_initialFileAst->decls; it; it = it->next) {
            const Token &firstToken = _tokens->at(it->value->firstToken());
            const Token &lastToken = _tokens->at(it->value->lastToken());
            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                accept(it->value);
                if (m_isGlobalCompletion) {
                    m_currentScope->fillMemberCompletions(m_completions, this, 0,
                                                    [this](Symbol *s) -> bool {
                                                        if (s->declExpr())
                                                            return _tokens->at(s->declExpr()->lastToken()).end() <= m_pos;
                                                        return _tokens->at(s->sourceLocation()).end() <= m_pos;
                                                    });
                }
                break;
            }

            if (m_pos <= firstToken.begin()) {
                break;
            }
        }
    }
}

bool GoCompletionAssistVisitor::visit(CompositeLitAST *ast)
{
    if (m_isGlobalCompletion) {
        if (ast->elements) {
            const Token &firstToken = _tokens->at(ast->elements->firstToken());
            const Token &lastToken = _tokens->at(ast->elements->lastToken());
            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                const Type *type = 0;
                if (ExprAST *typeExpr = ast->type) {
                    type = typeExpr->asType();
                    if (!type) {
                        type = tryResolveNamedType(this, typeExpr);
                    }
                } else if (!m_nestedCimpositLitType.empty()) {
                    type = m_nestedCimpositLitType.top();
                    if (type)
                        type = type->elementsType(this).type();
                }

                m_nestedCimpositLitType.push(type);
                if (type) {
                    type->fillMemberCompletions(m_completions, this);
                }
                accept(ast->elements);
                m_nestedCimpositLitType.pop();
                return false;
            }
        }
    }

    return true;
}

bool GoCompletionAssistVisitor::visit(DeclIdentAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->t_identifier);

        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            _traverseFinished = true;
            if (ast->isLookable())
                ast->resolve(this).fillMemberCompletions(m_completions, this);
        }

        else if (m_pos <= tk.end())
            _traverseFinished = true;
    }

    return false;
}

bool GoCompletionAssistVisitor::visit(IdentAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->t_identifier);

        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            _traverseFinished = true;
            if (ast->isLookable())
                ast->resolve(this).fillMemberCompletions(m_completions, this);
        }

        else if (m_pos <= tk.end())
            _traverseFinished = true;
    }

    return false;
}

bool GoCompletionAssistVisitor::visit(PackageTypeAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->packageAlias->t_identifier);

        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            _traverseFinished = true;
            QString packageAlias(ast->packageAlias->ident->toString());
            if (const PackageType *context = packageTypeForAlias(packageAlias))
                context->fillMemberCompletions(m_completions, this);
        }

        else if (m_pos <= tk.end())
            _traverseFinished = true;
    }

    return false;
}

bool GoCompletionAssistVisitor::visit(SelectorExprAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->x->lastToken());
        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            _traverseFinished = true;
            ast->x->resolve(this).fillMemberCompletions(m_completions, this);
            return false;
        } else if (ast->sel) {
            const Token &tk = _tokens->at(ast->sel->lastToken());
            if (m_pos >= tk.begin() && m_pos <= tk.end()) {
                _traverseFinished = true;
                ast->resolve(this).fillMemberCompletions(m_completions, this);
                return false;
            }
        }
        return true;
    }

    return false;
}

bool GoCompletionAssistVisitor::inImportSection() const
{ return m_inImportSection; }

}   // namespace GoTools

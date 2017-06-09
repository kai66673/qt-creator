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
#include "ast.h"
#include "gosnapshot.h"

namespace GoTools {

GoCompletionAssistVisitor::GoCompletionAssistVisitor(GoSource::Ptr doc, QList<TextEditor::AssistProposalItemInterface *> &completions)
    : ScopePositionVisitor(doc)
    , m_doc(doc)
    , m_completions(completions)
{ }

void GoCompletionAssistVisitor::fillCompletions(bool isGlobalCompletion, unsigned pos)
{
    m_isGlobalCompletion = isGlobalCompletion;
    m_pos = pos;

    if (m_doc->translationUnit() && m_snapshot) {
        m_snapshot->runProtectedTask(
            [this]() -> void {
                if (FileAST *fileAst = m_doc->translationUnit()->fileAst()) {
                    m_currentScope = fileAst->scope;
                    m_currentIndex = fileAst->scope->indexInSnapshot();
                    if (m_currentIndex != -1) {
                        m_ended = false;

                        /// TODO: imports autocompletions
                        m_inImportSection = false;
                        if (fileAst->importDecls) {
                            const Token &firstToken = _tokens->at(fileAst->importDecls->firstToken());
                            const Token &lastToken = _tokens->at(fileAst->importDecls->lastToken());
                            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                                m_inImportSection = true;
                                return;
                            }
                        }

                        for (DeclListAST *it = fileAst->decls; it; it = it->next) {
                            const Token &firstToken = _tokens->at(it->value->firstToken());
                            const Token &lastToken = _tokens->at(it->value->lastToken());
                            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                                accept(it->value);
                                if (m_isGlobalCompletion) {
                                    m_currentScope->fillMemberCompletions(m_completions, this,
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

                        eraseResolvedTypes();
                    }
                }
            }
        );
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
                        type = resolveCompositExpr(ast);
                    }
                } else if (!m_nestedCimpositLitType.empty()) {
                    type = m_nestedCimpositLitType.top();
                    if (type)
                        type = type->elementsType(this);
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
            m_ended = true;
            if (ast->isLookable()) {
                int derefLevel = 0;
                if (const Type *type = resolveExpr(ast, derefLevel)) {
                    derefLevel += type->refLevel();
                    if (derefLevel == 0 || derefLevel == -1) {
                        if (const Type *baseTyp = type->baseType()) {
                            baseTyp->fillMemberCompletions(m_completions, this);
                        }
                    }
                }
            }
        }

        else if (m_pos <= tk.end())
            m_ended = true;
    }

    return false;
}

bool GoCompletionAssistVisitor::visit(IdentAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->t_identifier);

        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            m_ended = true;
            if (ast->isLookable()) {
                int derefLevel = 0;
                if (const Type *type = resolveExpr(ast, derefLevel)) {
                    derefLevel += type->refLevel();
                    if (derefLevel == 0 || derefLevel == -1) {
                        if (const Type *baseTyp = type->baseType()) {
                            baseTyp->fillMemberCompletions(m_completions, this);
                        }
                    }
                }
            }
        }

        else if (m_pos <= tk.end())
            m_ended = true;
    }

    return false;
}

bool GoCompletionAssistVisitor::visit(PackageTypeAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->packageAlias->t_identifier);

        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            m_ended = true;
            QString packageAlias(ast->packageAlias->ident->toString());
            if (PackageType *context = m_snapshot->packageTypeForAlias(m_currentIndex, packageAlias))
                context->fillMemberCompletions(m_completions, this);
        }

        else if (m_pos <= tk.end())
            m_ended = true;
    }

    return false;
}

bool GoCompletionAssistVisitor::visit(SelectorExprAST *ast)
{
    if (!m_isGlobalCompletion) {
        const Token &tk = _tokens->at(ast->x->lastToken());
        if (m_pos >= tk.begin() && m_pos <= tk.end()) {
            m_ended = true;
            int derefLevel = 0;
            if (const Type *type = resolveExpr(ast->x, derefLevel)) {
                derefLevel += type->refLevel();
                if (derefLevel == 0 || derefLevel == -1) {
                    if (const Type *baseTyp = type->baseType()) {
                        baseTyp->fillMemberCompletions(m_completions, this);
                    }
                }
            }
            return false;
        } else if (ast->sel) {
            const Token &tk = _tokens->at(ast->sel->lastToken());
            if (m_pos >= tk.begin() && m_pos <= tk.end()) {
                m_ended = true;
                int derefLevel = 0;
                if (const Type *type = resolveExpr(ast, derefLevel)) {
                    derefLevel += type->refLevel();
                    if (derefLevel == 0 || derefLevel == -1) {
                        if (const Type *baseTyp = type->baseType()) {
                            baseTyp->fillMemberCompletions(m_completions, this);
                        }
                    }
                }
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

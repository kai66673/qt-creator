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
#include "gofunctionhintassistvisitor.h"
#include "ast.h"
#include "gosnapshot.h"

namespace GoTools {

GoFunctionHintAssistVisitor::GoFunctionHintAssistVisitor(GoSource::Ptr doc)
    : ASTVisitor(doc->translationUnit())
    , m_doc(doc)
{
    _tokens = translationUnit()->tokens();
}

QStringList GoFunctionHintAssistVisitor::functionArguments(unsigned pos)
{
    m_type = 0;
    m_pos = pos;
    m_functionArgs.clear();

    if (m_doc->translationUnit() && m_snapshot) {
        m_snapshot->runProtectedTask(
            [this]() -> void {
                if (FileAST *fileAst = m_doc->translationUnit()->fileAst()) {
                    m_currentScope = fileAst->scope;
                    m_currentIndex = fileAst->scope->indexInSnapshot();
                    if (m_currentIndex != -1) {
                        m_ended = false;

                        for (DeclListAST *it = fileAst->decls; it; it = it->next) {
                            const Token &firstToken = _tokens->at(it->value->firstToken());
                            const Token &lastToken = _tokens->at(it->value->lastToken());
                            if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
                                accept(it->value);
                                break;
                            }

                            if (m_pos <= firstToken.begin()) {
                                break;
                            }
                        }

                        eraseResolvedTypes();

                        if (FuncTypeAST *type = dynamic_cast<FuncTypeAST *>(m_type)) {
                            if (type->params) {
                                for (FieldListAST *field_it = type->params->fields; field_it; field_it = field_it->next) {
                                    if (FieldAST *field = field_it->value) {
                                        QString typeDescr = field->type ? field->type->describe() : "";
                                        if (DeclIdentListAST *name_it = field->names) {
                                            for (; name_it; name_it = name_it->next) {
                                                QString argName;
                                                if (DeclIdentAST *name = name_it->value)
                                                    argName = name->ident->toString();
                                                QString argDescr(argName + QLatin1String(" ") + typeDescr);
                                                m_functionArgs << argDescr.trimmed();
                                            }
                                        } else if (!typeDescr.isEmpty()) {
                                            m_functionArgs << typeDescr;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        );
    }

    return m_functionArgs;
}

bool GoFunctionHintAssistVisitor::preVisit(AST *)
{ return !m_ended; }

bool GoFunctionHintAssistVisitor::visit(FuncDeclAST *ast)
{
    if (ast->body) {
        const Token &firstToken = _tokens->at(ast->firstToken());
        const Token &lastToken = _tokens->at(ast->lastToken());
        if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
            switchScope(ast->scope);
            accept(ast->body);
        }
        else if (m_pos <= firstToken.begin()) {
            m_ended = true;
        }
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(BlockStmtAST *ast)
{
    switchScope(ast->scope);
    for (StmtListAST *it = ast->list; it; it = it->next) {
        const Token &firstToken = _tokens->at(it->value->firstToken());
        const Token &lastToken = _tokens->at(it->value->lastToken());
        if (m_pos >= firstToken.begin() && m_pos <= lastToken.end()) {
            accept(it->value);
            break;
        }
        if (m_pos <= firstToken.begin()) {
            m_ended = true;
            break;
        }
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(IfStmtAST *ast)
{
    accept(ast->init);
    Scope *scope = switchScope(ast->scope);
    if (!m_ended) {
        accept(ast->cond);
        if (!m_ended) {
            accept(ast->body);
            if (!m_ended) {
                switchScope(scope); // ?? what about init of if-statement
                accept(ast->elseStmt);
            }
        }
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(RangeStmtAST *ast)
{
    accept(ast->key);
    if (!m_ended) {
        accept(ast->value);
        if (!m_ended) {
            accept(ast->x);
            if (!m_ended) {
                switchScope(ast->scope);
                accept(ast->body);
            }
        }
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(ForStmtAST *ast)
{
    accept(ast->init);
    if (!m_ended) {
        switchScope(ast->scope);
        accept(ast->cond);
        if (!m_ended) {
            accept(ast->post);
            if (!m_ended)
                accept(ast->body);
        }
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(TypeSwitchStmtAST *ast)
{
    accept(ast->init);
    if (!m_ended) {
        switchScope(ast->scope);
        accept(ast->assign);
        if (!m_ended)
            accept(ast->body);
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(SwitchStmtAST *ast)
{
    accept(ast->init);
    if (!m_ended) {
        accept(ast->tag);
        switchScope(ast->scope);
        if (!m_ended)
            accept(ast->body);
    }

    return false;
}

bool GoFunctionHintAssistVisitor::visit(CaseClauseAST *ast)
{
    accept(ast->list);
    switchScope(ast->scope);
    if (!m_ended)
        accept(ast->body);

    return false;
}

bool GoFunctionHintAssistVisitor::visit(CallExprAST *ast)
{
    unsigned lparen = ast->t_lparen;
    if (lparen) {
        const Token &tk = _tokens->at(lparen);
        if (m_pos == tk.begin()) {
            m_ended = true;
            int derefLevel = 0;
            m_type = resolveExpr(ast->fun, derefLevel);
        }
    }

    return false;
}

}   // namespace GoTools

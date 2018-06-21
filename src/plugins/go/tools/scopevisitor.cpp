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
#include "scopevisitor.h"
#include "ast.h"

namespace GoTools {

ScopeSwitchVisitor::ScopeSwitchVisitor(GoSource *source, bool protectCache)
    : ASTVisitor(source->translationUnit())
    , ResolveContext(source, protectCache)
{ }

bool ScopeSwitchVisitor::visit(FuncDeclAST *ast)
{
    accept(ast->recv);
    accept(ast->name);
    accept(ast->type);

    Scope *scope = switchScope(ast->scope);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(BlockStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(IfStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->cond);
    accept(ast->body);
    accept(ast->elseStmt);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(RangeStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->key);
    accept(ast->value);
    accept(ast->x);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(ForStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->cond);
    accept(ast->post);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(TypeSwitchStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->assign);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(SwitchStmtAST *ast)
{
    accept(ast->init);

    Scope *scope = switchScope(ast->scope);
    accept(ast->tag);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool ScopeSwitchVisitor::visit(CaseClauseAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    accept(ast->body);
    switchScope(scope);

    return false;
}

ScopePositionVisitor::ScopePositionVisitor(GoSource *source, bool protectCache)
    : ASTVisitor(source->translationUnit())
    , ResolveContext(source, protectCache)
{ }

bool ScopePositionVisitor::visit(FuncDeclAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->recv);
            accept(ast->name);
            accept(ast->type);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->body);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

bool ScopePositionVisitor::visit(BlockStmtAST *ast)
{
    switchScope(ast->scope);
    for (StmtListAST *it = ast->list; it; it = it->next) {
        switch (it->value->positionRelation(m_pos, _tokens)) {
            case Contain:
                accept(it->value);
                _traverseFinished = true;
                return false;
            case Before:
                _traverseFinished = true;
                return false;
            case After:
                break;
        }
    }

    return false;
}

bool ScopePositionVisitor::visit(IfStmtAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->init);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->cond);
                accept(ast->body);
                accept(ast->elseStmt);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

bool ScopePositionVisitor::visit(RangeStmtAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->key);
            accept(ast->value);
            accept(ast->x);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->body);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

bool ScopePositionVisitor::visit(ForStmtAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->init);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->cond);
                accept(ast->post);
                accept(ast->body);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

bool ScopePositionVisitor::visit(TypeSwitchStmtAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->init);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->assign);
                accept(ast->body);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

bool ScopePositionVisitor::visit(SwitchStmtAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->init);
            accept(ast->tag);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->body);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

bool ScopePositionVisitor::visit(CaseClauseAST *ast)
{
    switch (ast->positionRelation(m_pos, _tokens)) {
        case Contain:
            accept(ast->list);
            if (!_traverseFinished) {
                switchScope(ast->scope);
                accept(ast->body);
            }
            _traverseFinished = true;
            break;
        case Before:
            _traverseFinished = true;
            break;
        case After:
            break;
    }

    return false;
}

}   // namespace GoTools

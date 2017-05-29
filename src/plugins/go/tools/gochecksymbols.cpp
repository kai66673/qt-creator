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
#include "gochecksymbols.h"
#include "ast.h"
#include "control.h"

#include <utils/algorithm.h>
#include <utils/qtcassert.h>

#include <QDateTime>

namespace GoTools {

QMutex g_semanticMutex;

static GoSemanticHighlighter::Kind kindForSymbol(const Symbol *symbol)
{
    switch (symbol->kind()) {
        case Symbol::Pkg: return GoSemanticHighlighter::Package;
        case Symbol::Con: return GoSemanticHighlighter::Const;
        case Symbol::Typ: return GoSemanticHighlighter::Type;
        case Symbol::Var: return GoSemanticHighlighter::Var;
        case Symbol::Fun: return GoSemanticHighlighter::Func;
        case Symbol::Lbl: return GoSemanticHighlighter::Label;
        default: break; // prevent -Wswitch warning
    }

    return GoSemanticHighlighter::Var;
}

GoCheckSymbols::GoCheckSymbols(GoSource::Ptr doc)
    : ASTVisitor(doc->translationUnit())
    , ExprTypeResolver()
    , m_doc(doc)
    , m_control(doc->translationUnit()->control())
    , _chunkSize(50)
{
    _tokens = translationUnit()->tokens();
    _usages.reserve(_chunkSize);
}

void GoCheckSymbols::run()
{
    if (!isCanceled()) {
        if (m_doc->translationUnit() && !m_doc->isTooBig() && m_snapshot) {
            m_snapshot->runProtectedTask(
                [this]() -> void {
                    if (FileAST *fileAst = m_doc->translationUnit()->fileAst()) {
                        m_currentScope = fileAst->scope;
                        m_currentIndex = fileAst->scope->indexInSnapshot();
                        if (m_currentIndex != -1) {
                            if (IdentAST *packageName = fileAst->packageName)
                            addUseCheckFirstLine(packageName, GoSemanticHighlighter::Package);
                            accept(fileAst->importDecls);
                            accept(fileAst->decls);
                            flush();
                            eraseResolvedTypes();
                        }
                    }
                }
            );
        }
    }

    reportFinished();
}

static bool sortByLinePredicate(const GoCheckSymbols::Result &lhs, const GoCheckSymbols::Result &rhs)
{
    if (lhs.line == rhs.line)
        return lhs.column < rhs.column;
    else
        return lhs.line < rhs.line;
}

void GoCheckSymbols::flush()
{
    Utils::sort(_usages, sortByLinePredicate);
    reportResults(_usages);
    int cap = _usages.capacity();
    _usages.clear();
    _usages.reserve(cap);
}

void GoCheckSymbols::addUse(IdentAST *ast, GoCheckSymbols::Kind kind)
{
    if (!ast->ident || ast->ident == Control::underscoreIdentifier())
        return;

    const Token &tok = _tokens->at(ast->t_identifier);
    _usages.append(Result(tok.line(), tok.column(), tok.length(), kind));
}

void GoCheckSymbols::addUseCheckFirstLine(IdentAST *ast, GoCheckSymbols::Kind kind)
{
    if (!ast->ident || ast->ident == Control::underscoreIdentifier())
        return;

    const Token &tok = _tokens->at(ast->t_identifier);
    unsigned line = tok.line();
    unsigned column = tok.column();
    if (line == 1)
        column++;
    _usages.append(Result(line, column, tok.length(), kind));
}

bool GoCheckSymbols::preVisit(AST *)
{ return !isCanceled(); }

bool GoCheckSymbols::visit(ImportSpecAST *ast)
{
    if (ast->name) {
        addUse(ast->name, GoSemanticHighlighter::Package);
    } else if (ast->t_path) {
        const Token &tk = _tokens->at(ast->t_path);
        QString path = tk.string->unquoted();
        if (!path.isEmpty()) {
            int pos = path.lastIndexOf('/') + 1;
            _usages.append(Result(tk.line(), tk.column() + pos + 1, path.length() - pos, GoSemanticHighlighter::Package));
        }
    }
    return false;
}

bool GoCheckSymbols::visit(TypeSpecAST *ast)
{
    addUse(ast->name, GoSemanticHighlighter::TypeDecl);
    accept(ast->type);
    return false;
}

bool GoCheckSymbols::visit(VarSpecWithTypeAST *ast)
{
    for (DeclIdentListAST *it = ast->names; it; it = it->next)
        if (DeclIdentAST *ident = it->value)
            addUse(ident, GoSemanticHighlighter::VarDecl);

    accept(ast->type);
    return false;
}

bool GoCheckSymbols::visit(VarSpecWithValuesAST *ast)
{
    for (DeclIdentListAST *it = ast->names; it; it = it->next)
        if (DeclIdentAST *ident = it->value)
            addUse(ident, GoSemanticHighlighter::VarDecl);

    /// TODO: prevent double resolving
    accept(ast->values);
    return false;
}

bool GoCheckSymbols::visit(ConstSpecAST *ast)
{
    for (DeclIdentListAST *it = ast->names; it; it = it->next)
        if (DeclIdentAST *ident = it->value)
            addUse(ident, GoSemanticHighlighter::ConstDecl);

    accept(ast->type);
    accept(ast->values);
    return false;
}

bool GoCheckSymbols::visit(FuncDeclAST *ast)
{
    _isFuncDecl = true;
    accept(ast->recv);
    _isFuncDecl = false;
    addUse(ast->name, GoSemanticHighlighter::FuncDecl);
    accept(ast->type);

    Scope *scope = switchScope(ast->scope);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(FuncTypeAST *ast)
{
    _isFuncDecl = true;
    accept(ast->params);
    accept(ast->results);
    _isFuncDecl = false;

    return false;
}

bool GoCheckSymbols::visit(BlockStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(IfStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    accept(ast->cond);
    accept(ast->body);
    accept(ast->elseStmt);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(RangeStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->key);
    accept(ast->value);
    accept(ast->x);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(ForStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    accept(ast->cond);
    accept(ast->post);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(TypeSwitchStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    accept(ast->assign);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(SwitchStmtAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->init);
    accept(ast->tag);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(CaseClauseAST *ast)
{
    Scope *scope = switchScope(ast->scope);
    accept(ast->list);
    accept(ast->body);
    switchScope(scope);

    return false;
}

bool GoCheckSymbols::visit(FieldAST *ast)
{
    for (DeclIdentListAST *it = ast->names; it; it = it->next)
        if (DeclIdentAST *ident = it->value)
            addUse(ident, _isFuncDecl ? GoSemanticHighlighter::Arg
                                      : (ident->symbol && ident->symbol->kind() == Symbol::Fun
                                         ? GoSemanticHighlighter::Func
                                         : GoSemanticHighlighter::Field));

    accept(ast->type);
    return false;
}

Type *GoCheckSymbols::resolveNamedType(TypeIdentAST *ast)
{
    if (IdentAST *ident = ast->ident) {
        if (ident->isLookable()) {
            if (Symbol *symbol = m_currentScope->lookupMember(ident, this)) {
                if (symbol->kind() == Symbol::Typ) {
                    Type *resolvedType = symbol->type(this);
                    addUse(ident, GoSemanticHighlighter::Type);
                    return resolvedType;
                }
            }
        }
    }

    return 0;
}

bool GoCheckSymbols::visit(TypeIdentAST *ast)
{
    resolveNamedType(ast);
    return false;
}

Type *GoCheckSymbols::resolveNamedType(PackageTypeAST *ast)
{
    QLatin1String packageAlias(ast->packageAlias->ident->toLatin1());
    PackageType *context = m_snapshot->packageTypeForAlias(m_currentIndex, packageAlias);
    if (!context)
        return 0;

    addUse(ast->packageAlias, GoSemanticHighlighter::Package);

    if (ast->typeName->isLookable()) {
        if (Symbol *symbol = context->lookupMember(ast->typeName, this))
            if (symbol->kind() == Symbol::Typ) {
                Type *resolvedType = symbol->type(this);
                addUse(ast->typeName, GoSemanticHighlighter::Type);
                return resolvedType;
            }
    }

    return 0;
}

bool GoCheckSymbols::visit(PackageTypeAST *ast)
{
    resolveNamedType(ast);
    return false;
}

bool GoCheckSymbols::visit(SelectorExprAST *ast)
{
    bool isFieldOrMethod = false;
    int derefLevel = 0;
    Type *context = resolveSelectorExpr(ast->x, isFieldOrMethod, derefLevel);
    if (context && ast->sel && ast->sel->isLookable()) {
        derefLevel += context->refLevel();
        if (derefLevel == 0 || derefLevel == -1) {
            if (Type *baseTyp = context->baseType()) {
                if (Symbol *s = baseTyp->lookupMember(ast->sel, this)) {
                    addUse(ast->sel, isFieldOrMethod
                                     ? (s->kind() == Symbol::Fun
                                        ? GoSemanticHighlighter::Func
                                        : GoSemanticHighlighter::Field)
                                     : kindForSymbol(s));
                }
            }
        }
    }

    return false;
}

Type *GoCheckSymbols::acceptCompositLiteral(CompositeLitAST *ast)
{
    Type *type = 0;
    if (ExprAST *typeExpr = ast->type) {
        type = typeExpr->asType();
        if (!type) {
            type = resolveCompositExprType(ast);
        } else {
            accept(ast->type);
        }
    } else if (!m_nestedCimpositLitType.empty()) {
        type = m_nestedCimpositLitType.top();
        if (type)
            type = type->elementsType(this);
    }

    m_nestedCimpositLitType.push(type);
    accept(ast->elements);
    m_nestedCimpositLitType.pop();

    return type;
}

bool GoCheckSymbols::visit(CompositeLitAST *ast)
{
    acceptCompositLiteral(ast);
    return false;
}

bool GoCheckSymbols::visit(KeyValueExprAST *ast)
{
    Type *elementsType = m_nestedCimpositLitType.empty() ? 0 : m_nestedCimpositLitType.top();
    if (elementsType && ast->key) {
        if (IdentAST *keyIdent = ast->key->asIdent()) {
            if (keyIdent->isLookable() && elementsType->lookupMember(keyIdent, this))
                addUse(keyIdent, GoSemanticHighlighter::Field);
            accept(ast->value);
            return false;
        }
    }
    return true;
}

bool GoCheckSymbols::visit(IdentAST *ast)
{
    if (ast->isLookable()) {
        if (Symbol *symbol = m_currentScope->lookupMember(ast, this))
            addUse(ast, kindForSymbol(symbol));
    }
    return false;
}

bool GoCheckSymbols::visit(DeclIdentAST *ast)
{
    if (ast->symbol)
        addUse(ast, GoSemanticHighlighter::VarDecl);
    return false;
}

Type *GoCheckSymbols::resolveSelectorExpr(ExprAST *x, bool &isFieldOrMethod, int &derefLevel)
{
    if (RefUnaryExprAST *refExpr = x->asRefUnaryExpr()) {
        int xDerefLevel = 0;
        Type *typ = resolveSelectorExpr(refExpr->x, isFieldOrMethod, xDerefLevel);
        derefLevel += xDerefLevel - 1;
        return typ;
    }

    if (StarExprAST *starExpr = x->asStarExpr()) {
        int xDerefLevel = 0;
        Type *typ = resolveSelectorExpr(starExpr->x, isFieldOrMethod, xDerefLevel);
        derefLevel += xDerefLevel + 1;
        return typ;
    }

    if (ArrowUnaryExprAST *arrowExpr = x->asArrowUnaryExpr()) {
        int xDerefLevel = 0;
        if (Type *type = resolveSelectorExpr(arrowExpr->x, isFieldOrMethod, xDerefLevel)) {
            if (type->refLevel() + xDerefLevel == 0)
                if (Type *baseTyp = type->baseType())
                    return baseTyp->chanValueType();
        }
        return 0;
    }

    if (IdentAST *ident = x->asIdent()) {
        QTC_ASSERT(!derefLevel, return 0);
        if (!ident->isLookable())
            return 0;
        QLatin1String idStr(ident->ident->toLatin1());
        if (Symbol *s = m_currentScope->lookupMember(ident, this)) {
            addUse(ident, kindForSymbol(s));
            isFieldOrMethod = true;
            return s->type(this);
        }
        if (PackageType *context = m_snapshot->packageTypeForAlias(m_currentIndex, idStr)) {
            addUse(ident, GoSemanticHighlighter::Package);
            return context;
        }
    } else if (SelectorExprAST *selAst = x->asSelectorExpr()) {
        QTC_ASSERT(!derefLevel, return 0);
        Type *context = resolveSelectorExpr(selAst->x, isFieldOrMethod, derefLevel);
        if (IdentAST *ident = selAst->sel) {
            if (context && ident->isLookable()) {
                int testDerefLevel = derefLevel + context->refLevel();
                if (testDerefLevel == 0 || testDerefLevel == -1) {
                    if (Type *baseTyp = context->baseType()) {
                        if (Symbol *s = baseTyp->lookupMember(ident, this)) {
                            addUse(ident, isFieldOrMethod
                                   ? (s->kind() == Symbol::Fun ? GoSemanticHighlighter::Func : GoSemanticHighlighter::Field)
                                   : kindForSymbol(s));
                            isFieldOrMethod = true;
                            derefLevel = 0;
                            return s->type(this);
                        }
                    }
                }
            }
        }
    } else if (CallExprAST *callExpr = x->asCallExpr()) {
        accept(callExpr->args);
        QTC_ASSERT(!derefLevel, return 0);
        // check for...
        if (IdentAST *funcIdent = callExpr->fun->asIdent()) {
            if (funcIdent->isNewKeyword()) {            // new(...) builting function
                if (callExpr->args) {
                    int xDerefLevel = 0;
                    if (Type *typ = resolveExpr(callExpr->args->value, xDerefLevel)) {
                        derefLevel = xDerefLevel - 1;
                        return typ;
                    }
                }
                return 0;
            } else if (funcIdent->isMakeKeyword()) {    // make(...) builting function
                if (callExpr->args) {
                    int xDerefLevel = 0;
                    if (Type *typ = resolveExpr(callExpr->args->value, xDerefLevel)) {
                        derefLevel = xDerefLevel;
                        return typ;
                    }
                }
                return 0;
            }
        }
        // check for type convertion
        if (ParenExprAST *parenExpr = callExpr->fun->asParenExpr()) {
            if (parenExpr->x) {
                if (StarExprAST *starExpr = parenExpr->x->asStarExpr()) {
                    if (Type *typeConvertion = tryAcceptTypeConvertion(starExpr->x)) {
                        derefLevel = 0;
                        return typeConvertion;
                    }
                }
            }
        }
        // common case - function call
        if (Type *context = resolveSelectorExpr(callExpr->fun, isFieldOrMethod, derefLevel)) {
            if (context->refLevel() + derefLevel == 0) {
                if (Type *baseTyp = context->baseType()) {
                    isFieldOrMethod = true;
                    derefLevel = 0;
                    return baseTyp->calleeType(0, this);
                }
            }
        }
    } else if (IndexExprAST *indexExpr = x->asIndexExpr()) {
        accept(indexExpr->index);
        QTC_ASSERT(!derefLevel, return 0);
        if (Type *context = resolveSelectorExpr(indexExpr->x, isFieldOrMethod, derefLevel)) {
            if (context->refLevel() + derefLevel == 0) {
                if (Type *baseTyp = context->baseType()) {
                    isFieldOrMethod = true;
                    derefLevel = 0;
                    return baseTyp->elementsType(this);
                }
            }
        }
    } else if (CompositeLitAST *compositLit = x->asCompositeLit()) {
        if (Type *context = acceptCompositLiteral(compositLit)) {
            isFieldOrMethod = true;
            return context;
        }
    } else if (TypeAssertExprAST *typeAssert = x->asTypeAssertExpr()) {
        accept(typeAssert->x);
        accept(typeAssert->typ);
        isFieldOrMethod = true;
        derefLevel = 0;
        return typeAssert->typ ? typeAssert->typ->asType() : 0;
    } else if (ParenExprAST *parenExpr = x->asParenExpr()) {
        if (parenExpr->x) {
            isFieldOrMethod = true;
            if (StarExprAST *starExpr = parenExpr->x->asStarExpr()) {
                if (Type *typeConvertion = tryAcceptTypeConvertion(starExpr->x)) {
                    derefLevel = 0;
                    return typeConvertion;
                }
            }
            return resolveSelectorExpr(parenExpr->x, isFieldOrMethod, derefLevel);
        }
    }

    return 0;
}

Type *GoCheckSymbols::tryAcceptTypeConvertion(ExprAST *x)
{
    if (x) {
        if (IdentAST *ident = x->asIdent()) {
            if (ident->isLookable()) {
                if (Symbol *s = m_currentScope->lookupMember(ident, this)) {
                    if (s->kind() == Symbol::Typ) {
                        addUse(ident, GoSemanticHighlighter::Type);
                        return s->type(this);
                    }
                }
            }
        } else if (SelectorExprAST *selExpr = x->asSelectorExpr()) {
            if (selExpr->sel->isLookable()) {
                if (IdentAST *packageIdent = selExpr->x->asIdent()) {
                    QLatin1String packageAlias(packageIdent->ident->toLatin1());
                    if (PackageType *context = m_snapshot->packageTypeForAlias(m_currentIndex, packageAlias)) {
                        if (Symbol *s = context->lookupMember(selExpr->sel, this)) {
                            if (s->kind() == Symbol::Typ) {
                                addUse(packageIdent, GoSemanticHighlighter::Package);
                                addUse(selExpr->sel, GoSemanticHighlighter::Type);
                                return s->type(this);
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

Type *GoCheckSymbols::resolveCompositExprType(CompositeLitAST *ast)
{
    if (ExprAST *type = ast->type) {
        if (TypeAST *typ = type->asType())
            return typ;
        if (IdentAST *typeIdent = ast->type->asIdent()) {
            if (typeIdent->isLookable()) {
                if (Symbol *s = m_currentScope->lookupMember(typeIdent, this)) {
                    if (s->kind() == Symbol::Typ) {
                        addUse(typeIdent, GoSemanticHighlighter::Type);
                        return s->type(this);
                    }
                }
            }
        } else if (SelectorExprAST *selExpr = ast->type->asSelectorExpr()) {
            if (IdentAST *packageIdent = selExpr->x->asIdent()) {
                if (packageIdent->isLookable()) {
                    QLatin1String packageAlias(packageIdent->ident->toLatin1());
                    if (PackageType *context = m_snapshot->packageTypeForAlias(m_currentIndex, packageAlias)) {
                        addUse(packageIdent, GoSemanticHighlighter::Package);
                        if (selExpr->sel->isLookable()) {
                            if (Symbol *s = context->lookupMember(selExpr->sel, this)) {
                                if (s->kind() == Symbol::Typ) {
                                    addUse(selExpr->sel, GoSemanticHighlighter::Type);
                                    return s->type(this);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

}   // namespace GoTools

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
#include "packagetype.h"

#include <utils/algorithm.h>
#include <utils/qtcassert.h>

#include <QDateTime>

namespace GoTools {

static GoSemanticHighlighter::Kind kindForSymbol(const Symbol *symbol)
{
    switch (symbol->kind()) {
        case Symbol::Pkg: return GoSemanticHighlighter::Package;
        case Symbol::Con: return GoSemanticHighlighter::Const;
        case Symbol::Typ: return GoSemanticHighlighter::Type;
        case Symbol::Var: return GoSemanticHighlighter::Var;
        case Symbol::Fld: return GoSemanticHighlighter::Field;
        case Symbol::Fun: return GoSemanticHighlighter::Func;
        case Symbol::Mtd: return GoSemanticHighlighter::Func;
        case Symbol::Lbl: return GoSemanticHighlighter::Label;
        default: break; // prevent -Wswitch warning
    }

    return GoSemanticHighlighter::Var;
}

GoCheckSymbols::GoCheckSymbols(GoSource *source)
    : ScopeSwitchVisitor(source)
    , m_source(source)
    , m_control(source->translationUnit()->control())
    , _chunkSize(50)
{
    _tokens = translationUnit()->tokens();
    _usages.reserve(_chunkSize);
}

void GoCheckSymbols::run()
{
    if (!isCanceled() && isValidResolveContext() && !m_source->isTooBig()) {
        if (IdentAST *packageName = m_initialFileAst->packageName)
            addUseCheckFirstLine(packageName, GoSemanticHighlighter::Package);
        accept(m_initialFileAst->importDecls);
        accept(m_initialFileAst->decls);
        flush();
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

bool GoCheckSymbols::visit(FieldAST *ast)
{
    for (DeclIdentListAST *it = ast->names; it; it = it->next)
        if (DeclIdentAST *ident = it->value)
            if (ident->isLookable() && ident->symbol)
                addUse(ident, ident->symbol->kind() == Symbol::Fld
                       ? GoSemanticHighlighter::Field
                       : GoSemanticHighlighter::Arg);

    accept(ast->type);
    return false;
}

const Type *GoCheckSymbols::resolveNamedType(TypeIdentAST *ast)
{
    if (IdentAST *ident = ast->ident) {
        if (ident->isLookable()) {
            if (Symbol *symbol = m_currentScope->lookupMember(ident, this)) {
                if (symbol->kind() == Symbol::Typ) {
                    const Type *resolvedType = symbol->type(this);
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

const Type *GoCheckSymbols::resolveNamedType(PackageTypeAST *ast)
{
    QString packageAlias(ast->packageAlias->ident->toString());
    PackageType *context = packageTypeForAlias(packageAlias);
    if (!context)
        return 0;

    addUse(ast->packageAlias, GoSemanticHighlighter::Package);

    if (ast->typeName->isLookable()) {
        if (Symbol *symbol = context->lookupMember(ast->typeName, this))
            if (symbol->kind() == Symbol::Typ) {
                const Type *resolvedType = symbol->type(this);
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
    int derefLevel = 0;
    const Type *context = resolveSelectorExpr(ast->x, derefLevel);
    if (context && ast->sel && ast->sel->isLookable()) {
        derefLevel += context->refLevel();
        if (derefLevel == 0 || derefLevel == -1) {
            if (const Type *baseTyp = context->baseType()) {
                if (Symbol *s = baseTyp->lookupMember(ast->sel, this)) {
                    addUse(ast->sel, kindForSymbol(s));
                }
            }
        }
    }

    return false;
}

const Type *GoCheckSymbols::acceptCompositLiteral(CompositeLitAST *ast)
{
    const Type *type = 0;
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
    const Type *elementsType = m_nestedCimpositLitType.empty() ? 0 : m_nestedCimpositLitType.top();
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
    if (ast->symbol) {
        switch (ast->symbol->kind()) {
            case Symbol::Typ:
                addUse(ast, GoSemanticHighlighter::TypeDecl);
                break;
            case Symbol::Con:
                addUse(ast, GoSemanticHighlighter::ConstDecl);
                break;
            case Symbol::Fun:
            case Symbol::Mtd:
                addUse(ast, GoSemanticHighlighter::FuncDecl);
                break;
            default:
                addUse(ast, GoSemanticHighlighter::VarDecl);
                break;
        }
    }

    return false;
}

const Type *GoCheckSymbols::resolveSelectorExpr(ExprAST *x, int &derefLevel)
{
    if (RefUnaryExprAST *refExpr = x->asRefUnaryExpr()) {
        int xDerefLevel = 0;
        const Type *typ = resolveSelectorExpr(refExpr->x, xDerefLevel);
        derefLevel += xDerefLevel - 1;
        return typ;
    }

    if (StarExprAST *starExpr = x->asStarExpr()) {
        int xDerefLevel = 0;
        const Type *typ = resolveSelectorExpr(starExpr->x, xDerefLevel);
        derefLevel += xDerefLevel + 1;
        return typ;
    }

    if (ArrowUnaryExprAST *arrowExpr = x->asArrowUnaryExpr()) {
        int xDerefLevel = 0;
        if (const Type *type = resolveSelectorExpr(arrowExpr->x, xDerefLevel)) {
            if (type->refLevel() + xDerefLevel == 0)
                if (const Type *baseTyp = type->baseType())
                    return baseTyp->chanValueType();
        }
        return 0;
    }

    if (IdentAST *ident = x->asIdent()) {
        QTC_ASSERT(!derefLevel, return 0);
        if (!ident->isLookable())
            return 0;
        QString idStr(ident->ident->toString());
        if (Symbol *s = m_currentScope->lookupMember(ident, this)) {
            addUse(ident, kindForSymbol(s));
            return s->type(this);
        }
        if (PackageType *context = packageTypeForAlias(idStr)) {
            addUse(ident, GoSemanticHighlighter::Package);
            return context;
        }
    } else if (SelectorExprAST *selAst = x->asSelectorExpr()) {
        QTC_ASSERT(!derefLevel, return 0);
        const Type *context = resolveSelectorExpr(selAst->x, derefLevel);
        if (IdentAST *ident = selAst->sel) {
            if (context && ident->isLookable()) {
                int testDerefLevel = derefLevel + context->refLevel();
                if (testDerefLevel == 0 || testDerefLevel == -1) {
                    if (const Type *baseTyp = context->baseType()) {
                        if (Symbol *s = baseTyp->lookupMember(ident, this)) {
                            addUse(ident, kindForSymbol(s));
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
                    if (const Type *typ = callExpr->args->value->resolve(this, xDerefLevel)) {
                        derefLevel = xDerefLevel - 1;
                        return typ;
                    }
                }
                return 0;
            } else if (funcIdent->isMakeKeyword()) {    // make(...) builting function
                if (callExpr->args) {
                    int xDerefLevel = 0;
                    if (const Type *typ = callExpr->args->value->resolve(this, xDerefLevel)) {
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
                    if (const Type *typeConvertion = tryAcceptTypeConvertion(starExpr->x)) {
                        derefLevel = 0;
                        return typeConvertion;
                    }
                }
            }
        }
        // common case - function call
        if (const Type *context = resolveSelectorExpr(callExpr->fun, derefLevel)) {
            if (context->refLevel() + derefLevel == 0) {
                if (const Type *baseTyp = context->baseType()) {
                    derefLevel = 0;
                    return baseTyp->calleeType(0, this);
                }
            }
        }
    } else if (IndexExprAST *indexExpr = x->asIndexExpr()) {
        accept(indexExpr->index);
        QTC_ASSERT(!derefLevel, return 0);
        if (const Type *context = resolveSelectorExpr(indexExpr->x, derefLevel)) {
            if (context->refLevel() + derefLevel == 0) {
                if (const Type *baseTyp = context->baseType()) {
                    derefLevel = 0;
                    return baseTyp->elementsType(this);
                }
            }
        }
    } else if (CompositeLitAST *compositLit = x->asCompositeLit()) {
        if (const Type *context = acceptCompositLiteral(compositLit)) {
            return context;
        }
    } else if (TypeAssertExprAST *typeAssert = x->asTypeAssertExpr()) {
        accept(typeAssert->x);
        accept(typeAssert->typ);
        derefLevel = 0;
        return typeAssert->typ ? typeAssert->typ->asType() : 0;
    } else if (ParenExprAST *parenExpr = x->asParenExpr()) {
        if (parenExpr->x) {
            if (StarExprAST *starExpr = parenExpr->x->asStarExpr()) {
                if (const Type *typeConvertion = tryAcceptTypeConvertion(starExpr->x)) {
                    derefLevel = 0;
                    return typeConvertion;
                }
            }
            return resolveSelectorExpr(parenExpr->x, derefLevel);
        }
    }

    return 0;
}

const Type *GoCheckSymbols::tryAcceptTypeConvertion(ExprAST *x)
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
                    QString packageAlias(packageIdent->ident->toString());
                    if (PackageType *context = packageTypeForAlias(packageAlias)) {
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

const Type *GoCheckSymbols::resolveCompositExprType(CompositeLitAST *ast)
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
                    QString packageAlias(packageIdent->ident->toString());
                    if (PackageType *context = packageTypeForAlias(packageAlias)) {
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

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
#include "findreferences.h"
#include "gosettings.h"
#include "gopackage.h"
#include "packagetype.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <coreplugin/find/searchresultitem.h>

#include <QApplication>

namespace GoTools {

namespace {

class IReferencesFinder
{
public:
    virtual ~IReferencesFinder() { }
    virtual QList<Core::SearchResultItem> findReferences() = 0;
    virtual QString customReplaceSuffixForFirstItem() const = 0;
    virtual QString referenceDescription() const = 0;
    virtual QString referenceIdentifier() const = 0;
};

class GlobalReferencesVisitor: protected ASTVisitor
{
public:
    GlobalReferencesVisitor(GoSource *source, Symbol *symbol, unsigned symbolLength, const GoPackageKey &pkgKey)
        : ASTVisitor(source->translationUnit())
        , m_source(source)
        , m_symbol(symbol)
        , m_symbolLength(symbolLength)
        , m_aliasShadowed(false)
    {
        for (const GoSource::Import &import: m_source->imports()) {
            if (import.resolvedDir == pkgKey.first && import.packageName == pkgKey.second) {
                alias = import.aliasSpecified ? import.alias : import.packageName;
                break;
            }
        }
    }

    QList<Core::SearchResultItem> acceptDeclarations() {
        if (!alias.isEmpty()) {
            if (FileAST *fileAst = m_source->translationUnit()->fileAst()) {
                const auto aliasBa = alias.toUtf8();
                const Identifier id(aliasBa.constData(), aliasBa.length());
                m_aliasIdent = &id;
                if (!m_source->package()->type()->lookup(m_aliasIdent))     // check for symbol identifier shadowing at package level
                    accept(fileAst->decls);
            }
        }
        return m_results;
    }

protected:
    virtual bool visit(FuncDeclAST *ast) override {
        accept(ast->recv);
        accept(ast->name);
        accept(ast->type);
        if (m_aliasShadowed)
            m_aliasShadowed = false;
        else
            accept(ast->body);
        return false;
    }

    virtual bool visit(BlockStmtAST *ast) override {
        for (StmtListAST *stmt_it = ast->list; stmt_it && !m_aliasShadowed; stmt_it = stmt_it->next)
            accept(stmt_it->value);
        m_aliasShadowed = false;
        return false;
    }

    virtual bool visit(IfStmtAST *ast) override {
        accept(ast->init);
        if (m_aliasShadowed)
            m_aliasShadowed = false;
        else {
            accept(ast->cond);
            accept(ast->body);
            accept(ast->elseStmt);
        }
        return false;
    }

    virtual bool visit(RangeStmtAST *ast) override {
        accept(ast->key);
        accept(ast->value);
        accept(ast->x);
        if (m_aliasShadowed)
            m_aliasShadowed = false;
        else
            accept(ast->body);
        return false;
    }

    virtual bool visit(ForStmtAST *ast) override {

        accept(ast->init);
        if (m_aliasShadowed)
            m_aliasShadowed = false;
        else {
            accept(ast->cond);
            accept(ast->post);
            accept(ast->body);
        }
        return false;
    }

    virtual bool visit(TypeSwitchStmtAST *ast) override {
        accept(ast->init);
        accept(ast->assign);
        if (m_aliasShadowed)
            m_aliasShadowed = false;
        else
            accept(ast->body);
        return false;
    }

    virtual bool visit(SwitchStmtAST *ast) override {
        accept(ast->init);
        if (m_aliasShadowed)
            m_aliasShadowed = false;
        else
            accept(ast->body);
        return false;
    }

    virtual bool visit(CaseClauseAST *ast) override {
        accept(ast->list);
        for (StmtListAST *stmt_it = ast->body; stmt_it && !m_aliasShadowed; stmt_it = stmt_it->next)
            accept(stmt_it->value);
        m_aliasShadowed = false;
        return false;
    }

    virtual bool visit(DeclIdentAST *ast) {
        if (!ast->symbol || ast->symbol->kind() == Symbol::Fld)
            return false;
        if (ast->ident->equalTo(m_aliasIdent)) {
            m_aliasShadowed = true;
        }
        return false;
    }

    virtual bool visit(PackageTypeAST *ast) override {
        if (m_symbol->kind() == Symbol::Typ) {
            if (ast->packageAlias && ast->packageAlias->ident->equalTo(m_aliasIdent) &&
                ast->typeName && ast->typeName->ident->equalTo(m_symbol->identifier())) {
                m_results << m_source->searchResultItemForTokenIndex(ast->typeName->t_identifier, m_symbolLength);
            }
        }
        return false;
    }

    virtual bool visit(SelectorExprAST *ast) override {
        if (IdentAST *ident = ast->x->asIdent()) {
            if (ident->ident->equalTo(m_aliasIdent) && ast->sel && ast->sel->ident->equalTo(m_symbol->identifier()))
                m_results << m_source->searchResultItemForTokenIndex(ast->sel->t_identifier, m_symbolLength);
            return false;
        }

        accept(ast->x);
        return false;
    }

private:
    GoSource *m_source;
    Symbol *m_symbol;
    unsigned m_symbolLength;
    QString alias;
    const Identifier *m_aliasIdent;
    bool m_aliasShadowed;
    QList<Core::SearchResultItem> m_results;
};

class LocalReferencesVisitor: public IReferencesFinder, protected ASTVisitor
{
public:
    LocalReferencesVisitor(GoSource *source, Symbol *symbol, bool inSymbolScope = true)
        : ASTVisitor(source->translationUnit())
        , m_inSymbolScope(inSymbolScope)
        , m_source(source)
        , m_symbol(symbol)
        , m_symbolShadowed(false)
        , m_inShadowDecl(false)
    {
        m_symbolLength = m_symbol->identifier()->toString().length();
        m_sourceFileScope = source->translationUnit()->fileAst() ? source->translationUnit()->fileAst()->scope : 0;
    }

    virtual QList<Core::SearchResultItem> findReferences() override {
        if (m_inSymbolScope)
            accept(m_symbol->owner()->ast());
        else if (m_source && m_source->translationUnit())
            if (FileAST *fileAst = m_source->translationUnit()->fileAst())
                accept(fileAst->decls);
        return m_results;
    }

    virtual QString customReplaceSuffixForFirstItem() const override
    { return QString(); }

    virtual QString referenceDescription() const override
    { return QObject::tr("Local Symbol Usages:"); }

    virtual QString referenceIdentifier() const override
    { return m_symbol->identifier()->toString(); }

protected:
    virtual bool preVisit(AST *) { return !m_symbolShadowed; }

    virtual void endVisit(FuncDeclAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(BlockStmtAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(IfStmtAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(RangeStmtAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(ForStmtAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(TypeSwitchStmtAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(SwitchStmtAST *) override { m_symbolShadowed = false; }
    virtual void endVisit(CaseClauseAST *) override { m_symbolShadowed = false; }

    virtual bool visit(DeclIdentAST *ast) {
        if (!ast->symbol || ast->symbol->kind() == Symbol::Fld)
            return false;
        if (ast->ident->equalTo(m_symbol->identifier())) {
            if (ast->symbol == m_symbol)
                m_results << m_source->searchResultItemForTokenIndex(ast->t_identifier, m_symbolLength);
            else {
                m_inShadowDecl = true;
                accept(ast->symbol->declExpr());
                m_inShadowDecl = false;
                m_symbolShadowed = true;
            }
        }
        return false;
    }

    virtual bool visit(IdentAST *ast) {
        if (ast->ident->equalTo(m_symbol->identifier()) && !m_symbolShadowed)
            if (m_inShadowDecl || m_symbol->owner()->fileScope() != m_sourceFileScope ||
                    !m_symbol->declExpr() ||
                    ast->t_identifier < m_symbol->declExpr()->firstToken() ||
                    ast->t_identifier > m_symbol->declExpr()->lastToken())
                m_results << m_source->searchResultItemForTokenIndex(ast->t_identifier, m_symbolLength);
        return false;
    }

    virtual bool visit(SelectorExprAST *ast) {
        accept(ast->x);
        return false;
    }

    virtual bool visit(PackageTypeAST *) {
        return false;
    }

private:
    bool m_inSymbolScope;
    GoSource *m_source;
    Symbol *m_symbol;
    bool m_symbolShadowed;
    bool m_inShadowDecl;
    QList<Core::SearchResultItem> m_results;
    unsigned m_symbolLength;
    FileScope *m_sourceFileScope;
};

class FieldReferencesVisitor: protected ScopeSwitchVisitor
{
public:
    FieldReferencesVisitor(GoSource *source, Symbol *symbol, unsigned symbolLength)
        : ScopeSwitchVisitor(source, false)
        , m_source(source)
        , m_symbol(symbol)
        , m_fieldOwnerType(symbol->owner()->ast()->asType())
        , m_symbolLength(symbolLength)
    { }

    virtual ~FieldReferencesVisitor() {}

    QList<Core::SearchResultItem> acceptDeclarations() {
        if (isValidResolveContext() && m_source)
            if (FileAST *fileAst = m_source->translationUnit()->fileAst())
                accept(fileAst->decls);
        return m_results;
    }

    QList<Core::SearchResultItem> acceptScopeAst(AST *ast) {
        if (isValidResolveContext())
            accept(ast);
        return m_results;
    }

protected:
    virtual bool visit(SelectorExprAST *ast) {
        if (m_symbol->identifier()->equalTo(ast->sel->ident)) {
            if (const Type *type = ast->x->resolve(this).typeForMemberAccess()) {
                if (type == m_fieldOwnerType) {
                    m_results << m_source->searchResultItemForTokenIndex(ast->sel->t_identifier, m_symbolLength);
                } else if (const NamedType *namedType = type->asNamedType()) {
                    if (const TypeSpecAST *ts = namedType->typeSpec(this)) {
                        if (ts->hasEmbedOrEqualToType(m_fieldOwnerType, this))
                            m_results << m_source->searchResultItemForTokenIndex(ast->sel->t_identifier, m_symbolLength);
                    }
                }
            }
        }

        accept(ast->x);
        return false;
    }

    virtual bool visit(CompositeLitAST *ast) {
        if (ast->elements) {
            const Type *type = 0;
            if (ExprAST *typeExpr = ast->type) {
                type = typeExpr->asType();
                if (!type) {
                    type = tryResolveNamedType(this, typeExpr);
                }
            } else if (!m_nestedCompositLitType.empty()) {
                type = m_nestedCompositLitType.top();
                if (type)
                    type = type->elementsType(this).type();
            }
            m_nestedCompositLitType.push(type);
            accept(ast->elements);
            m_nestedCompositLitType.pop();
            return false;
        }
        return true;
    }

    virtual bool visit(KeyValueExprAST *ast) {
        const Type *elementsType = m_nestedCompositLitType.empty() ? 0 : m_nestedCompositLitType.top();
        if (elementsType && ast->key) {
            if (IdentAST *keyIdent = ast->key->asIdent()) {
                if (m_symbol->identifier()->equalTo(keyIdent->ident)) {
                    if (elementsType == m_fieldOwnerType) {
                        m_results << m_source->searchResultItemForTokenIndex(keyIdent->t_identifier, m_symbolLength);
                    } else if (const NamedType *namedType = elementsType->asNamedType()) {
                        if (const TypeSpecAST *ts = namedType->typeSpec(this)) {
                            if (ts->type == m_fieldOwnerType)
                                m_results << m_source->searchResultItemForTokenIndex(keyIdent->t_identifier, m_symbolLength);
                        }
                    }
                }
            }
        }
        accept(ast->value);
        return false;
    }

protected:
    GoSource *m_source;
    Symbol *m_symbol;
    const Type *m_fieldOwnerType;
    std::stack<const Type *> m_nestedCompositLitType;
    unsigned m_symbolLength;
    QList<Core::SearchResultItem> m_results;
};

class LocalFieldReferences: public IReferencesFinder
{
public:
    LocalFieldReferences(GoSource *source, Symbol *symbol, AST *ast)
        : m_symbolLength(symbol->identifier()->toString().length())
        , m_fieldsVisistor(source, symbol, m_symbolLength)
        , m_source(source)
        , m_symbol(symbol)
        , m_ast(ast)
    { }

    virtual ~LocalFieldReferences() {}

    virtual QList<Core::SearchResultItem> findReferences() override {
        QList<Core::SearchResultItem> results = m_fieldsVisistor.acceptScopeAst(m_ast);
        results.prepend(m_source->searchResultItemForTokenIndex(m_symbol->sourceLocation(), m_symbolLength));
        return results;
    }

    virtual QString customReplaceSuffixForFirstItem() const override
    { return QString(); }

    virtual QString referenceDescription() const override
    { return QObject::tr("Field Usages:"); }

    virtual QString referenceIdentifier() const override
    { return m_symbol->identifier()->toString(); }

private:
    unsigned m_symbolLength;
    FieldReferencesVisitor m_fieldsVisistor;
    GoSource *m_source;
    Symbol *m_symbol;
    AST *m_ast;
};

class GlobalFieldReferences: public IReferencesFinder
{
public:
    GlobalFieldReferences(Symbol *symbol, ResolveContext *resolveContext)
        : m_symbolLength(symbol->identifier()->toString().length())
        , m_symbol(symbol)
        , m_resolveContext(resolveContext)
    { }

    virtual ~GlobalFieldReferences() {}

    virtual QList<Core::SearchResultItem> findReferences() override {
        QList<Core::SearchResultItem> results;

        if (FileScope *symbolFileScope = m_symbol->owner()->fileScope()) {
            const QList<GoSource *> sources = m_resolveContext->sources();
            QFutureInterface<void> interface;
            interface.setProgressRange(0, sources.size());
            Core::ProgressManager::addTask(interface.future(), QObject::tr("Finding Field Usages"),
                                           "GoEditor.FindUsages");
            int progress = 0;
            for (GoSource *source: sources) {
                FieldReferencesVisitor visitor(source, m_symbol, m_symbolLength);
                QList<Core::SearchResultItem> fileResults = visitor.acceptDeclarations();
                if (FileAST *fileAst = source->translationUnit()->fileAst())
                    if (fileAst->scope == symbolFileScope)
                        fileResults.prepend(source->searchResultItemForTokenIndex(m_symbol->sourceLocation(), m_symbolLength));
                interface.setProgressValue(++progress);
                QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                results << fileResults;
            }
            interface.reportFinished();
        }

        return results;
    }

    virtual QString customReplaceSuffixForFirstItem() const override
    { return QString(); }

    virtual QString referenceDescription() const override
    { return QObject::tr("Field Usages:"); }

    virtual QString referenceIdentifier() const override
    { return m_symbol->identifier()->toString(); }

private:
    unsigned m_symbolLength;
    Symbol *m_symbol;
    ResolveContext *m_resolveContext;
};

class PackageAliasReferences: public IReferencesFinder, protected ASTVisitor
{
public:
    PackageAliasReferences(GoSource::Ptr doc, const QString &packageAlias,
                           ResolveContext *ctx)
        : ASTVisitor(doc->translationUnit())
        , m_doc(doc)
        , m_resolveContext(ctx)
        , m_packageAlias(packageAlias)
        , m_packageAliasLength(packageAlias.length())
        , m_packageIdent(0)
    { }

    virtual QList<Core::SearchResultItem> findReferences() override
    {
        const QByteArray rawIdent = m_packageAlias.toUtf8();
        const Identifier packageIdent(rawIdent.constData(), rawIdent.length());
        m_packageIdent = &packageIdent;

        FileAST *fileAst = m_doc->translationUnit()->fileAst();
        accept(fileAst->importDecls);
        accept(fileAst->decls);
        return m_results;
    }

    virtual QString customReplaceSuffixForFirstItem() const override
    { return m_customReplaceSuffixForFirstItem; }

    virtual QString referenceDescription() const override
    { return QObject::tr("Package Alias Usages:"); }

    virtual QString referenceIdentifier() const override
    { return m_packageAlias; }

protected:
    virtual bool visit(ImportSpecAST *importSpec) {
        if (IdentAST *alias = importSpec->name) {
            if (alias->ident->equalTo(m_packageIdent))
                m_results << m_doc->searchResultItemForTokenIndex(alias->t_identifier, m_packageAliasLength);
        } else if (importSpec->t_path) {
            const StringLiteral *path = _tokens->at(importSpec->t_path).string;
            QString pathStr = path->unquoted();
            if (!pathStr.isEmpty()) {
                QString aliasPathPart = pathStr.split('/').last();
                if (aliasPathPart == m_packageAlias) {
                    m_results << m_doc->searchResultItemForTokenIndex(importSpec->t_path, pathStr.length() + 2);
                    m_customReplaceSuffixForFirstItem = QLatin1String(" \"") + pathStr + QLatin1String("\"");
                }
            }
        }
        return false;
    }

    virtual bool visit(PackageTypeAST *ast) {
        if (IdentAST *packageAlias = ast->packageAlias) {
            if (packageAlias->ident->equalTo(m_packageIdent))
                m_results << m_doc->searchResultItemForTokenIndex(packageAlias->t_identifier, m_packageAliasLength);
        }
        return false;
    }

    virtual bool visit(SelectorExprAST *ast) {
        if (ast->x)
            if (IdentAST *packageAlias = ast->x->asIdent())
                if (packageAlias->ident->equalTo(m_packageIdent))
                    if (!m_resolveContext->currentScope()->lookupMember(packageAlias, m_resolveContext))
                        m_results << m_doc->searchResultItemForTokenIndex(packageAlias->t_identifier, m_packageAliasLength);
        return false;
    }

private:
    GoSource::Ptr m_doc;
    ResolveContext *m_resolveContext;
    QString m_packageAlias;
    int m_packageAliasLength;
    const Identifier *m_packageIdent;
    QList<Core::SearchResultItem> m_results;

    QString m_customReplaceSuffixForFirstItem;
};

class MethodReferencesVisistor: protected ScopeSwitchVisitor
{
public:
    MethodReferencesVisistor(GoSource *source, MethodDecl *methodDecl, TypeIdentAST *recvIdent,
                             const TypeSpecAST *recvTypeSpec, unsigned symbolLength)
        : ScopeSwitchVisitor(source, false)
        , m_source(source)
        , m_methodDecl(methodDecl)
        , m_recvIdent(recvIdent)
        , m_recvTypeSpec(recvTypeSpec)
        , m_symbolLength(symbolLength)
    { }

    QList<Core::SearchResultItem> acceptDeclarations() {
        if (m_source && m_methodDecl && isValidResolveContext()) {
            m_methodDeclFileScope = m_methodDecl->owner()->fileScope();
            accept(m_initialFileAst->decls);
        }
        return m_results;
    }

protected:
    virtual bool visit(FuncDeclAST *ast) {
        if (m_methodDeclFileScope == m_initialFileScope && ast->name) {
            if (ast->recv && ast->recv->fields && !ast->recv->fields->next) {
                if (FieldAST *field = ast->recv->fields->value) {
                    if (TypeAST *typ = field->type) {
                        if (StarTypeAST *baseTyp = typ->asStarType())
                            typ = baseTyp->typ;
                        if (m_recvIdent == typ)
                            m_results << m_source->searchResultItemForTokenIndex(ast->name->t_identifier, m_symbolLength);
                    }
                }
            }
        }
        accept(ast->body);
        return false;
    }

    virtual bool visit(SelectorExprAST *ast) {
        if (ast->sel && ast->sel->ident->equalTo(m_methodDecl->identifier())) {
            if (const Type *typ = ast->x->resolve(this).typeForMemberAccess()) {
                if (const NamedType *namedType = typ->asNamedType()) {
                    if (const TypeSpecAST *typeSpec = namedType->typeSpec(this))
                        if (typeSpec->hasEmbedOrEqualTo(m_recvTypeSpec, this))
                            m_results << m_source->searchResultItemForTokenIndex(ast->sel->t_identifier, m_symbolLength);
                }
            }
        }
        accept(ast->x);
        return false;
    }

private:
    GoSource *m_source;
    MethodDecl *m_methodDecl;
    TypeIdentAST *m_recvIdent;
    const TypeSpecAST *m_recvTypeSpec;
    FileScope *m_methodDeclFileScope;
    unsigned m_symbolLength;
    QList<Core::SearchResultItem> m_results;
};

class MethodReferences: public IReferencesFinder
{
public:
    MethodReferences(Symbol *symbol, ResolveContext *resolveContext)
        : m_symbolLength(symbol->identifier()->toString().length())
        , m_symbol(symbol)
        , m_resolveContext(resolveContext)
    { }

    virtual QList<Core::SearchResultItem> findReferences() override {
        QList<Core::SearchResultItem> results;
        if (MethodDecl *methodDeclaration = m_symbol->asMethodDecl()) {
            if (const NamedType *namedType = methodDeclaration->recvIdent()->asNamedType()) {
                if (const TypeSpecAST *typeSpec = namedType->typeSpec(m_resolveContext)) {
                    const QList<GoSource *> sources = m_resolveContext->sources();
                    QFutureInterface<void> interface;
                    interface.setProgressRange(0, sources.size());
                    Core::ProgressManager::addTask(interface.future(), QObject::tr("Finding Method Usages"),
                                                   "GoEditor.FindUsages");
                    int progress = 0;
                    for (GoSource *source: sources) {
                        MethodReferencesVisistor visitor(source, methodDeclaration, methodDeclaration->recvIdent(), typeSpec, m_symbolLength);
                        results << visitor.acceptDeclarations();
                        interface.setProgressValue(++progress);
                        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                    }
                    interface.reportFinished();
                }
            }
        }
        return results;
    }

    virtual QString customReplaceSuffixForFirstItem() const override
    { return QString(); }

    virtual QString referenceDescription() const override
    { return QObject::tr("Method Usages:"); }

    virtual QString referenceIdentifier() const override
    { return m_symbol->identifier()->toString(); }

private:
    unsigned m_symbolLength;
    Symbol *m_symbol;
    ResolveContext *m_resolveContext;
};

class GlobalReferences: public IReferencesFinder
{
public:
    GlobalReferences(Symbol *symbol, ResolveContext *resolveContext)
        : m_symbolLength(symbol->identifier()->toString().length())
        , m_symbol(symbol)
        , m_resolveContext(resolveContext)
    { }

    virtual QList<Core::SearchResultItem> findReferences() override {
        QList<Core::SearchResultItem> results;
        if (m_symbol) {
            if (FileScope *symbolFileScope = m_symbol->owner()->fileScope()) {
                GoPackageKey pkgKey{symbolFileScope->source()->location(), symbolFileScope->source()->packageName()};
                if (GoPackage *symbolPackage = symbolFileScope->source()->package()) {
                    const QList<GoSource *> sources = m_resolveContext->sources();
                    QFutureInterface<void> interface;
                    interface.setProgressRange(0, sources.size());
                    Core::ProgressManager::addTask(interface.future(), QObject::tr("Finding Symbol Usages"),
                                                   "GoEditor.FindUsages");
                    int progress = 0;
                    for (GoSource *source: sources) {
                        if (source->package() == symbolPackage) {
                            LocalReferencesVisitor visitor(source, m_symbol, false);
                            results << visitor.findReferences();
                        } else {
                            GlobalReferencesVisitor visitor(source, m_symbol, m_symbolLength, pkgKey);
                            results << visitor.acceptDeclarations();
                        }
                        interface.setProgressValue(++progress);
                        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
                    }
                    interface.reportFinished();
                }
            }
        }
        return results;
    }

    virtual QString customReplaceSuffixForFirstItem() const override
    { return QString(); }

    virtual QString referenceDescription() const override
    { return QObject::tr("Symbol Usages:"); }

    virtual QString referenceIdentifier() const override
    { return m_symbol->identifier()->toString(); }

private:
    unsigned m_symbolLength;
    Symbol *m_symbol;
    ResolveContext *m_resolveContext;
};

}   // anonimous namesapace

FindReferences::FindReferences(GoSource::Ptr doc)
    : SymbolUnderCursorDescriber(doc)
    , m_search(0)
{ }

Core::SearchResult *FindReferences::proceedReferences(unsigned pos, bool isReplace)
{
    m_pos = pos;
    m_symbol = 0;
    m_token = 0;
    m_packageAlias.clear();

    m_search = 0;
    m_isReplace = isReplace;

    if (m_doc->translationUnit() && isValidResolveContext()) {
        acceptForPosition(m_initialFileAst->importDecls);
        if (!_traverseFinished)
            acceptForPosition(m_initialFileAst->decls);
        QScopedPointer <IReferencesFinder> finder;

        if (m_symbol && m_symbol->owner()) {
            if (m_symbol->kind() == Symbol::Fld) {
                if (m_symbol->owner()) {
                    if (m_symbol->owner()->isLocal()) {
                        finder.reset(new LocalFieldReferences(m_doc.data(), m_symbol,
                                                              m_symbol->owner()->enclosingAst()));
                    } else {
                        finder.reset(new GlobalFieldReferences(m_symbol, this));
                    }
                }
            } else {
                bool isPackageLevelSymbol = m_symbol->owner() == m_symbol->fileScope();
                if (!isPackageLevelSymbol) {
                    finder.reset(new LocalReferencesVisitor(m_doc.data(), m_symbol));
                } else if (m_symbol->kind() == Symbol::Mtd) {
                    finder.reset(new MethodReferences(m_symbol, this));
                } else {
                    finder.reset(new GlobalReferences(m_symbol, this));
                }
                bool isCurrentPackageSymbol = m_symbol->fileScope() == m_fileScope;
                qDebug() << "FRU:" << m_symbol->identifier()->toString() << isCurrentPackageSymbol << isPackageLevelSymbol;
            }

        } else if (!m_packageAlias.isEmpty()) {
            finder.reset(new PackageAliasReferences(m_doc, m_packageAlias, this));
        }

        if (finder) {
            m_search = Core::SearchResultWindow::instance()->startNewSearch(finder->referenceDescription(),
                                                                            QString(),
                                                                            finder->referenceIdentifier(),
                                                                            m_isReplace ? Core::SearchResultWindow::SearchAndReplace
                                                                                        : Core::SearchResultWindow::SearchOnly,
                                                                            Core::SearchResultWindow::PreserveCaseDisabled,
                                                                            QLatin1String("GoEditor"));
            m_search->addResults(finder->findReferences(), Core::SearchResult::AddOrdered);
            GoFindReferencesParameters parameters;
            parameters.customReplaceSuffixForFirstItem = finder->customReplaceSuffixForFirstItem();
            m_search->setUserData(qVariantFromValue(parameters));
            if (m_isReplace)
                m_search->setTextToReplace(finder->referenceIdentifier());
            m_search->finishSearch(false);
        }
    }

    return m_search;
}

}   // namespace GoTools

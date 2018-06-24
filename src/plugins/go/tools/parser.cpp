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
#include "parser.h"
#include "ast.h"
#include "control.h"

namespace GoTools {

Parser::Parser(GoSource *source, unsigned mode)
    : _source(source)
    , _translationUnit(source->translationUnit())
    , _control(_translationUnit->control())
    , _pool(_translationUnit->memoryPool())
    , _firstSourceChar(_translationUnit->firstSourceChar())
    , _lastSourceChar(_translationUnit->lastSourceChar())
    , _mode(mode)
    , _scanner(_translationUnit, _firstSourceChar, _lastSourceChar)
    , _tokens(_translationUnit->tokens())
{
    _trace = _mode & Trace;

    tokenIndex = 0;
    tok.kindAndPos.kind = T_EOF;
    _tokens->push_back(tok);
    exprLev = 0;
    inRhs = false;
    syncTokenIndex = 0;
    syncCnt = 0;
    topScope = nullptr;
    structScope = nullptr;
    fileScope = nullptr;
}

FileAST *Parser::parseFile()
{
    next();

    CommentGroupAST *doc = leadComment;
    unsigned t_package = expectStrict(PACKAGE);
    if (!t_package)
        return nullptr;
    unsigned t_package_name = expectStrict(IDENT);
    if (!t_package_name)
        return nullptr;
    IdentAST *packageName = new (_pool) IdentAST(t_package_name, _tokens->at(t_package_name).identifier);
    expectSemi();

    DeclListAST *decls = nullptr;
    DeclListAST **decls_ptr = &decls;

    DeclListAST *import_decls = nullptr;
    DeclListAST **import_decls_ptr = &import_decls;

    fileScope = _control->newFileScope(_source);
    topScope = fileScope;

    // parse imports
    while (tok.kindAndPos.kind == IMPORT) {
        if (GenDeclAST *importDecl = parseGenDecl(IMPORT)) {
            *import_decls_ptr = new (_pool) DeclListAST(importDecl);
            import_decls_ptr = &(*import_decls_ptr)->next;
        }
    }

    // parse declarations
    while (tok.kindAndPos.kind != T_EOF) {
        if (DeclAST *decl = parseDeclSyncDecl()) {
            *decls_ptr = new (_pool) DeclListAST(decl);
            decls_ptr = &(*decls_ptr)->next;
        }
    }

    closeScope();

    FileAST *fileAst = new (_pool) FileAST(doc, t_package, packageName, import_decls, decls);
    fileScope->setAst(fileAst);
    fileAst->scope = fileScope;
    return fileAst;
}

FileAST *Parser::parsePackageFile(const QString &wantedPackageName)
{
    next();

    CommentGroupAST *doc = leadComment;
    unsigned t_package = expectStrict(PACKAGE);
    if (!t_package)
        return nullptr;
    unsigned t_package_name = expectStrict(IDENT);
    if (!t_package_name)
        return nullptr;
    IdentAST *packageName = new (_pool) IdentAST(t_package_name, _tokens->at(t_package_name).identifier);
    if (wantedPackageName != packageName->ident->toString())
        return nullptr;
    expectSemi();

    DeclListAST *decls = nullptr;
    DeclListAST **decls_ptr = &decls;

    DeclListAST *import_decls = nullptr;
    DeclListAST **import_decls_ptr = &import_decls;

    fileScope = _control->newFileScope(_source);
    topScope = fileScope;

    // parse imports
    while (tok.kindAndPos.kind == IMPORT) {
        if (GenDeclAST *importDecl = parseGenDecl(IMPORT)) {
            *import_decls_ptr = new (_pool) DeclListAST(importDecl);
            import_decls_ptr = &(*import_decls_ptr)->next;
        }
    }

    // parse declarations
    while (tok.kindAndPos.kind != T_EOF) {
        if (DeclAST *decl = parseDeclSyncDecl()) {
            *decls_ptr = new (_pool) DeclListAST(decl);
            decls_ptr = &(*decls_ptr)->next;
        }
    }

    closeScope();

    FileAST *fileAst = new (_pool) FileAST(doc, t_package, packageName, import_decls, decls);
    fileScope->setAst(fileAst);
    fileAst->scope = fileScope;
    return fileAst;
}

DeclAST *Parser::parseDeclSyncDecl()
{
    switch (tok.kindAndPos.kind) {
        case CONST:
        case TYPE:
        case VAR:
            return parseGenDecl(static_cast<TokenKind>(tok.kindAndPos.kind));
        case FUNC:
            return parseFuncDecl();
    }

    unsigned t_start = tokenIndex;
    _translationUnit->error(t_start, "expected declaration");
    syncDecl();
    return new (_pool) BadDeclAST(t_start, tokenIndex);
}

DeclAST *Parser::parseDeclSyncStmt()
{
    switch (tok.kindAndPos.kind) {
        case CONST:
        case TYPE:
        case VAR:
            return parseGenDecl(static_cast<TokenKind>(tok.kindAndPos.kind));
        case FUNC:
            return parseFuncDecl();
    }

    unsigned t_start = tokenIndex;
    _translationUnit->error(t_start, "expected declaration");
    syncStmt();
    return new (_pool) BadDeclAST(t_start, tokenIndex);
}

GenDeclAST *Parser::parseGenDecl(TokenKind kind)
{
    CommentGroupAST *doc = leadComment;
    unsigned t_token = expect(kind);
    unsigned t_lparen = 0;
    SpecListAST *specs = nullptr;
    unsigned t_rparen = 0;
    SpecListAST **specs_ptr = &specs;

    if (tok.kindAndPos.kind == LPAREN) {
        t_lparen = next();
        for (int index = 0; tok.kindAndPos.kind != RPAREN && tok.kindAndPos.kind != T_EOF; index++) {
            if (SpecAST *spec = parseGenDeclSpec(kind, doc, index)) {
                *specs_ptr = new (_pool) SpecListAST(spec);
                specs_ptr = &(*specs_ptr)->next;
            }
        }
        t_rparen = expect(RPAREN);
        expectSemi();
    } else {
        if (SpecAST *spec = parseGenDeclSpec(kind, doc, 0)) {
            *specs_ptr = new (_pool) SpecListAST(spec);
            specs_ptr = &(*specs_ptr)->next;
        }
    }

    GenDeclAST *node = new (_pool) GenDeclAST(doc, t_token, t_lparen, specs, t_rparen);
    node->kind = kind;
    return node;
}

SpecAST *Parser::parseGenDeclSpec(TokenKind kind, CommentGroupAST *doc, int index)
{
    switch (kind) {
        case IMPORT:
            return parseGenDeclImportSpec(doc);
        case CONST:
             return parseGenDeclConstSpec(doc, index);
        case VAR:
            return parseGenDeclVarSpec(doc);
        case TYPE:
            return parseGenDeclTypeSpec(doc);
        default: break; // prevent -Wswitch warning
    }

    return nullptr;
}

SpecAST *Parser::parseGenDeclImportSpec(CommentGroupAST *doc)
{
    IdentAST *ident = nullptr;

    switch (tok.kindAndPos.kind) {
        case PERIOD:
            ident = new (_pool) IdentAST(next(), Control::dotIdentifier());
            break;
        case IDENT: {
            const Identifier *id = tok.identifier;
            ident = new (_pool) IdentAST(next(), id);
            break;
        }
    }

    unsigned t_path = expectStrict(STRING);
    expectSemi();

    return new (_pool) ImportSpecAST(doc, ident, t_path, lineComment);
}

SpecAST *Parser::parseGenDeclConstSpec(CommentGroupAST *doc, int index)
{
    unsigned pos = tokenIndex;

    DeclIdentListAST *idents = parseDeclIdentList();
    TypeAST *type = tryType();

    ExprListAST *values = nullptr;
    if (tok.kindAndPos.kind == ASSIGN) {
        next();
        values = parseRhsList();
    }

    expectSemi();

    if (!values && (!index || type))
        _translationUnit->error(pos, "missing constant value");

    ConstSpecAST *spec = new (_pool) ConstSpecAST(doc, idents, type, values, lineComment);
    declareConst(topScope, idents, values);

    return spec;
}

SpecAST *Parser::parseGenDeclVarSpec(CommentGroupAST *doc)
{
    unsigned pos = tokenIndex;

    DeclIdentListAST *idents = parseDeclIdentList();
    TypeAST *type = tryType();

    ExprListAST *values = nullptr;
    if (tok.kindAndPos.kind == ASSIGN) {
        next();
        values = parseRhsList();
    }

    expectSemi();

    if (!values && !type)
        _translationUnit->error(pos, "missing variable type or initialization");

    if (type) {
        VarSpecWithTypeAST *typeSpec = new (_pool) VarSpecWithTypeAST(doc, idents, type, lineComment);
        declareVar(type, topScope, idents);
        return typeSpec;
    }

    VarSpecWithValuesAST *valSpec = new (_pool) VarSpecWithValuesAST(doc, idents, values, lineComment);
    shortVarDecl(idents, values);
    return valSpec;
}

SpecAST *Parser::parseGenDeclTypeSpec(CommentGroupAST *doc)
{
    DeclIdentAST *ident = parseDeclIdent();

    TypeSpecAST *spec = new (_pool) TypeSpecAST(doc, ident);
    spec->scope = fileScope;
    // Or ???
    // spec->scope = fileScope == topScope ? fileScope : 0;

    Scope *scope = topScope;

    spec->type = parseType();
    expectSemi();
    spec->comment = lineComment;

    declareType(spec, scope, ident);

    return spec;
}

FuncDeclAST *Parser::parseFuncDecl()
{
    CommentGroupAST *doc = leadComment;
    unsigned pos = expect(FUNC);

    Scope *scope = _control->newScope(topScope);

    FieldGroupAST *recv = nullptr;
    if (tok.kindAndPos.kind == LPAREN)
        recv = parseParameters(scope, false);

    DeclIdentAST *ident = parseDeclIdent();

    FieldGroupAST *params = nullptr;
    FieldGroupAST *results = nullptr;
    parseSignature(params, results, scope);

    BlockStmtAST *body = nullptr;
    if (tok.kindAndPos.kind == LBRACE)
        body = parseBody(scope);

    expectSemi();

    FuncTypeAST *type = new (_pool) FuncTypeAST(pos, params, results, results ? results->callType(_pool) : nullptr);
    FuncDeclAST *decl = new (_pool) FuncDeclAST(doc, recv, ident, type, body);
    scope->setAst(decl);
    decl->scope = scope;

    if (!recv) {
        if (!ident->ident->equalTo("init", 4))
            declareFunc(type, fileScope, ident);
    } else {
        declareMethod(decl);
    }

    return decl;
}

IdentAST *Parser::parseIdent()
{
    if (tok.kindAndPos.kind == IDENT) {
        const Identifier *id = tok.identifier;
        return new (_pool) IdentAST(next(), id);
    }

    _translationUnit->error(tokenIndex, "expected identifier");
    return new (_pool) IdentAST(next(), Control::underscoreIdentifier());
}

DeclIdentAST *Parser::parseDeclIdent()
{
    if (tok.kindAndPos.kind == IDENT) {
        const Identifier *id = tok.identifier;
        return new (_pool) DeclIdentAST(next(), id);
    }

    _translationUnit->error(tokenIndex, "expected identifier");
    return new (_pool) DeclIdentAST(next(), Control::underscoreIdentifier());
}

IdentListAST *Parser::parseIdentList()
{
    IdentListAST *node;
    IdentListAST **node_ptr = &node;

    IdentAST *ident = parseIdent();
    *node_ptr = new (_pool) IdentListAST(ident);
    node_ptr = &(*node_ptr)->next;

    while (tok.kindAndPos.kind == COMMA) {
        next();
        ident = parseIdent();
        *node_ptr = new (_pool) IdentListAST(ident);
        node_ptr = &(*node_ptr)->next;
    }

    return node;
}

DeclIdentListAST *Parser::parseDeclIdentList()
{
    DeclIdentListAST *node;
    DeclIdentListAST **node_ptr = &node;

    DeclIdentAST *ident = parseDeclIdent();
    *node_ptr = new (_pool) DeclIdentListAST(ident);
    node_ptr = &(*node_ptr)->next;

    while (tok.kindAndPos.kind == COMMA) {
        next();
        ident = parseDeclIdent();
        *node_ptr = new (_pool) DeclIdentListAST(ident);
        node_ptr = &(*node_ptr)->next;
    }

    return node;
}

TypeAST *Parser::parseType()
{
    TypeAST *typ = tryType();

    if (!typ) {
        unsigned pos = tokenIndex;
        _translationUnit->error(pos, "expected type");
        next();     // make progress
        return new (_pool) BadTypeAST(pos, tokenIndex);
    }

    return typ;
}

ExprListAST *Parser::parseTypeList()
{
    ExprListAST *list = nullptr;
    ExprListAST **list_ptr = &list;

    if (ExprAST *x = parseType()) {
        *list_ptr = new (_pool) ExprListAST(x);
        list_ptr = &(*list_ptr)->next;
    }
    while (tok.kindAndPos.kind == COMMA) {
        next();
        if (ExprAST *x = parseType()) {
            *list_ptr = new (_pool) ExprListAST(x);
            list_ptr = &(*list_ptr)->next;
        }
    }

    return list;
}

TypeAST *Parser::parseTypeName()
{
    if (tok.kindAndPos.kind == IDENT) {
        const Identifier *id1 = tok.identifier;
        unsigned t1 = next();
        if (tok.kindAndPos.kind != PERIOD) {
            TypeIdentAST *typeIdent = new (_pool) TypeIdentAST(new (_pool) IdentAST(t1, id1));
            typeIdent->usingScope = topScope;
            return typeIdent;
        }
        unsigned t_dot = next();
        const Identifier *id2 = nullptr;
        if (tok.kindAndPos.kind != IDENT) {
            _translationUnit->error(tokenIndex, "expected identifier");
            id2 = _control->underscoreIdentifier();
        } else {
            id2 = tok.identifier;
        }
        PackageTypeAST *pkgType =  new (_pool) PackageTypeAST(new (_pool) IdentAST(t1, id1),
                                                              t_dot,
                                                              new (_pool) IdentAST(next(), id2));
        pkgType->fileScope = fileScope;
        return pkgType;
    }

    _translationUnit->error(tokenIndex, "expected identifier");
    return new (_pool) TypeIdentAST(new (_pool) IdentAST(next(), _control->underscoreIdentifier()));
}

TypeAST *Parser::parseArrayType()
{
    unsigned t_lbracket = expect(LBRACK);

    exprLev++;
    ExprAST *len = nullptr;
    if (tok.kindAndPos.kind == ELLIPSIS) {
        len = new (_pool) EllipsisAST(next());
    } else if (tok.kindAndPos.kind != RBRACK) {
        len = parseRhs();
    }
    exprLev--;

    unsigned t_rbracket = expect(RBRACK);

    TypeAST *ellipsisElement = parseType();

    return new (_pool) ArrayTypeAST(t_lbracket, len, t_rbracket, ellipsisElement);
}

TypeAST *Parser::parseStructType()
{
    unsigned pos = expect(STRUCT);
    unsigned lbrace = expect(LBRACE);

    FieldListAST *list = nullptr;
    FieldListAST **list_ptr = &list;

    openStructScope();

    while (tok.kindAndPos.kind == IDENT || tok.kindAndPos.kind == MUL || tok.kindAndPos.kind == LPAREN) {
        if (FieldAST *field = parseStructFieldDecl()) {
            *list_ptr = new (_pool) FieldListAST(field);
            list_ptr = &(*list_ptr)->next;
        }
    }

    unsigned rbrace = expect(RBRACE);

    FieldGroupAST *fields = new (_pool) FieldGroupAST(lbrace, list, rbrace);
    StructTypeAST *structType = new (_pool) StructTypeAST(pos, fields);

    structScope->setAst(structType);
    closeStructScope();

    return structType;
}

StarTypeAST *Parser::parsePointerType()
{
    unsigned t_star = expect(MUL);
    TypeAST *x = parseType();

    return new (_pool) StarTypeAST(t_star, x);
}

QPair<FuncTypeAST *, Scope *> Parser::parseFuncType()
{
    unsigned pos = expect(FUNC);

    Scope *scope = _control->newScope(topScope);

    FieldGroupAST *params = nullptr;
    FieldGroupAST *results = nullptr;
    parseSignature(params, results, scope);

    return qMakePair(new (_pool) FuncTypeAST(pos, params, results, results ? results->callType(_pool) : nullptr), scope);
}

InterfaceTypeAST *Parser::parseInterfaceType()
{
    unsigned pos = expect(INTERFACE);

    unsigned lbrace = expect(LBRACE);

    Scope *scope = _control->newScope(fileScope);

    FieldListAST *list = nullptr;
    FieldListAST **list_ptr = &list;

    while (tok.kindAndPos.kind == IDENT) {
        if (FieldAST *field = parseMethodSpec(scope)) {
            *list_ptr = new (_pool) FieldListAST(field);
            list_ptr = &(*list_ptr)->next;
        }
    }

    unsigned rbrace = expect(RBRACE);

    FieldGroupAST *methods = new (_pool) FieldGroupAST(lbrace, list, rbrace);
    return new (_pool) InterfaceTypeAST(pos, methods);
}

MapTypeAST *Parser::parseMapType()
{
    unsigned pos  = expect(MAP);
    expect(LBRACK);
    TypeAST *key = parseType();
    expect(RBRACK);
    TypeAST *value = parseType();

    return new (_pool) MapTypeAST(pos, key, value);
}

ChanTypeAST *Parser::parseChanType()
{
    unsigned pos = tokenIndex;
    ChanTypeAST::ChanDir dir = ChanTypeAST::SEND_OR_RECV;
    unsigned arrow = 0;

    if (tok.kindAndPos.kind == CHAN) {
        next();
        if (tok.kindAndPos.kind == ARROW) {
            arrow = next();
            dir = ChanTypeAST::SEND;
        }
    } else {
        arrow = expect(ARROW);
        expect(CHAN);
        dir = ChanTypeAST::RECV;
    }
    TypeAST *value = parseType();

    ChanTypeAST *typ = new (_pool) ChanTypeAST(pos, arrow, value);
    typ->dir = dir;
    return typ;
}

FieldAST *Parser::parseMethodSpec(Scope *scope)
{
    CommentGroupAST *doc = leadComment;

    DeclIdentListAST *idents = nullptr;
    TypeAST *typ = nullptr;

    TypeAST *x = parseTypeName();
    IdentAST *ident = x->asIdent();
    if (tok.kindAndPos.kind == LPAREN && ident) {
        idents = new (_pool) DeclIdentListAST(new (_pool) DeclIdentAST(ident->t_identifier, ident->ident));
        scope = _control->newScope(fileScope);
        FieldGroupAST *params = nullptr;
        FieldGroupAST *results = nullptr;
        parseSignature(params, results, scope);
        FuncTypeAST *funcTyp = new (_pool) FuncTypeAST(0, params, results, results ? results->callType(_pool) : nullptr);
        declareFunc(funcTyp, scope, idents);
        typ = funcTyp;
    } else {
        typ = x;
    }

    expectSemi();

    FieldAST *spec = new (_pool) FieldAST(doc, idents, typ, nullptr, lineComment);
    return spec;
}

void Parser::parseSignature(FieldGroupAST *&params, FieldGroupAST *&results, Scope *scope)
{
    params = parseParameters(scope, true);
    results = parseResult(scope);
}

FieldGroupAST *Parser::parseParameters(Scope *scope, bool ellipsisOk)
{
    unsigned lparen = expect(LPAREN);

    FieldListAST *params = nullptr;
    if (tok.kindAndPos.kind != RPAREN)
        params = parseParameterList(scope, ellipsisOk);

    unsigned rparen = expect(RPAREN);

    return new (_pool) FieldGroupAST(lparen, params, rparen);
}

FieldGroupAST *Parser::parseResult(Scope *scope)
{
    if (tok.kindAndPos.kind == LPAREN)
        return parseParameters(scope, false);

    if (TypeAST *typ = tryType()) {
        FieldListAST *params = nullptr;
        FieldListAST **params_ptr = &params;
        FieldAST *field = new (_pool) FieldAST(nullptr, nullptr, typ);
        *params_ptr = new (_pool) FieldListAST(field);
        return new (_pool) FieldGroupAST(0, params);
    }

    return nullptr;
}

FieldListAST *Parser::parseParameterList(Scope *scope, bool ellipsisOk)
{
    TypeListAST *list = nullptr;
    TypeListAST **list_ptr = &list;

    FieldListAST *params = nullptr;
    FieldListAST **params_ptr = &params;

    while (true) {
        if (TypeAST *field = parseVarType(ellipsisOk)) {
            *list_ptr = new (_pool) TypeListAST(field);
            list_ptr = &(*list_ptr)->next;
        }
        if (tok.kindAndPos.kind != COMMA)
            break;
        next();
        if (tok.kindAndPos.kind == RPAREN)
            break;
    }

    if (TypeAST *typ = tryVarType(ellipsisOk)) {
        DeclIdentListAST *idents = makeDeclIdentList(list);
        FieldAST *field = new (_pool) FieldAST(nullptr, idents, typ);
        *params_ptr = new (_pool) FieldListAST(field);
        params_ptr = &(*params_ptr)->next;
        declareVar(typ, scope, idents);
        if (!atComma(RPAREN))
            return params;
        next();
        while (tok.kindAndPos.kind != RPAREN && tok.kindAndPos.kind != T_EOF) {
            idents = parseDeclIdentList();
            typ = parseVarType(ellipsisOk);
            field = new (_pool) FieldAST(nullptr, idents, typ);
            *params_ptr = new (_pool) FieldListAST(field);
            params_ptr = &(*params_ptr)->next;
            declareVar(typ, scope, idents);
            if (!atComma(RPAREN))
                break;
            next();
        }
        return params;
    }

    for (TypeListAST *it = list; it; it = it->next) {
        if (TypeAST *typ = it->value) {
            FieldAST *field = new (_pool) FieldAST(nullptr, nullptr, typ);
            *params_ptr = new (_pool) FieldListAST(field);
            params_ptr = &(*params_ptr)->next;
        }
    }
    return params;
}

ExprAST *Parser::tryParseNewExpr(bool &isNewOrMakeExpr)
{
    const Identifier *id = tok.identifier;
    unsigned t_new = next();

    if (tok.kindAndPos.kind != LPAREN)
        return new(_pool) IdentAST(t_new, id);

    isNewOrMakeExpr = true;
    unsigned t_lparen = next();
    TypeAST *typ = tryType();
    unsigned t_rparen = expect(RPAREN);
    return new(_pool) NewExprAST(t_new, t_lparen, typ, t_rparen);
}

ExprAST *Parser::tryParseMakeExpr(bool &isNewOrMakeExpr)
{
    const Identifier *id = tok.identifier;
    unsigned t_make = next();

    if (tok.kindAndPos.kind != LPAREN)
        return new(_pool) IdentAST(t_make, id);

    isNewOrMakeExpr = true;
    unsigned t_lparen = next();

    ExprListAST *args = nullptr;
    ExprListAST **args_ptr = &args;

    exprLev++;
    while (tok.kindAndPos.kind != RPAREN && tok.kindAndPos.kind != T_EOF) {
        if (ExprAST *arg = parseRhsOrType()) {
            *args_ptr = new (_pool) ExprListAST(arg);
            args_ptr = &(*args_ptr)->next;
        }
        if (!atComma(RPAREN))
            break;
        next();
    }
    exprLev--;

    unsigned t_rparen = expect(RPAREN);
    return new (_pool) MakeExprAST(t_make, t_lparen, args, t_rparen);
}

ExprAST *Parser::parseRhs()
{
    unsigned old  = inRhs;
    inRhs = true;
    ExprAST *x = checkExpr(parseExpr());
    inRhs = old;
    return x;
}

ExprAST *Parser::parseRhsOrType()
{
    unsigned old  = inRhs;
    inRhs = true;
    ExprAST *x = checkExprOrType(parseExpr());
    inRhs = old;
    return x;
}

ExprListAST *Parser::parseRhsList()
{
    bool old = inRhs;
    inRhs = true;
    ExprListAST *list = parseExprList();
    inRhs = old;
    return list;
}

ExprListAST *Parser::parseLhsList()
{
    bool old = inRhs;
    inRhs = false;
    ExprListAST *list = parseExprList();
    inRhs = old;
    return list;
}

ExprAST *Parser::parseExpr()
{
    return parseBinaryExpr(Token::LowestPrec + 1);
}

ExprListAST *Parser::parseExprList()
{
    ExprListAST *node = nullptr;
    ExprListAST **node_ptr = &node;

    if (ExprAST *x = checkExpr(parseExpr())) {
        *node_ptr = new (_pool) ExprListAST(x);
        node_ptr = &(*node_ptr)->next;
    }

    while (tok.kindAndPos.kind == COMMA) {
        next();
        if (ExprAST *x = checkExpr(parseExpr())) {
            *node_ptr = new (_pool) ExprListAST(x);
            node_ptr = &(*node_ptr)->next;
        }
    }

    return node;
}

ExprAST *Parser::parseBinaryExpr(int prec1)
{
    ExprAST *x = parseUnaryExpr();

    TokenKind k;
    int prec;
    for (tokPrec(k, prec); prec >= prec1; prec--) {
        while (true) {
            TokenKind op;
            int oprec;
            tokPrec(op, oprec);
            if (oprec != prec)
                break;
            unsigned t_op = tokenIndex;
            next();
            ExprAST *y = parseBinaryExpr(prec + 1);
            x = op == ADD ? new (_pool) BinaryPlusExprAST(checkExpr(x), t_op, checkExpr(y))
                          : new (_pool) BinaryExprAST(checkExpr(x), t_op, checkExpr(y));
        }
    }

    return x;
}

ExprAST *Parser::parseUnaryExpr()
{
    switch (tok.kindAndPos.kind) {
        case ADD: case SUB: case NOT: case XOR: {
            unsigned t_op = next();
            ExprAST *x = parseUnaryExpr();
            return new (_pool) UnaryExprAST(t_op, checkExpr(x));
        }
        case AND: {
            unsigned t_op = next();
            ExprAST *x = parseUnaryExpr();
            return new (_pool) RefUnaryExprAST(t_op, checkExpr(x));
        }
        case MUL: {
            unsigned t_star = next();
            ExprAST *x = parseUnaryExpr();
            return new (_pool) StarExprAST(t_star, checkExprOrType(x));
        }

        case ARROW: {
            unsigned arrow = next();

            ExprAST *x = parseUnaryExpr();

            if (ChanTypeAST *typ = x->asChanType()) {
                // (<-type)
                ChanTypeAST::ChanDir dir = ChanTypeAST::SEND;
                while (typ && dir == ChanTypeAST::SEND) {
                    if (typ->dir == ChanTypeAST::RECV) {
                        // error: (<-type) is (<-(<-chan T))
                        _translationUnit->error(typ->t_arrow, "expected 'chan'");
                    }
                    arrow = typ->t_arrow;
                    typ->t_chan = arrow;
                    typ->t_arrow = arrow;
                    typ = typ->value->asChanType();
                }
                if (dir == ChanTypeAST::SEND) {
                    _translationUnit->error(arrow, "expected channel type");
                }
                return x;
            }

            // <-(expr)
            return new (_pool) ArrowUnaryExprAST(arrow, checkExpr(x));
        }
    }

    return parsePrimaryExpr();
}

ExprAST *Parser::parsePrimaryExpr()
{
    bool isNewOrMakeExpr = false;
    ExprAST *x = parseOperand(isNewOrMakeExpr);
    if (isNewOrMakeExpr)
        return x;

    while (true) {
        switch (tok.kindAndPos.kind) {
            case PERIOD:
                next();
                switch (tok.kindAndPos.kind) {
                    case IDENT:
                        x = parseSelector(checkExprOrType(x));
                        break;
                    case LPAREN:
                        x = parseTypeAssertion(checkExpr(x));
                        break;
                    default:
                        _translationUnit->error(tokenIndex, "expected selector or type assertion");
                        next(); // make progress
                        x = new (_pool) SelectorExprAST(x);
                        break;
                }
                break;
            case LBRACK:
                x = parseIndexOrSlice(checkExpr(x));
                break;
            case LPAREN:
                x = parseCallOrConversion(checkExprOrType(x));
                break;
            case LBRACE:
                if (isLiteralType(x) && (exprLev >= 0 || !isTypeName(x))) {
                    x = parseLiteralValue(x);
                } else {
                    return x;
                }
                break;
            default:
                return x;
        }
    }
}

ExprAST *Parser::parseOperand(bool &isNewOrMakeExpr)
{
    switch (tok.kindAndPos.kind) {
        case IDENT:
            if (tok.identifier == Control::newIdentifier())
                return tryParseNewExpr(isNewOrMakeExpr);
            if (tok.identifier == Control::makeIdentifier())
                return tryParseMakeExpr(isNewOrMakeExpr);
            return parseIdent();

        case INT: case FLOAT: case IMAG: case CHAR:
            return new (_pool) BasicLitAST(next());
        case STRING:
            return new (_pool) StringLitAST(next());

        case LPAREN: {
            unsigned t_lparen = next();
            exprLev++;
            ExprAST *x = parseRhsOrType();
            exprLev--;
            unsigned t_rparen = expect(RPAREN);
            return new (_pool) ParenExprAST(t_lparen, x, t_rparen);
        }

        case FUNC:
            return parseFuncTypeOrLit();
    }

    if (ExprAST *node = tryIdentOrType()) {
        if (node) {
            if (IdentAST *ident = node->asIdent())
                _translationUnit->warning(ident->t_identifier, "type cannot be identifier");
            return node;
        }
    }

    // we have an error
    unsigned t_start = tokenIndex;
    _translationUnit->error(t_start, "expected operand");
    syncStmt();
    return new (_pool) BadExprAST(t_start, tokenIndex);

}

ExprAST *Parser::parseSelector(ExprAST *x)
{
    return new (_pool) SelectorExprAST(x, parseIdent());
}

ExprAST *Parser::parseTypeAssertion(ExprAST *x)
{
    unsigned t_lparen =  next();

    TypeAST *typ = nullptr;
    if (tok.kindAndPos.kind == TYPE)
        next();
    else
        typ = parseType();

    unsigned t_rparen = expect(RPAREN);

    return new (_pool) TypeAssertExprAST(x, t_lparen, typ, t_rparen);
}

ExprAST *Parser::parseIndexOrSlice(ExprAST *x)
{
    const int N = 3;

    unsigned t_lbracket = next();

    ExprAST *index[N];
    for (int i = 0; i < N; i++)
        index[i] = nullptr;
    unsigned t_colons[N-1];

    exprLev++;

    if (tok.kindAndPos.kind != COLON)
        index[0] = parseRhs();

    int ncolons = 0;
    while (tok.kindAndPos.kind == COLON && ncolons < N - 1) {
        t_colons[ncolons++] = next();
        if (tok.kindAndPos.kind != COLON && tok.kindAndPos.kind != RBRACK && tok.kindAndPos.kind != T_EOF)
            index[ncolons] = parseRhs();
    }

    exprLev--;

    unsigned t_rbracket = expect(RBRACK);

    if (ncolons) {
        // slice expression
        bool slice3 = false;
        if (ncolons == 2) {
            slice3 = true;
            if (!index[1]) {
                _translationUnit->error(t_colons[0], "2nd index required in 3-index slice");
                index[1] = new (_pool) BadExprAST(t_colons[0] + 1, t_colons[1]);
            }
            if (!index[2]) {
                _translationUnit->error(t_colons[1], "3rd index required in 3-index slice");
                index[2] = new (_pool) BadExprAST(t_colons[1] + 1, t_rbracket);
            }
        }
        SliceExprAST *sliceAst = new (_pool) SliceExprAST(x, t_lbracket, index[0], index[1], index[2], t_rbracket);
        sliceAst->slice3 = slice3;
        return sliceAst;
    }

    return new (_pool) IndexExprAST(x, t_lbracket, index[0], t_rbracket);
}

CallExprAST *Parser::parseCallOrConversion(ExprAST *fun)
{
    unsigned t_lparen = next();

    unsigned t_ellipsis = 0;
    ExprListAST *args = nullptr;
    ExprListAST **args_ptr = &args;

    exprLev++;
    while (tok.kindAndPos.kind != RPAREN && tok.kindAndPos.kind != T_EOF && !t_ellipsis) {
        if (ExprAST *arg = parseRhsOrType()) {
            *args_ptr = new (_pool) ExprListAST(arg);
            args_ptr = &(*args_ptr)->next;
        }
        if (tok.kindAndPos.kind == ELLIPSIS)
            t_ellipsis = next();
        if (!atComma(RPAREN))
            break;
        next();
    }
    exprLev--;

    unsigned t_rparen = expect(RPAREN);

    return new (_pool) CallExprAST(fun, t_lparen, args, t_ellipsis, t_rparen);
}

ExprAST *Parser::parseLiteralValue(ExprAST *type)
{
    unsigned t_lbrace = next();

    ExprListAST *elements = nullptr;
    ExprListAST **elements_ptr = &elements;

    exprLev++;
    while (tok.kindAndPos.kind != RBRACE && tok.kindAndPos.kind != T_EOF) {
        if (ExprAST *element = parseElement()) {
            *elements_ptr = new (_pool) ExprListAST(element);
            elements_ptr = &(*elements_ptr)->next;
        }
        if (!atComma(RBRACE))
            break;
        next();
    }
    exprLev--;

    unsigned t_rbrace = expect(RBRACE);

    return new (_pool) CompositeLitAST(type, t_lbrace, elements, t_rbrace);
}

ExprAST *Parser::parseElement()
{
    ExprAST *x = parseValue();
    if (tok.kindAndPos.kind == COLON) {
        unsigned t_colon = next();
        x = new (_pool) KeyValueExprAST(x, t_colon, parseValue());
    }
    return x;
}

ExprAST *Parser::parseValue()
{
    if (tok.kindAndPos.kind == LBRACE)
        return parseLiteralValue(nullptr);

    return  checkExpr(parseExpr());
}

FieldAST *Parser::parseStructFieldDecl()
{
    CommentGroupAST *doc = leadComment;

    TypeListAST *list = nullptr;
    TypeListAST **list_ptr = &list;
    while (true) {
        if (TypeAST *ident = parseVarType(false)) {
            *list_ptr = new (_pool) TypeListAST(ident);
            list_ptr = &(*list_ptr)->next;
        }
        if (tok.kindAndPos.kind != COMMA)
            break;
        next();
    }

    TypeAST *typ = tryVarType(false);

    DeclIdentListAST *idents = nullptr;
    if (typ) {
        idents = makeDeclIdentList(list);
    } else {
        if (list) {
            typ = list->value;
            if (list->next) {
                _translationUnit->error(tokenIndex, "expected type");
                typ = new (_pool) BadTypeAST(tokenIndex, tokenIndex);
            } else if (!isTypeName(deref(typ))) {
                _translationUnit->error(typ->firstToken(), "expected anonymous field");
                typ = new (_pool) BadTypeAST(typ->firstToken(), typ->lastToken());
            }
        }
    }

    StringLitAST *tag = nullptr;
    if (tok.kindAndPos.kind == STRING)
        tag = new (_pool) StringLitAST(next());

    expectSemi();

    FieldAST *field = new (_pool) FieldAST(doc, idents, typ, tag, lineComment);
    declareFld(typ, structScope, idents);
    return field;
}

TypeAST *Parser::parseVarType(bool isParam)
{
    TypeAST *typ = tryVarType(isParam);
    if (!typ) {
        unsigned pos = next();
        _translationUnit->error(pos, "expected type");
        typ = new (_pool) BadTypeAST(pos, tokenIndex);
    }
    return typ;
}

BlockStmtAST *Parser::parseBody(Scope *scope)
{
    unsigned lbrace = next();

    topScope = scope;
    StmtListAST *list = parseStmtList();
    closeScope();
    unsigned rbrace = expect(RBRACE);

    BlockStmtAST *block = new (_pool) BlockStmtAST(lbrace, list, rbrace);
    scope->setAst(block);
    block->scope = scope;
    return block;
}

StmtListAST *Parser::parseStmtList()
{
    StmtListAST *list = nullptr;
    StmtListAST **list_ptr = &list;

    while (true) {
        switch (tok.kindAndPos.kind) {
            case CASE:
            case DEFAULT:
            case RBRACE:
            case T_EOF:
                return list;
        }

        if (StmtAST *stmt = parseStmt()) {
            *list_ptr = new (_pool) StmtListAST(stmt);
            list_ptr = &(*list_ptr)->next;
        }
    }
}

StmtAST *Parser::parseStmt()
{
    StmtAST *s = nullptr;

    switch (tok.kindAndPos.kind) {
        case CONST: case TYPE: case VAR:
            s = new (_pool) DeclStmtAST(parseDeclSyncStmt());
            break;

        case IDENT: case INT: case FLOAT: case IMAG: case CHAR: case STRING: case FUNC: case LPAREN:
        case LBRACK: case STRUCT: case MAP: case CHAN: case INTERFACE:
        case ADD: case SUB: case MUL: case AND: case XOR: case ARROW: case NOT:
            s = parseSimpleStmt(labelOk).first;
            if (!s->asLabeledStmt())
                expectSemi();
            break;

        case GO:
            s = parseGoStmt();
            break;

        case DEFER:
            s = parseDeferStmt();
            break;

        case RETURN:
            s = parseReturnStmt();
            break;

        case BREAK: case CONTINUE: case GOTO: case FALLTHROUGH:
            s = parseBranchStmt(static_cast<TokenKind>(tok.kindAndPos.kind));
            break;

        case LBRACE:
            s = parseBlockStmt();
            expectSemi();
            break;

        case IF:
            s = parseIfStmt();
            break;

        case SWITCH:
            s = parseSwitchStmt();
            break;

        case SELECT:
            s = parseSelectStmt();
            break;

        case FOR:
            s = parseForStmt();
            break;

        case SEMICOLON:
            s = new (_pool) EmptyStmtAST(next());
            break;

        case RBRACE:
            s = new (_pool) EmptyStmtAST(tokenIndex);
            break;

        default: {
            unsigned pos = tokenIndex;
            _translationUnit->error(pos, "expected statement");
            syncStmt();
            s = new (_pool) BadStmtAST(pos, tokenIndex);
            break;
        }
    }

    return s;
}

QPair<StmtAST *, bool> Parser::parseSimpleStmt(Parser::ParseSimpleStmtMode mode)
{
    ExprListAST *x = parseLhsList();

    if (!x || !x->value)
        return qMakePair(static_cast<StmtAST *>(nullptr), false);   // WTF

    switch (tok.kindAndPos.kind) {
        case DEFINE: case ASSIGN: case ADD_ASSIGN:
        case SUB_ASSIGN: case MUL_ASSIGN: case QUO_ASSIGN:
        case REM_ASSIGN: case AND_ASSIGN: case OR_ASSIGN:
        case XOR_ASSIGN: case SHL_ASSIGN: case SHR_ASSIGN: case AND_NOT_ASSIGN: {
            unsigned kind = tok.kindAndPos.kind;
            bool isRange = false;
            unsigned pos = next();
            ExprListAST *y = nullptr;
            ExprListAST **y_ptr = &y;
            if (mode == rangeOk && tok.kindAndPos.kind == RANGE && (kind == DEFINE || kind == ASSIGN)) {
                pos = next();
                isRange = true;
                *y_ptr = new (_pool) ExprListAST(new (_pool) UnaryExprAST(pos, parseRhs()));
            } else {
                y = parseRhsList();
            }
            StmtAST *stmt = nullptr;
            if (kind == DEFINE) {
                DefineStmtAST *as = new (_pool) DefineStmtAST(x, pos, y);
                if (!isRange)
                    shortVarDecl(x, y);
                stmt = as;
            } else {
                AssignStmtAST *as = new (_pool) AssignStmtAST(x, pos, y);
                stmt = as;
            }
            return qMakePair(stmt, isRange);
        }
    }

    if (x->next)
        _translationUnit->error(x->value->firstToken(), "expected 1 expression");

    ExprAST *xf = x->value;

    switch (tok.kindAndPos.kind) {
        case COLON: {
            unsigned colon = next();
            if (xf->asIdent() && mode == labelOk) {
                StmtAST *stmt = new (_pool) LabeledStmtAST(xf->asIdent(), colon, parseStmt());
                return qMakePair(stmt, false);
            }
            _translationUnit->error(colon, "illegal label declaration");
            StmtAST *stmt = new (_pool) BadStmtAST(xf->firstToken(), colon);
            return qMakePair(stmt, false);
        }

        case ARROW: {
            unsigned arrow = next();
            ExprAST *y = parseRhs();
            StmtAST *stmt = new (_pool) SendStmtAST(xf, arrow, y);
            return qMakePair(stmt, false);
        }

        case INC: case DEC: {
            unsigned pos = next();
            StmtAST *stmt = new (_pool) IncDecStmtAST(xf, pos);
            return qMakePair(stmt, false);
        }
    }

    StmtAST *stmt = new (_pool) ExprStmtAST(xf);
    return qMakePair(stmt, false);
}

StmtAST *Parser::parseGoStmt()
{
    unsigned pos = expect(GO);
    CallExprAST *call = parseCallExpr(pos, "go");
    expectSemi();
    if (!call)
        return new (_pool) BadStmtAST(pos, pos);
    return new (_pool) GoStmtAST(pos, call);
}

StmtAST *Parser::parseDeferStmt()
{
    unsigned pos = expect(DEFER);
    CallExprAST *call = parseCallExpr(pos, "defer");
    expectSemi();
    if (!call)
        return new (_pool) BadStmtAST(pos, pos);
    return new (_pool) DeferStmtAST(pos, call);
}

CallExprAST *Parser::parseCallExpr(unsigned pos, const char *callType)
{
    ExprAST *x = parseRhsOrType();
    if (CallExprAST *cx = x->asCallExpr())
        return cx;
    if (!x->asBadExpr())
        _translationUnit->error(pos + 1, "function must be invoked in %s statement", callType);
    return nullptr;
}

ReturnStmtAST *Parser::parseReturnStmt()
{
    unsigned pos = expect(RETURN);

    ExprListAST *x = nullptr;
    if (tok.kindAndPos.kind != SEMICOLON && tok.kindAndPos.kind != RBRACE)
        x = parseRhsList();
    expectSemi();

    return new (_pool) ReturnStmtAST(pos, x);
}

BranchStmtAST *Parser::parseBranchStmt(TokenKind kind)
{
    unsigned pos = expect(kind);

    IdentAST *label = nullptr;
    if (kind != FALLTHROUGH && tok.kindAndPos.kind == IDENT)
        label = parseIdent();

    expectSemi();

    return new (_pool) BranchStmtAST(pos, label);
}

BlockStmtAST *Parser::parseBlockStmt()
{
    unsigned lbrace = expect(LBRACE);
    openScope();
    Scope *scope = topScope;
    StmtListAST *list = parseStmtList();
    closeScope();
    unsigned rbrace = expect(RBRACE);

    BlockStmtAST *block = new (_pool) BlockStmtAST(lbrace, list, rbrace);
    scope->setAst(block);
    block->scope = scope;
    return block;
}

IfStmtAST *Parser::parseIfStmt()
{
    unsigned pos = expect(IF);

    openScope();

    StmtAST *s = nullptr;
    ExprAST *x = nullptr;
    {
        int prevLev = exprLev;
        exprLev = -1;
        if (tok.kindAndPos.kind == SEMICOLON) {
            next();
            x = parseRhs();
        } else {
            s = parseSimpleStmt(basic).first;
            if (tok.kindAndPos.kind == SEMICOLON) {
                next();
                x = parseRhs();
            } else {
                x = makeExpr(s, "boolean expression");
                s = nullptr;
            }
        }
        exprLev = prevLev;
    }

    BlockStmtAST *body = parseBlockStmt();

    StmtAST *else_ = nullptr;
    if (tok.kindAndPos.kind == ELSE) {
        next();
        switch (tok.kindAndPos.kind) {
            case IF:
                else_ = parseIfStmt();
                break;
            case LBRACE:
                else_ = parseBlockStmt();
                expectSemi();
                break;
            default:
                _translationUnit->error(tokenIndex, "expected if statement or block");
                else_ = new (_pool) BadStmtAST(tokenIndex, tokenIndex);
        }
    } else {
        expectSemi();
    }

    Scope *scope = topScope;
    closeScope();
    IfStmtAST *ifStmt = new (_pool) IfStmtAST(pos, s, x, body, else_);
    scope->setAst(ifStmt);
    ifStmt->scope = scope;
    return ifStmt;
}

StmtAST *Parser::parseSwitchStmt()
{
    unsigned pos = expect(SWITCH);

    openScope();

    StmtAST *s1 = nullptr;
    StmtAST *s2 = nullptr;
    if (tok.kindAndPos.kind != LBRACE) {
        int prevLev = exprLev;
        exprLev = -1;
        if (tok.kindAndPos.kind != SEMICOLON)
            s2 = parseSimpleStmt(basic).first;
        if (tok.kindAndPos.kind == SEMICOLON) {
            next();
            s1 = s2;
            s2 = nullptr;
            if (tok.kindAndPos.kind != LBRACE) {
                openScope();
                s2 = parseSimpleStmt(basic).first;
                closeScope();
            }
        }
        exprLev = prevLev;
    }

    IdentAST *declForTypeSwitch = nullptr;
    bool typeSwitch = isTypeSwitchGuard(s2, declForTypeSwitch);

    unsigned lbrace = expect(LBRACE);

    StmtListAST *list = nullptr;
    StmtListAST **list_ptr = &list;
    while (tok.kindAndPos.kind == CASE || tok.kindAndPos.kind == DEFAULT) {
        if (CaseClauseAST *cc = parseCaseClause(typeSwitch)) {
            *list_ptr = new (_pool) StmtListAST(cc);
            list_ptr = &(*list_ptr)->next;
        }
    }

    unsigned rbrace = expect(RBRACE);
    expectSemi();

    BlockStmtAST *body = new (_pool) BlockStmtAST(lbrace, list, rbrace);
    body->scope = topScope;

    Scope *scope = topScope;
    closeScope();

    if (typeSwitch) {
        TypeSwitchStmtAST *typeSwitchStmt = new (_pool) TypeSwitchStmtAST(pos, s1, s2, body);
        scope->setAst(typeSwitchStmt);
        typeSwitchStmt->scope = scope;
        if (declForTypeSwitch)
            declareInTypeSwitchCaseClauses(declForTypeSwitch, list);
        return typeSwitchStmt;
    }

    SwitchStmtAST *switchStmt = new (_pool) SwitchStmtAST(pos, s1, makeExpr(s2, "switch expression"), body);
    scope->setAst(switchStmt);
    switchStmt->scope = scope;
    return switchStmt;
}

CaseClauseAST *Parser::parseCaseClause(bool typeSwitch)
{
    unsigned pos = tokenIndex;

    ExprListAST *list = nullptr;
    if (tok.kindAndPos.kind == CASE) {
        next();
        if (typeSwitch)
            list = parseTypeList();
        else
            list = parseRhsList();
    } else {
        expect(DEFAULT);
    }

    unsigned colon = expect(COLON);

    openScope();
    Scope *scope = topScope;
    StmtListAST *body = parseStmtList();
    closeScope();

    CaseClauseAST *caseClause = new (_pool) CaseClauseAST(pos, list, colon, body);
    scope->setAst(caseClause);
    caseClause->scope = scope;
    return caseClause;
}

SelectStmtAST *Parser::parseSelectStmt()
{
    unsigned pos = expect(SELECT);
    unsigned lbrace = expect(LBRACE);

    StmtListAST *list = nullptr;
    StmtListAST **list_ptr = &list;

    while (tok.kindAndPos.kind == CASE || tok.kindAndPos.kind == DEFAULT) {
        if (CommClauseAST *c = parseCommClause()) {
            *list_ptr = new (_pool) StmtListAST(c);
            list_ptr = &(*list_ptr)->next;
        }
    }

    unsigned rbrace = expect(RBRACE);
    expectSemi();

    BlockStmtAST *body = new (_pool) BlockStmtAST(lbrace, list, rbrace);

    return new (_pool) SelectStmtAST(pos, body);
}

CommClauseAST *Parser::parseCommClause()
{
    unsigned pos = tokenIndex;

    openScope();

    StmtAST *comm = nullptr;
    if (tok.kindAndPos.kind == CASE) {
        next();
        ExprListAST *lhs = parseLhsList();

        if (!lhs || !lhs->value) {  // WTF
            closeScope();
            return nullptr;
        }

        if (tok.kindAndPos.kind == ARROW) {
            if (lhs->next)
                _translationUnit->error(lhs->firstToken(), "expected 1 expression");
            unsigned arrow = next();
            ExprAST *rhs = parseRhs();
            comm = new (_pool) SendStmtAST(lhs->value, arrow, rhs);
        } else {
            unsigned kind = tok.kindAndPos.kind;
            if (kind == ASSIGN || kind == DEFINE) {
                if (lhs->next && lhs->next->next) {
                    _translationUnit->error(lhs->firstToken(), "expected 1 or 2 expressions");
                    lhs->next->next = nullptr;
                }
                unsigned pp = next();
                ExprAST *rhs = parseRhs();
                if (kind == DEFINE) {
                    DefineStmtAST *as = new (_pool) DefineStmtAST(lhs, pp, new (_pool) ExprListAST(rhs));
                    shortVarDecl(lhs, as->rhs);
                    comm = as;
                } else {
                    AssignStmtAST *as = new (_pool) AssignStmtAST(lhs, pp, new (_pool) ExprListAST(rhs));
                    comm = as;
                }
            } else {
                if (lhs->next)
                    _translationUnit->error(lhs->firstToken(), "expected 1 expression");
                comm = new (_pool) ExprStmtAST(lhs->value);
            }
        }
    } else {
        expect(DEFAULT);
    }

    unsigned colon = expect(COLON);
    StmtListAST *body = parseStmtList();

    closeScope();

    return new (_pool) CommClauseAST(pos, comm, colon, body);
}

StmtAST *Parser::parseForStmt()
{
    unsigned pos = expect(FOR);

    openScope();

    StmtAST *s1 = nullptr;
    StmtAST *s2 = nullptr;
    StmtAST *s3 = nullptr;
    bool isRange = false;

    if (tok.kindAndPos.kind != LBRACE) {
        int prevLev = exprLev;
        exprLev = -1;
        if (tok.kindAndPos.kind != SEMICOLON) {
            if (tok.kindAndPos.kind == RANGE) {
                unsigned pp = next();
                ExprAST *y = new (_pool) UnaryExprAST(pp, parseRhs());
                s2 = new (_pool) AssignStmtAST(nullptr, 0, new (_pool) ExprListAST(y));
                isRange = true;
            } else {
                QPair<StmtAST *, bool> r = parseSimpleStmt(rangeOk);
                s2 = r.first;
                isRange = r.second;
            }
        }
        if (!isRange && tok.kindAndPos.kind == SEMICOLON) {
            next();
            s1 = s2;
            s2 = nullptr;
            if (tok.kindAndPos.kind != SEMICOLON)
                s2 = parseSimpleStmt(basic).first;
            expectSemi();
            if (tok.kindAndPos.kind != LBRACE)
                s3 = parseSimpleStmt(basic).first;
        }
        exprLev = prevLev;
    }

    BlockStmtAST *body = parseBlockStmt();
    expectSemi();

    Scope *scope = topScope;
    closeScope();

    if (isRange) {
        AssignStmtAST *as = s2->asAssignStmt();
        ExprAST *key = nullptr;
        ExprAST *value = nullptr;
        if (!as->lhs) {
            // nothing to do
        } else if (!as->lhs->next) {
            key = as->lhs->value;
        } else if (!as->lhs->next->next) {
            key = as->lhs->value;
            value = as->lhs->next->value;
        } else {
            _translationUnit->error(as->lhs->next->next->firstToken(), "expected at most 2 expressions");
            return new (_pool) BadStmtAST(as->firstToken(), as->lastToken());
        }
        ExprAST *x = as->rhs->value->asUnaryExpr()->x;
        declareRange(key, value, scope, x);
        RangeStmtAST *rangeStmt = new (_pool) RangeStmtAST(pos, key, value, as->t_assign, x, body);
        scope->setAst(rangeStmt);
        rangeStmt->scope = scope;
        return rangeStmt;
    }

    ForStmtAST *forStmt = new (_pool) ForStmtAST(pos, s1, makeExpr(s2, "boolean or range expression"), s3, body);
    scope->setAst(forStmt);
    forStmt->scope = scope;
    return forStmt;
}

ExprAST *Parser::parseFuncTypeOrLit()
{
    QPair<FuncTypeAST *, Scope *> pf = parseFuncType();
    if (tok.kindAndPos.kind != LBRACE)
        return pf.first;

    exprLev++;
    BlockStmtAST *body = parseBody(pf.second);
    exprLev--;

    return new (_pool) FuncLitAST(pf.first, body);
}

TypeAST *Parser::tryType()
{ return tryIdentOrType(); }

TypeAST *Parser::tryIdentOrType()
{
    switch (tok.kindAndPos.kind) {
        case IDENT:
            return parseTypeName();
        case LBRACK:
            return parseArrayType();
        case STRUCT:
            return parseStructType();
        case MUL:
            return parsePointerType();
        case FUNC:
            return parseFuncType().first;
        case INTERFACE:
            return parseInterfaceType();
        case MAP:
            return parseMapType();
        case CHAN: case ARROW:
            return parseChanType();
        case LPAREN: {
            unsigned lparen = next();
            TypeAST *typ = parseType();
            unsigned rparen = expect(RPAREN);
            return new (_pool) ParenTypeAST(lparen, typ, rparen);
        }
    }

    return nullptr;
}

TypeAST *Parser::tryVarType(bool isParam)
{
    if (isParam && tok.kindAndPos.kind == ELLIPSIS) {
        unsigned pos = next();
        TypeAST *typ = tryIdentOrType();
        if (!typ) {
            _translationUnit->error(pos, "expected '...' parameter is missing type");
            typ = new (_pool) BadTypeAST(pos, tokenIndex);
        }
        return new (_pool) EllipsisTypeAST(pos, typ);
    }

    return tryIdentOrType();
}

void Parser::declareVar(TypeAST *typ, Scope *scope, DeclIdentAST *ident)
{
    if (!ident || ident->ident == _control->underscoreIdentifier())
        return;

    if (scope->find(ident->ident)) {
        _translationUnit->error(ident->t_identifier, "variable redeclaration");
        return;
    }

    ident->symbol = _control-> newVarDecl(ident->t_identifier, ident->ident, typ, scope);
    scope->addMember(ident->symbol);
}

void Parser::declareVar(TypeAST *typ, Scope *scope, DeclIdentListAST *idents)
{
    for (DeclIdentListAST *it = idents; it; it = it->next)
        declareVar(typ, scope, it->value);
}

void Parser::declareFld(TypeAST *typ, Scope *scope, DeclIdentAST *field)
{
    if (!field || field->ident == _control->underscoreIdentifier())
        return;

    if (scope->find(field->ident)) {
        _translationUnit->error(field->t_identifier, "field redeclaration");
        return;
    }

    field->symbol = _control-> newFieldDecl(field->t_identifier, field->ident, typ, scope);
    scope->addMember(field->symbol);
}

void Parser::declareFld(TypeAST *typ, Scope *scope, DeclIdentListAST *fields)
{
    for (DeclIdentListAST *it = fields; it; it = it->next)
        declareFld(typ, scope, it->value);
}

void Parser::declareType(TypeSpecAST *typ, Scope *scope, DeclIdentAST *ident)
{
    if (!ident || ident->ident == _control->underscoreIdentifier())
        return;

    if (scope->find(ident->ident)) {
        _translationUnit->error(ident->t_identifier, "type redeclaration");
        return;
    }

    ident->symbol = _control->newTypeDecl(ident->t_identifier, ident->ident, typ, scope);
    scope->addMember(ident->symbol);
}

void Parser::declareConst(Scope *scope, DeclIdentAST *ident, ExprAST *value)
{
    if (!ident || ident->ident == _control->underscoreIdentifier())
        return;

    if (scope->find(ident->ident)) {
        _translationUnit->error(ident->t_identifier, "constant redeclaration");
        return;
    }

    ident->symbol = _control->newConstDecl(ident->t_identifier, ident->ident, value, scope);
    scope->addMember(ident->symbol);
}

void Parser::declareConst(Scope *scope, DeclIdentListAST *idents, ExprListAST *values)
{
    ExprListAST *values_it = values;
    for (DeclIdentListAST *it = idents; it; it = it->next) {
        if (values_it) {
            declareConst(scope, it->value, values_it->value);
            values_it = values_it->next;
        } else {
            declareConst(scope, it->value, nullptr);
        }
    }
}

void Parser::declareFunc(FuncTypeAST *typ, Scope *scope, DeclIdentAST *ident)
{
    if (!ident || ident->ident == _control->underscoreIdentifier())
        return;

    if (scope->find(ident->ident)) {
        _translationUnit->error(ident->t_identifier, "function redeclaration");
        return;
    }

    ident->symbol = _control->newFuncDecl(ident->t_identifier, ident->ident, typ, scope);
    scope->addMember(ident->symbol);
}

void Parser::declareFunc(FuncTypeAST *typ, Scope *scope, DeclIdentListAST *idents)
{
    for (DeclIdentListAST *it = idents; it; it = it->next)
        declareFunc(typ, scope, it->value);
}

void Parser::declareMethod(FuncDeclAST *ast)
{
    if (ast->recv->fields && ast->recv->fields && !ast->recv->fields->next && ast->name) {
        if (FieldAST *recvField = ast->recv->fields->value) {
            if (TypeAST *typ = recvField->type) {
                if (StarTypeAST *starType = typ->asStarType())
                    typ = starType->typ;
                if (TypeIdentAST *typeIdent = typ->asTypeIdent()) {
                    fileScope->declareMethod(typeIdent->ident->ident, ast);
                    ast->symbol = _control->newMethod(typeIdent, ast, fileScope);
                    ast->name->symbol = ast->symbol;
                    if (ast->type->results)
                        ast->type->_callType = ast->type->results->callType(_pool);
                }
            }
        }
    }
}

void Parser::declareInTypeSwitchCaseClauses(IdentAST *decl, StmtListAST *cases)
{
    for (StmtListAST *cases_it = cases; cases_it; cases_it = cases_it->next) {
        if (CaseClauseAST *caseClause = cases_it->value->asCaseClause()) {
            if (caseClause->list) {
                if (TypeAST *typ = caseClause->list->value->asType()) {
                    VarDecl *symbol = new VarDecl(decl->t_identifier, decl->ident, typ, caseClause->scope);
                    caseClause->scope->addMember(symbol);
                }
            }
        }
    }
}

void Parser::shortVarDecl(ExprListAST *lhs, ExprListAST *rhs)
{
    int n = 0;
    unsigned index = 0;
    RhsExprListAST *rhsExprList = new (_pool) RhsExprListAST(rhs);

    for (ExprListAST *it = lhs; it; it = it->next, index++) {
        if (ExprAST *x = it->value) {
            if (IdentAST *ident = x->asIdent()) {
                if (ident->isLookable()) {
                    if (!topScope->find(ident->ident)) {
                        DeclIdentAST *declIdent = new (_pool) DeclIdentAST(ident->t_identifier, ident->ident);
                        declIdent->symbol = _control->newShortVarDecl(ident->t_identifier, ident->ident,
                                                                      rhsExprList, index, topScope);
                        it->value = declIdent;
                        topScope->addMember(declIdent->symbol);
                        n++;
                    }
                } else if (ident->ident != Control::underscoreIdentifier()) {
                    n++;
                }
            } else {
                _translationUnit->error(x->firstToken(), "expected identifier on left side of :=");
            }
        }
    }

    if (!n && lhs)
        _translationUnit->error(lhs->firstToken(), "no new variables on left side of :=");
}

void Parser::shortVarDecl(DeclIdentListAST *lhs, ExprListAST *rhs)
{
    unsigned index = 0;
    RhsExprListAST *rhsExprList = new (_pool) RhsExprListAST(rhs);

    for (DeclIdentListAST *it = lhs; it; it = it->next, index++) {
        if (DeclIdentAST *ident = it->value) {
            if (ident->isLookable()) {
                if (!topScope->find(ident->ident)) {
                    ident->symbol = _control->newShortVarDecl(ident->t_identifier, ident->ident,
                                                              rhsExprList, index, topScope);
                    topScope->addMember(ident->symbol);
                } else {
                    _translationUnit->error(ident->t_identifier, "variable redeclaration");
                }
            }
        }
    }
}

void Parser::declareRange(ExprAST *&key, ExprAST *&value, Scope *scope, ExprAST *range)
{
    IdentAST *keyIdent = key ? key->asIdent() : nullptr;
    IdentAST *valueIdent = value ? value->asIdent() : nullptr;

    if (!keyIdent && !valueIdent)
        return;

    RangeExpAST *x = new (_pool) RangeExpAST(range);

    if (keyIdent && keyIdent->isLookable()) {
        if (!scope->find(keyIdent->ident)) {
            DeclIdentAST *keyDecl = new (_pool) DeclIdentAST(keyIdent->t_identifier, keyIdent->ident);
            keyDecl->symbol = _control->newRangeKeyDecl(keyIdent->t_identifier, keyIdent->ident, x, scope);
            key = keyDecl;
            scope->addMember(keyDecl->symbol);
        } else {
            _translationUnit->error(keyIdent->t_identifier, "variable redeclaration");
        }
    }

    if (valueIdent && valueIdent->isLookable()) {
        if (!scope->find(valueIdent->ident)) {
            DeclIdentAST *valueDecl = new (_pool) DeclIdentAST(valueIdent->t_identifier, valueIdent->ident);
            valueDecl->symbol = _control->newRangeValueDecl(valueIdent->t_identifier, valueIdent->ident, x, scope);
            value = valueDecl;
            scope->addMember(valueDecl->symbol);
        } else {
            _translationUnit->error(valueIdent->t_identifier, "variable redeclaration");
        }
    }
}

ExprAST *Parser::checkExpr(ExprAST *x)
{
    ExprAST *unpe = unparen(x);
    if (TypeAST *te = unpe->asType()) {
        if (!unpe->asTypeAssertExpr()) {
            _translationUnit->error(te->firstToken(), "expected expression");
            return new (_pool) BadExprAST(x->firstToken(), x->lastToken());
        }
    }
    return x;
}

ExprAST *Parser::checkExprOrType(ExprAST *x)
{
    ExprAST *unpe = unparen(x);
    if (ArrayTypeAST *at = unpe->asArrayType()) {
        if (at->len) {
            if (EllipsisAST *et = at->len->asEllipsis()) {
                _translationUnit->error(et->firstToken(), "expected array length, found '...'");
                x = new (_pool) BadExprAST(unpe->firstToken(), unpe->lastToken());
            }
        }
    }

    return x;
}

ExprAST *Parser::makeExpr(StmtAST *s, const char *kind)
{
    if (!s)
        return nullptr;

    if (ExprStmtAST *es = s->asExprStmt())
        return checkExpr(es->x);

    _translationUnit->error(s->firstToken(),
                            "expected %s, found simple statement (missing parentheses around composite literal?)", kind);
    return new (_pool) BadExprAST(s->firstToken(), s->lastToken());
}

ExprAST *Parser::unparen(ExprAST *x)
{
    if (ParenExprAST *pe = x->asParenExpr())
        return unparen(pe->x);
    return x;
}

ExprAST *Parser::deref(ExprAST *x)
{
    if (StarTypeAST *star = x->asStarType())
        x= star->typ;
    return x;
}

DeclIdentListAST *Parser::makeDeclIdentList(TypeListAST *list)
{
    DeclIdentListAST *idents = nullptr;
    DeclIdentListAST **idents_ptr = &idents;

    for (TypeListAST *it = list; it; it = it->next) {
        if (TypeAST *x = it->value) {
            DeclIdentAST *ident = nullptr;
            if (IdentAST *id = x->asIdent()) {
                ident = new (_pool) DeclIdentAST(id->t_identifier, id->ident);
            } else {
                if (!x->asBadType())
                    _translationUnit->error(x->firstToken(), "expected identifier");
                ident = new (_pool) DeclIdentAST(x->firstToken(), _control->underscoreIdentifier());
            }
            *idents_ptr = new (_pool) DeclIdentListAST(ident);
            idents_ptr = &(*idents_ptr)->next;
        }
    }

    return idents;
}

void Parser::tokPrec(TokenKind &kind, int &prec)
{
    kind = static_cast<TokenKind>(tok.kindAndPos.kind);
    if (inRhs && kind == ASSIGN)
        kind = EQL;
    prec = Token::precedence(kind);
}

bool Parser::atComma(TokenKind follow)
{
    if (tok.kindAndPos.kind == COMMA)
        return true;

    if (tok.kindAndPos.kind != follow) {
        _translationUnit->error(tokenIndex, "missing ','");
        return true;
    }

    return false;
}

bool Parser::isLiteralType(ExprAST *node)
{
    if (node->asBadExpr() || node->asBadType())
        return true;
    if (node->asIdent())
        return true;
    if (SelectorExprAST *se = node->asSelectorExpr())
        return se->x->asIdent();
    if (TypeAST *t = node->asType())
        return t->isValidCompositeLiteralType();
    return false;
}

bool Parser::isTypeName(ExprAST *node)
{
    if (node->asBadExpr() || node->asBadType())
        return true;
    if (node->asIdent())
        return true;
    if (node->asPackageType())
        return true;
    if (SelectorExprAST *se = node->asSelectorExpr())
        return se->x->asIdent();
    return false;
}

bool Parser::isTypeSwitchGuard(StmtAST *s, IdentAST *&decl)
{
    if (s) {
        if (ExprStmtAST *es = s->asExprStmt())
            return isTypeSwitchAssert(es->x);

        if (AssignStmtAST *t = s->asAssignStmt()) {
            if (t->lhs && !t->lhs->next && t->rhs && !t->rhs->next && isTypeSwitchAssert(t->rhs->value)) {
                const Token &tk = _tokens->at(t->t_assign);
                switch (tk.kindAndPos.kind) {
                    case ASSIGN:
                        _translationUnit->warning(t->t_assign, "expected ':=', found '='");
                        decl = t->lhs->value->asIdent();
                        return true;
                    case DEFINE:
                        decl = t->lhs->value->asIdent();
                        return true;
                }
            }
        }
    }

    return false;
}

bool Parser::isTypeSwitchAssert(ExprAST *x)
{
    if (TypeAssertExprAST *a = x->asTypeAssertExpr())
        return !a->typ;
    return false;
}

unsigned Parser::skipBlock()
{
    unsigned level = 1;

    while (true) {
        switch (tok.kindAndPos.kind) {
            case T_EOF:
                return 0;
            case LBRACE:
                level++;
                break;
            case RBRACE:
                level--;
                if (!level)
                    return next();
                break;
        }
        next();
    }
}

void Parser::openScope()
{ topScope = _control->newScope(topScope); }

void Parser::closeScope()
{ topScope = topScope->outer(); }

void Parser::openStructScope()
{
    Scope *outer = structScope ? structScope : topScope;
    structScope = _control->newStructScope(outer);
}

void Parser::closeStructScope()
{
    structScope = structScope->outer();
    if (structScope == topScope)
        structScope = nullptr;
}

unsigned Parser::next()
{
    unsigned result = tokenIndex;

    leadComment = nullptr;
    lineComment = nullptr;

    unsigned prevLineNumber = tok.kindAndPos.line;
    if (tok.kindAndPos.kind == SEMICOLON) {
        // may be generated
        prevLineNumber = _tokens->at(tokenIndex - 1).kindAndPos.line;
    }
    next0();

    if (tok.kindAndPos.kind == COMMENT) {
        CommentGroupAST *comment = nullptr;
        unsigned endline;

        if (prevLineNumber == tok.kindAndPos.line) {
            // The comment is on same line as the previous token; it
            // cannot be a lead comment but may be a line comment.
            consumeCommentGroup(0, comment, endline);
            if (tok.kindAndPos.line != endline) {
                // The next token is on a different line, thus
                // the last comment group is a line comment.
                lineComment = comment;
            }
        }

        // consume successor comments, if any
        endline = 0;
        while (tok.kindAndPos.kind == COMMENT) {
            consumeCommentGroup(1, comment, endline);
        }

        if (endline == tok.kindAndPos.line) {
            // The next token is following on the line immediately after the
            // comment group, thus the last comment group is a lead comment.
            leadComment = comment;
        }
    }

    return result;
}

void Parser::next0()
{
    const char *te = _scanner.scan(&tok);
    _tokens->push_back(tok);
    tokenIndex++;
    if (te[0] != '\0')
        _translationUnit->error(tokenIndex, te);
}

void Parser::consumeCommentGroup(unsigned offset, CommentGroupAST *&node, unsigned &endline)
{
    endline = tok.kindAndPos.line;
    CommentGroupAST **node_ptr = &node;
    while (tok.kindAndPos.kind == COMMENT && tok.kindAndPos.line <= endline + offset) {
        CommentAST *comment = nullptr;
        consumeComment(comment, endline);
        if (comment) {
            *node_ptr = new (_pool) CommentGroupAST(comment);
            node_ptr = &(*node_ptr)->next;
        }
    }
}

void Parser::consumeComment(CommentAST *&node, unsigned &endline)
{
    node = new (_pool) CommentAST(tokenIndex);
    endline = _scanner.currentLine();
    next0();
}

unsigned Parser::expect(TokenKind kind)
{
    if (tok.kindAndPos.kind != kind)
        _translationUnit->error(tokenIndex, "expected token %s", Token::spell(kind));

    return next();     // make progress
}

unsigned Parser::expectStrict(TokenKind kind)
{
    if (tok.kindAndPos.kind != kind) {
        _translationUnit->error(tokenIndex, "expected token %s", Token::spell(kind));
        return 0;
    }

    return next();     // make progress
}

void Parser::expectSemi()
{
    switch (tok.kindAndPos.kind) {
        case RPAREN:
        case RBRACE:
            break;
        case SEMICOLON:
            next();
            break;
        case COMMA:
            _translationUnit->warning(tokenIndex, "expected new line or \";\", got \",\"");
            next();
            break;
        default:
            _translationUnit->error(tokenIndex, "expected new line or \";\"");
            syncStmt();
    }
}

void Parser::syncStmt()
{
    while (true) {
        switch (tok.kindAndPos.kind) {
            case BREAK: case CONST: case CONTINUE: case DEFER:
            case FALLTHROUGH: case FOR: case GO: case GOTO: case IF:
            case RETURN: case SELECT: case SWITCH: case TYPE: case VAR:
                if (tokenIndex == syncTokenIndex && syncCnt < 10) {
                    syncCnt++;
                    return;
                }
                if (tokenIndex > syncTokenIndex) {
                    syncTokenIndex = tokenIndex;
                    syncCnt = 0;
                    return;
                }
                break;
            case T_EOF:
                return;
        }
        next();
    }
}

void Parser::syncDecl()
{
    while (true) {
        switch (tok.kindAndPos.kind) {
            case CONST: case TYPE: case VAR:
                if (tokenIndex == syncTokenIndex && syncCnt < 10) {
                    syncCnt++;
                    return;
                }
                if (tokenIndex > syncTokenIndex) {
                    syncTokenIndex = tokenIndex;
                    syncCnt = 0;
                    return;
                }
                break;
            case T_EOF:
                return;
        }
        next();
    }
}

}   // namespace GoTools

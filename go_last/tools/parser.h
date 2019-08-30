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

#include "translationunit.h"
#include "scanner.h"
#include "astfwd.h"
#include "scope.h"

#include <QPair>

namespace GoTools {

class Parser
{
public:
    enum Mode {
        PackageClauseOnly = 0x01,               // stop parsing after package clause
        ImportsOnly = 0x02,                     // stop parsing after import declarations
        ParseComments = 0x04,                   // parse comments and add them to AST
        Trace = 0x08,                           // print a trace of parsed productions
        DeclarationErrors = 0x10,               // report declaration errors
        SpuriousErrors = 0x20,                  // same as AllErrors, for backward-compatibility
        AllErrors  = SpuriousErrors             // report all errors (not just the first 10 on different lines)
    };

    enum ParseSimpleStmtMode {
        basic = 0,
        labelOk,
        rangeOk
    };

    Parser(GoSource *source, unsigned mode = DeclarationErrors);

    FileAST *parseFile();
    FileAST *parsePackageFile(const QString &wantedPackageName);

private:
    DeclAST *parseDeclSyncDecl();
    DeclAST *parseDeclSyncStmt();
    GenDeclAST *parseGenDecl(TokenKind kind);
    SpecAST *parseGenDeclSpec(TokenKind kind, CommentGroupAST *doc, int index);
    SpecAST *parseGenDeclImportSpec(CommentGroupAST *doc);
    SpecAST *parseGenDeclConstSpec(CommentGroupAST *doc, int index);
    SpecAST *parseGenDeclVarSpec(CommentGroupAST *doc);
    SpecAST *parseGenDeclTypeSpec(CommentGroupAST *doc);
    FuncDeclAST *parseFuncDecl();
    IdentAST *parseIdent();
    DeclIdentAST *parseDeclIdent();
    IdentListAST *parseIdentList();
    DeclIdentListAST *parseDeclIdentList();
    TypeAST *parseType();
    ExprListAST *parseTypeList();
    TypeAST *parseTypeName();
    TypeAST *parseArrayType();
    TypeAST *parseStructType();
    StarTypeAST *parsePointerType();
    QPair<FuncTypeAST *, Scope *> parseFuncType();
    InterfaceTypeAST *parseInterfaceType();
    MapTypeAST *parseMapType();
    ChanTypeAST *parseChanType();
    FieldAST *parseMethodSpec(Scope *scope);
    void parseSignature(FieldGroupAST *&params, FieldGroupAST *&results, Scope *scope);
    FieldGroupAST *parseParameters(Scope *scope, bool ellipsisOk);
    FieldGroupAST *parseResult(Scope *scope);
    FieldListAST *parseParameterList(Scope *scope, bool ellipsisOk);
    ExprAST *tryParseNewExpr(bool &isNewOrMakeExpr);
    ExprAST *tryParseMakeExpr(bool &isNewOrMakeExpr);
    ExprAST *parseRhs();
    ExprAST *parseRhsOrType();
    ExprListAST *parseRhsList();
    ExprListAST *parseLhsList();
    ExprAST *parseExpr();
    ExprListAST *parseExprList();
    ExprAST *parseBinaryExpr(int prec1);
    ExprAST *parseUnaryExpr();
    ExprAST *parsePrimaryExpr();
    ExprAST *parseOperand(bool &isNewOrMakeExpr);
    ExprAST *parseSelector(ExprAST *x);
    ExprAST *parseTypeAssertion(ExprAST *x);
    ExprAST *parseIndexOrSlice(ExprAST *x);
    CallExprAST *parseCallOrConversion(ExprAST *fun);
    ExprAST *parseLiteralValue(ExprAST *type);
    ExprAST *parseElement();
    ExprAST *parseValue();
    FieldAST *parseStructFieldDecl();
    TypeAST *parseVarType(bool isParam);
    BlockStmtAST *parseBody(Scope *scope);
    StmtListAST *parseStmtList();
    StmtAST *parseStmt();
    QPair<StmtAST *, bool> parseSimpleStmt(ParseSimpleStmtMode mode);
    StmtAST *parseGoStmt();
    StmtAST *parseDeferStmt();
    CallExprAST *parseCallExpr(unsigned pos, const char *callType);
    ReturnStmtAST *parseReturnStmt();
    BranchStmtAST *parseBranchStmt(TokenKind kind);
    BlockStmtAST *parseBlockStmt();
    IfStmtAST *parseIfStmt();
    StmtAST *parseSwitchStmt();
    CaseClauseAST *parseCaseClause(bool typeSwitch);
    SelectStmtAST *parseSelectStmt();
    CommClauseAST *parseCommClause();
    StmtAST *parseForStmt();
    ExprAST *parseFuncTypeOrLit();

    TypeAST *tryType();
    TypeAST *tryIdentOrType();
    TypeAST *tryVarType(bool isParam);

    void declareVar(TypeAST *typ, Scope *scope, DeclIdentAST *ident);
    void declareVar(TypeAST *typ, Scope *scope, DeclIdentListAST *idents);

    void declareFld(TypeAST *typ, Scope *scope, DeclIdentAST *field);
    void declareFld(TypeAST *typ, Scope *scope, DeclIdentListAST *fields);

    void declareType(TypeSpecAST *typ, Scope *scope, DeclIdentAST *ident);

    void declareConst(Scope *scope, DeclIdentAST *ident, ExprAST *value);
    void declareConst(Scope *scope, DeclIdentListAST *idents, ExprListAST *values);

    void declareFunc(FuncTypeAST *typ, Scope *scope, DeclIdentAST *ident);
    void declareFunc(FuncTypeAST *typ, Scope *scope, DeclIdentListAST *idents);
    void declareMethod(FuncDeclAST *ast);

    void declareInTypeSwitchCaseClauses(IdentAST *decl, StmtListAST *cases);

    void shortVarDecl(ExprListAST *lhs, ExprListAST *rhs);
    void shortVarDecl(DeclIdentListAST *lhs, ExprListAST *rhs);
    void declareRange(ExprAST *&key, ExprAST *&value, Scope *scope, ExprAST *range);

    ExprAST *checkExpr(ExprAST *x);
    ExprAST *checkExprOrType(ExprAST *x);
    ExprAST *makeExpr(StmtAST *s, const char *kind);
    ExprAST *unparen(ExprAST *x);
    ExprAST *deref(ExprAST *x);
    DeclIdentListAST *makeDeclIdentList(TypeListAST *list);
    void tokPrec(TokenKind &kind, int &prec);
    bool atComma(TokenKind follow);
    bool isLiteralType(ExprAST *node);
    bool isTypeName(ExprAST *node);
    bool isTypeSwitchGuard(StmtAST *s, IdentAST *&decl);
    bool isTypeSwitchAssert(ExprAST *x);
    unsigned skipBlock();

    void openScope();
    void closeScope();

    void openStructScope();
    void closeStructScope();

    unsigned next();
    void next0();
    void consumeCommentGroup(unsigned offset, CommentGroupAST *&node, unsigned &endline);
    void consumeComment(CommentAST *&node, unsigned &endline);
    unsigned expect(TokenKind kind);
    unsigned expectStrict(TokenKind kind);
    void expectSemi();
    void syncStmt();
    void syncDecl();

private:
    GoSource *_source;
    TranslationUnit *_translationUnit;
    Control *_control;
    MemoryPool *_pool;
    const char *_firstSourceChar;
    const char *_lastSourceChar;
    unsigned _mode;
    Scanner _scanner;
    std::vector<Token> *_tokens;

    bool _trace;

    CommentGroupAST *leadComment;
    CommentGroupAST *lineComment;
    int exprLev;
    bool inRhs;
    unsigned syncTokenIndex;
    int syncCnt;
    Scope *topScope;
    Scope *structScope;
    FileScope *fileScope;

    Token tok;
    unsigned tokenIndex;
};

}   // namespace GoTools

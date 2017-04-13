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

#include "literals.h"

namespace GoTools {

class Control;

enum TokenKind {
    ILLEGAL = 0,
    T_EOF,
    COMMENT,

    literal_beg,
    IDENT = literal_beg,
    INT,
    FLOAT,
    IMAG,
    CHAR,
    STRING,
    literal_end = STRING,

    operator_beg,
    ADD = operator_beg,     // +
    SUB,                    // -
    MUL,                    // *
    QUO,                    // /
    REM,                    // %
    AND,                    // &
    OR,                     // |
    XOR,                    // ^
    SHL,                    // <<
    SHR,                    // >>
    AND_NOT,                // &^
    ADD_ASSIGN,             // +=
    SUB_ASSIGN,             // -=
    MUL_ASSIGN,             // *=
    QUO_ASSIGN,             // /=
    REM_ASSIGN,             // %=
    AND_ASSIGN,             // &=
    OR_ASSIGN,              // |=
    XOR_ASSIGN,             // ^=
    SHL_ASSIGN,             // <<=
    SHR_ASSIGN,             // >>=
    AND_NOT_ASSIGN,         // &^=
    LAND,                   // &&
    LOR,                    // ||
    ARROW,                  // <-
    INC,                    // ++
    DEC,                    // --
    EQL,                    // ==
    LSS,                    // <
    GTR,                    // >
    ASSIGN,                 // =
    NOT,                    // !
    NEQ,                    // !=
    LEQ,                    // <=
    GEQ,                    // >=
    DEFINE,                 // :=
    ELLIPSIS,               // ...
    LPAREN,                 // (
    LBRACK,                 // [
    LBRACE,                 // {
    COMMA,                  // ,
    PERIOD,                 // .
    RPAREN,                 // )
    RBRACK,                 // ]
    RBRACE,                 // }
    SEMICOLON,              // ;
    COLON,                  // :
    operator_end = COLON,

    keyword_beg,
    BREAK = keyword_beg,
    CASE,
    CHAN,
    CONST,
    CONTINUE,
    DEFAULT,
    DEFER,
    ELSE,
    FALLTHROUGH,
    FOR,
    FUNC,
    GO,
    GOTO,
    IF,
    IMPORT,
    INTERFACE,
    MAP,
    PACKAGE,
    RANGE,
    RETURN,
    SELECT,
    STRUCT,
    SWITCH,
    TYPE,
    VAR,
    keyword_end = VAR
};

class Token
{
public:
    Token();
    void reset();

    struct KindAndPosition {
        unsigned kind       : 8;
        unsigned length     : 16;
        unsigned offset     : 32;
        unsigned line       : 24;
        unsigned column     : 16;

        KindAndPosition(unsigned kind_ = ILLEGAL, unsigned length_ = 0, unsigned offset_ = 0,
                        unsigned line_ = 0, unsigned column_ = 0)
            : kind(kind_)
            , length(length_)
            , offset(offset_)
            , line(line_)
            , column(column_)
        { }

        void reset() {
            kind = ILLEGAL;
            length = 0;
            offset = 0;
            line = 0;
            column = 0;
        }
    };

    KindAndPosition kindAndPos;

    inline const KindAndPosition *position() const { return &kindAndPos; }

    union {
        void *ptr;
        const HashedLiteral     *literal;
        const StringLiteral     *string;
        const Identifier        *identifier;
        const Comment           *comment;
    };

    inline unsigned length() const { return kindAndPos.length; }
    inline unsigned begin() const { return kindAndPos.offset; }
    inline unsigned end() const { return kindAndPos.offset + kindAndPos.length; }
    inline unsigned line() const { return kindAndPos.line; }
    inline unsigned column() const { return kindAndPos.column; }

    inline bool isValid() const
    { return kindAndPos.kind != ILLEGAL; }

    inline bool isLiteral() const
    { return kindAndPos.kind >= literal_beg && kindAndPos.kind <= literal_end; }

    inline bool isOperator() const
    { return kindAndPos.kind >= operator_beg && kindAndPos.kind <= operator_end; }

    inline bool isKeyword() const
    { return kindAndPos.kind >= keyword_beg && kindAndPos.kind <= keyword_end; }

    static void classify(Token *tok, Control *control, const char *s, int l);
    static const char *spell(TokenKind kind);

    enum Prec {
        LowestPrec  = 0, // non-operators
        UnaryPrec   = 6,
        HighestPrec = 7
    };

    static int precedence(TokenKind kind);
};

}   // namespace GoTools

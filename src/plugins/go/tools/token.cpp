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
#include "token.h"
#include "control.h"

#include <QMap>

namespace GoTools {

static const char *token_kind_name[] = {
    "<<ILLEGAL>>",
    "<<EOF>>",
    "<<COMMENT>>",

    "<<IDENT>>",
    "<<INT>>",
    "<<FLOAT>>",
    "<<IMAG>>",
    "<<CHAR>>",
    "<<STRING>>",

    "\"+\"",
    "\"-\"",
    "\"*\"",
    "\"/\"",
    "\"%\"",
    "\"&\"",
    "\"|\"",
    "\"^\"",
    "\"<<\"",
    "\">>\"",
    "\"&^\"",
    "\"+=\"",
    "\"-=\"",
    "\"*=\"",
    "\"/=\"",
    "\"%=\"",
    "\"&=\"",
    "\"|=\"",
    "\"^=\"",
    "\"<<=\"",
    "\">>=\"",
    "\"&^=\"",
    "\"&&\"",
    "\"||\"",
    "\"<-\"",
    "\"++\"",
    "\"--\"",
    "\"==\"",
    "\"<\"",
    "\">\"",
    "\"=\"",
    "\"!\"",
    "\"!=\"",
    "\"<=\"",
    "\">=\"",
    "\":=\"",
    "\"...\"",
    "\"(\"",
    "\"[\"",
    "\"{\"",
    "\",\"",
    "\".\"",
    "\")\"",
    "\"]\"",
    "\"}\"",
    "\";\"",
    "\":\"",

    "break",
    "case",
    "chan",
    "const",
    "continue",
    "default",
    "defer",
    "else",
    "fallthrough",
    "for",
    "func",
    "go",
    "goto",
    "if",
    "import",
    "interface",
    "map",
    "package",
    "range",
    "return",
    "select",
    "struct",
    "switch",
    "type",
    "var"
};

static void classify2(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'g':
            if (s[1] == 'o') {
                tok->kindAndPos.kind = GO;
                return;
            }
            break;
        case 'i':
            if (s[1] == 'f') {
                tok->kindAndPos.kind = IF;
                return;
            }
            break;
    }

    tok->identifier = control->identifier(s, 2);
    tok->kindAndPos.kind = IDENT;
}

static void classify3(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'f':
            if (s[1] == 'o' && s[2] == 'r') {
                tok->kindAndPos.kind = FOR;
                return;
            }
            break;
        case 'i':
            if (s[1] == 'n' && s[2] == 't') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinIntIdentifier();
                return;
            }
            break;
        case 'm':
            if (s[1] == 'a' && s[2] == 'p') {
                tok->kindAndPos.kind = MAP;
                return;
            }
            break;
        case 'n':
            if (s[1] == 'e' && s[2] == 'w') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::newIdentifier();
                return;
            }
            break;
        case 'v':
            if (s[1] == 'a' && s[2] == 'r') {
                tok->kindAndPos.kind = VAR;
                return;
            }
            break;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 3);
}

static void classify4(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'b':
            if (s[1] == 'y' && s[2] == 't' && s[3] == 'e') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinByteIdentifier();
                return;
            }
            break;
        case 'c':
            if (s[1] == 'a' && s[2] == 's' && s[3] == 'e') {
                tok->kindAndPos.kind = CASE;
                return;
            } else if (s[1] == 'h' && s[2] == 'a' && s[3] == 'n') {
                tok->kindAndPos.kind = CHAN;
                return;
            }
            break;
        case 'e':
            if (s[1] == 'l' && s[2] == 's' && s[3] == 'e') {
                tok->kindAndPos.kind = ELSE;
                return;
            }
            break;
        case 'f':
            if (s[1] == 'u' && s[2] == 'n' && s[3] == 'c') {
                tok->kindAndPos.kind = FUNC;
                return;
            }
            break;
        case 'g':
            if (s[1] == 'o' && s[2] == 't' && s[3] == 'o') {
                tok->kindAndPos.kind = GOTO;
                return;
            }
            break;
        case 'i':
            if (s[1] == 'n' && s[2] == 't' && s[3] == '8') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinInt8Identifier();
                return;
            }
            break;
        case 'm':
            if (s[1] == 'a' && s[2] == 'k' && s[3] == 'e') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::makeIdentifier();
                return;
            }
            break;
        case 'r':
            if (s[1] == 'u' && s[2] == 'n' && s[3] == 'e') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinRuneIdentifier();
                return;
            }
            break;
        case 't':
            if (s[1] == 'y' && s[2] == 'p' && s[3] == 'e') {
                tok->kindAndPos.kind = TYPE;
                return;
            }
            break;
        case 'u':
            if (s[1] == 'i' && s[2] == 'n' && s[3] == 't') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinUintIdentifier();
                return;
            }
            break;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 4);
}

static void classify5(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'b':
            if (s[1] == 'r' && s[2] == 'e' && s[3] == 'a' && s[4] == 'k') {
                tok->kindAndPos.kind = BREAK;
                return;
            }
            break;
        case 'c':
            if (s[1] == 'o' && s[2] == 'n' && s[3] == 's' && s[4] == 't') {
                tok->kindAndPos.kind = CONST;
                return;
            }
            break;
        case 'd':
            if (s[1] == 'e' && s[2] == 'f' && s[3] == 'e' && s[4] == 'r') {
                tok->kindAndPos.kind = DEFER;
                return;
            }
            break;
        case 'e':
            if (s[1] == 'r' && s[2] == 'r' && s[3] == 'o' && s[4] == 'r') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinErrorIdentifier();
                return;
            }
            break;
        case 'i':
            if (s[1] == 'n' && s[2] == 't') {
                if (s[3] == '1' && s[4] == '6') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinInt16Identifier();
                    return;
                } else if (s[3] == '3' && s[4] == '2') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinInt32Identifier();
                } else if (s[3] == '6' && s[4] == '4') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinInt64Identifier();
                    return;
                }
            }
            break;
        case 'r':
            if (s[1] == 'a' && s[2] == 'n' && s[3] == 'g' && s[4] == 'e') {
                tok->kindAndPos.kind = RANGE;
                return;
            }
            break;
        case 'u':
            if (s[1] == 'i' && s[2] == 'n' && s[3] == 't' && s[4] == '8') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinUint8Identifier();
                return;
            }
            break;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 5);
}

static void classify6(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'i':
            if (s[1] == 'm' && s[2] == 'p' && s[3] == 'o' && s[4] == 'r' && s[5] == 't') {
                tok->kindAndPos.kind = IMPORT;
                return;
            }
            break;
        case 'r':
            if (s[1] == 'e' && s[2] == 't' && s[3] == 'u' && s[4] == 'r' && s[5] == 'n') {
                tok->kindAndPos.kind = RETURN;
                return;
            }
            break;

        case 's':
            switch (s[1]) {
                case 'e':
                    if (s[2] == 'l' && s[3] == 'e' && s[4] == 'c' && s[5] == 't') {
                        tok->kindAndPos.kind = SELECT;
                        return;
                    }
                    break;
                case 't':
                    if (s[2] == 'r') {
                        if (s[3] == 'i' && s[4] == 'n' && s[5] == 'g') {
                            tok->kindAndPos.kind = IDENT;
                            tok->identifier = Control::builtinStringIdentifier();
                            return;
                        } else if (s[3] == 'u' && s[4] == 'c' && s[5] == 't') {
                            tok->kindAndPos.kind = STRUCT;
                            return;
                        }
                    }
                    break;
                case 'w':
                    if (s[2] == 'i' && s[3] == 't' && s[4] == 'c' && s[5] == 'h') {
                        tok->kindAndPos.kind = SWITCH;
                        return;
                    }
                    break;
            }
            break;
        case 'u':
            if (s[1] == 'i' && s[2] == 'n' && s[3] == 't') {
                if (s[4] == '1' && s[5] == '6') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinUint16Identifier();
                    return;
                } else if (s[4] == '3' && s[5] == '2') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinUint32Identifier();
                    return;
                } else if (s[4] == '6' && s[5] == '4') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinUint64Identifier();
                    return;
                }
            }
            break;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 6);
}

static void classify7(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'd':
            if (s[1] == 'e' && s[2] == 'f' && s[3] == 'a' &&
                s[4] == 'u' && s[5] == 'l' && s[6] == 't') {
                tok->kindAndPos.kind = DEFAULT;
                return;
            }
            break;
        case 'f':
            if (s[1] == 'l' && s[2] == 'o' && s[3] == 'a' && s[4] == 't') {
                if (s[5] == '3' && s[6] == '2') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinFloat32Identifier();
                    return;
                } else if (s[5] == '6' && s[6] == '4') {
                    tok->kindAndPos.kind = IDENT;
                    tok->identifier = Control::builtinFloat64Identifier();
                    return;
                }
            }
            break;
        case 'p':
            if (s[1] == 'a' && s[2] == 'c' && s[3] == 'k' &&
                s[4] == 'a' && s[5] == 'g' && s[6] == 'e') {
                tok->kindAndPos.kind = PACKAGE;
                return;
            }
            break;
        case 'u':
            if (s[1] == 'i' && s[2] == 'n' && s[3] == 't' &&
                s[4] == 'p' && s[5] == 't' && s[6] == 'r') {
                tok->kindAndPos.kind = IDENT;
                tok->identifier = Control::builtinUintptrIdentifier();
                return;
            }
            break;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 7);
}

static void classify8(Token *tok, Control *control, const char *s) {
    if (s[0] == 'c' && s[1] == 'o' && s[2] == 'n' && s[3] == 't' &&
        s[4] == 'i' && s[5] == 'n' && s[6] == 'u' && s[7] == 'e') {
        tok->kindAndPos.kind = CONTINUE;
        return;
    }

    tok->kindAndPos.kind =IDENT;
    tok->identifier = control->identifier(s, 8);
}

static void classify9(Token *tok, Control *control, const char *s) {
    switch (s[0]) {
        case 'c':
            if (s[1] == 'o' && s[2] == 'm' && s[3] == 'p' && s[4] == 'l' &&
                s[5] == 'e' && s[6] == 'x' && s[7] == '6' && s[8] == '4') {
                tok->kindAndPos.kind =IDENT;
                tok->identifier = Control::builtinComplex64Identifier();
                return;
            }
            break;
        case 'i':
            if (s[1] == 'n' && s[2] == 't' && s[3] == 'e' && s[4] == 'r' &&
                    s[5] == 'f' && s[6] == 'a' && s[7] == 'c' && s[8] == 'e') {
                tok->kindAndPos.kind =INTERFACE;
                return;
            }
            break;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 9);
}

static void classify10(Token *tok, Control *control, const char *s) {
    if (s[0] == 'c' && s[1] == 'o' && s[2] == 'm' && s[3] == 'p' && s[4] == 'l' &&
        s[5] == 'e' && s[6] == 'x' && s[7] == '1' && s[8] == '2' && s[10] == '8') {
        tok->kindAndPos.kind =IDENT;
        tok->identifier = Control::builtinComplex128Identifier();
        return;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 10);
}

static void classify11(Token *tok, Control *control, const char *s) {
    if (s[0] == 'f' && s[1] == 'a' && s[2] == 'l' && s[3] == 'l' && s[4] == 't' && s[5] == 'h' &&
        s[6] == 'r' && s[7] == 'o' && s[8] == 'u' && s[9] == 'g' && s[10] == 'h') {
        tok->kindAndPos.kind = FALLTHROUGH;
        return;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, 11);
}

Token::Token()
{ }

void Token::reset()
{
    kindAndPos.reset();
    ptr = 0;
}

void Token::classify(Token *tok, Control *control, const char *s, int l)
{
    switch (l) {
        case  2: classify2(tok, control, s);  return;
        case  3: classify3(tok, control, s);  return;
        case  4: classify4(tok, control, s);  return;
        case  5: classify5(tok, control, s);  return;
        case  6: classify6(tok, control, s);  return;
        case  7: classify7(tok, control, s);  return;
        case  8: classify8(tok, control, s);  return;
        case  9: classify9(tok, control, s);  return;
        case 10: classify10(tok, control, s); return;
        case 11: classify11(tok, control, s); return;
    }

    tok->kindAndPos.kind = IDENT;
    tok->identifier = control->identifier(s, l);
}

const char *Token::spell(TokenKind kind)
{ return token_kind_name[kind]; }

int Token::precedence(TokenKind kind)
{
    switch(kind) {
        case LOR:
            return 1;
        case LAND:
            return 2;
        case EQL: case NEQ: case LSS: case LEQ: case GTR: case GEQ:
            return 3;
        case ADD: case SUB: case OR: case XOR:
            return 4;
        case MUL: case QUO: case REM: case SHL: case SHR: case AND: case AND_NOT:
            return 5;
        default: break; // prevent -Wswitch warning
    }
    return LowestPrec;
}

}   // namespace GoTools

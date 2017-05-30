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
#include "scanner.h"
#include "control.h"

#include <cctype>

namespace GoTools {

const char *CTE_NO_ERROR                = "";
const char *CTE_ILLEGAL_HEX_NUMBER      = "illegal hexadecimal number";
const char *CTE_ILLEGAL_OCT_NUMBER      = "illegal octal number";
const char *CTE_STR_NOT_TERMINATED      = "string literal not terminated";
const char *CTE_ESC_SEQ_NOT_TERMINATED  = "escape sequence not terminated";
const char *CTE_ESC_SEQ_UNKNOWN         = "unknown escape sequence";
const char *CTE_ESC_SEQ_ILLEGAL_CHAR    = "illegal character in escape sequence";
const char *CTE_ESC_SEQ_INVALID_UCODE   = "escape sequence is invalid Unicode code point";
const char *CTE_RUNE_NOT_TERMINATED     = "rune literal not terminated";
const char *CTE_RUNE_ILLEGAL            = "illegal rune literal";
const char *CTE_RAWSTR_NOT_TERMINATED   = "raw string literal not terminated";
const char *CTE_COMMENT_NOT_TERMINATED  = "comment not terminated";

Scanner::Scanner(TranslationUnit *translationUnit, const char *firstSourceChar, const char *lastSourceChar)
    : _translationUnit(translationUnit)
    , _control(translationUnit->control())
    , _firstChar(firstSourceChar)
    , _currentChar(_firstChar - 1)
    , _lastChar(lastSourceChar)
    , _yychar(0)
    , _insertSemi(false)
    , _currentLine(1)
    , _tokenError(CTE_NO_ERROR)
{
    _currentCharUtf16 = ~0;
    _currentColCharUtf16 = ~0;
    _tokenStart = _currentChar;
    _yychar = '\n';
}

const char *Scanner::scan(Token *tok)
{
    tok->reset();
    _tokenError = CTE_NO_ERROR;
    scan_helper(tok);
    tok->kindAndPos.length = _currentCharUtf16 - _tokenStartUtf16;
    return _tokenError;
}

void Scanner::pushLineStartOffset()
{
    ++_currentLine;
    _currentColCharUtf16 = 0;
}

void Scanner::scan_helper(Token *tok)
{
    skipWhitespace();

    bool insertSemi = false;

    tok->kindAndPos.line = _currentLine;
    tok->kindAndPos.column = _currentColCharUtf16;

    _tokenStart = _currentChar;

    _tokenStartUtf16 = _currentCharUtf16;
    tok->kindAndPos.offset = _currentCharUtf16;

    if (!_yychar) {
        if (_insertSemi) {
            _insertSemi = false;
            tok->kindAndPos.kind = SEMICOLON;
            return;
        }
        tok->kindAndPos.kind = T_EOF;
        return;
    }

    if (isByteOfMultiByteCodePoint(_yychar)) {
        unsigned utf8bytesCount = 2;
        for (unsigned char c = _yychar << 2; isByteOfMultiByteCodePoint(c); c <<= 1)
            ++utf8bytesCount;

        if (QString::fromUtf8(_currentChar, utf8bytesCount)[0].isLetter()) {
            scanIdentifier(tok, utf8bytesCount);
            switch (tok->kindAndPos.kind) {
                case IDENT:
                case BREAK:
                case CONTINUE:
                case FALLTHROUGH:
                case RETURN:
                    insertSemi = true;
                    break;
            }
        } else {
            yyinp();
            tok->kindAndPos.kind = ILLEGAL;
        }

        _insertSemi = insertSemi;;
        return;
    }


    char ch = _yychar;
    switch (ch) {
        case '_': case '$':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
        case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
        case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            scanIdentifier(tok, 1);
            switch (tok->kindAndPos.kind) {
                case IDENT:
                case BREAK:
                case CONTINUE:
                case FALLTHROUGH:
                case RETURN:
                    insertSemi = true;
                    break;
            }
            break;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            insertSemi = true;
            tok->kindAndPos.kind = scanNumber(false);
            break;
        default:
            yyinp();     // always make progress
            switch (ch) {
                case '\n':
                    // we only reach here if s.insertSemi was
                    // set in the first place and exited early
                    // from s.skipWhitespace()
                    _insertSemi = false; // newline consumed
                    tok->kindAndPos.kind = SEMICOLON;
                    return;
                case '"':
                    insertSemi = true;
                    tok->kindAndPos.kind = STRING;
                    scanString();
                    tok->string = _control->string(_tokenStart, _currentChar - _tokenStart);
                    break;
                case '\'':
                    insertSemi = true;
                    tok->kindAndPos.kind = CHAR;
                    scanRune();
                    break;
                case '`':
                    insertSemi = true;
                    tok->kindAndPos.kind = STRING;
                    scanRawString();
                    break;
                case ':':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = DEFINE;
                    } else {
                        tok->kindAndPos.kind = COLON;
                    }
                    break;
                case '.':
                    if (_yychar >= '0' && _yychar <= '9') {
                        insertSemi = true;
                        tok->kindAndPos.kind = scanNumber(true);
                    } else if (_yychar == '.') {
                        yyinp();
                        if (_yychar == '.') {
                            yyinp();
                            tok->kindAndPos.kind = ELLIPSIS;
                        }
                    } else {
                        tok->kindAndPos.kind = PERIOD;
                    }
                    break;
                case ',':
                    tok->kindAndPos.kind = COMMA;
                    break;
                case ';':
                    tok->kindAndPos.kind = SEMICOLON;
                    break;
                case '(':
                    tok->kindAndPos.kind = LPAREN;
                    break;
                case ')':
                    insertSemi = true;
                    tok->kindAndPos.kind = RPAREN;
                    break;
                case '[':
                    tok->kindAndPos.kind = LBRACK;
                    break;
                case ']':
                    insertSemi = true;
                    tok->kindAndPos.kind = RBRACK;
                    break;
                case '{':
                    tok->kindAndPos.kind = LBRACE;
                    break;
                case '}':
                    insertSemi = true;
                    tok->kindAndPos.kind = RBRACE;
                    break;
                case '+':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = ADD_ASSIGN;
                    } else if (_yychar == '+') {
                        yyinp();
                        tok->kindAndPos.kind = INC;
                        insertSemi = true;
                    } else {
                        tok->kindAndPos.kind = ADD;
                    }
                    break;
                case '-':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = SUB_ASSIGN;
                    } else if (_yychar == '-') {
                        yyinp();
                        tok->kindAndPos.kind = DEC;
                        insertSemi = true;
                    } else {
                        tok->kindAndPos.kind = SUB;
                    }
                    break;
                case '*':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = MUL_ASSIGN;
                    } else {
                        tok->kindAndPos.kind = MUL;
                    }
                    break;
                case '/':
                    if (_yychar == '/' || _yychar == '*') {
                        // comment
                        if (_insertSemi && findLineEnd()) {
                            yyuninp();
                            _insertSemi = false;
                            tok->kindAndPos.kind = SEMICOLON;
                            return;
                        }
                        scanComment();
                        tok->kindAndPos.kind = COMMENT;
                        tok->comment = _control->comment(_tokenStart, _currentChar - _tokenStart);
                    } else {
                        if (_yychar == '=') {
                            yyinp();
                            tok->kindAndPos.kind = QUO_ASSIGN;
                        } else {
                            tok->kindAndPos.kind = QUO;
                        }
                    }
                    break;
                case '%':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = REM_ASSIGN;
                    } else {
                        tok->kindAndPos.kind = REM;
                    }
                    break;
                case '^':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = XOR_ASSIGN;
                    } else {
                        tok->kindAndPos.kind = XOR;
                    }
                    break;
                case '<':
                    if (_yychar == '-') {
                        yyinp();
                        tok->kindAndPos.kind = ARROW;
                    } else if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = LEQ;
                    } else if (_yychar == '<') {
                        yyinp();
                        if (_yychar == '=') {
                            yyinp();
                            tok->kindAndPos.kind = SHL_ASSIGN;
                        } else {
                            tok->kindAndPos.kind = SHL;
                        }
                    } else {
                        tok->kindAndPos.kind = LSS;
                    }
                    break;
                case '>':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = GEQ;
                    } else if (_yychar == '>') {
                        yyinp();
                        if (_yychar == '=') {
                            yyinp();
                            tok->kindAndPos.kind = SHR_ASSIGN;
                        } else {
                            tok->kindAndPos.kind = SHR;
                        }
                    } else {
                        tok->kindAndPos.kind = GTR;
                    }
                    break;
                case '=':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = EQL;
                    } else {
                        tok->kindAndPos.kind = ASSIGN;
                    }
                    break;
                case '!':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = NEQ;
                    } else {
                        tok->kindAndPos.kind = NOT;
                    }
                    break;
                case '&':
                    if (_yychar == '^') {
                        yyinp();
                        if (_yychar == '=') {
                            yyinp();
                            tok->kindAndPos.kind = AND_NOT_ASSIGN;
                        } else {
                            tok->kindAndPos.kind = AND_NOT;
                        }
                    } else {
                        if (_yychar == '=') {
                            yyinp();
                            tok->kindAndPos.kind = AND_ASSIGN;
                        } else if (_yychar == '&') {
                            yyinp();
                            tok->kindAndPos.kind = LAND;
                        } else {
                            tok->kindAndPos.kind = AND;
                        }
                    }
                    break;
                case '|':
                    if (_yychar == '=') {
                        yyinp();
                        tok->kindAndPos.kind = OR_ASSIGN;
                    } else if (_yychar == '|') {
                        yyinp();
                        tok->kindAndPos.kind = LOR;
                    } else {
                        tok->kindAndPos.kind = OR;
                    }
                    break;
                default:
                    insertSemi = _insertSemi;
                    tok->kindAndPos.kind = ILLEGAL;
                    break;
            }
    }

    _insertSemi = insertSemi;
}

void Scanner::skipWhitespace()
{
    while (true) {
        switch (_yychar) {
            case ' ':
            case '\t':
            case '\r':
                yyinp();
                break;
            case '\n':
                if (_insertSemi)
                    return;
                yyinp();
                break;
            default:
                return;
        }
    }
}

bool Scanner::isLetterOrDigit(unsigned &utf8bytesCount)
{
    utf8bytesCount = 1;

    if (isByteOfMultiByteCodePoint(_yychar)) {
        ++utf8bytesCount;
        for (unsigned char c = _yychar << 2; isByteOfMultiByteCodePoint(c); c <<= 1)
            ++utf8bytesCount;
        return QString::fromUtf8(_currentChar, utf8bytesCount)[0].isLetter();
    }

    switch (_yychar) {
        case '_': case '$':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
        case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
        case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return true;
        default:
            return false;
    }

    return false;
}

void Scanner::scanIdentifier(Token *tok, unsigned utf8bytesCount)
{
    // skip first letter
    unsigned tokenLength = utf8bytesCount;
    bool isLatin1 = utf8bytesCount == 1;
    yyinp();

    while (isLetterOrDigit(utf8bytesCount)) {
        tokenLength += utf8bytesCount;
        if (utf8bytesCount != 1)
            isLatin1 = false;
        yyinp();
    }


    if (isLatin1) {
        Token::classify(tok, _control, _tokenStart, tokenLength);
    } else {
        tok->kindAndPos.kind = IDENT;
        tok->identifier = _control->identifier(_tokenStart, tokenLength);
    }
}

TokenKind Scanner::scanNumber(bool seenDecimalPoint)
{
    TokenKind kind;
    kind = INT;

    if (seenDecimalPoint) {
        kind = FLOAT;
        scanMantissa(10);
        goto exponent;
    }

    if (_yychar == '0') {
        // int or float
        yyinp();
        if (_yychar == 'x' || _yychar == 'X') {
            // hexadecimal int
            yyinp();
            if (!scanMantissa(16)) {
                // only scanned "0x" or "0X"
                _tokenError = CTE_ILLEGAL_HEX_NUMBER;
            }
        } else {
            // octal int or float
            bool seenDecimalDigit = false;
            scanMantissa(8);
            if (_yychar == '8' || _yychar == '9') {
                // illegal octal int or float
                seenDecimalDigit = true;
                scanMantissa(10);
            }
            if (_yychar == '.' || _yychar == 'e' || _yychar == 'E' || _yychar == 'i') {
                goto fraction;
            }
            // octal int
            if (seenDecimalDigit) {
                _tokenError = CTE_ILLEGAL_OCT_NUMBER;
            }
        }
        goto exit;
    }

    // decimal int or float
    scanMantissa(10);

fraction:
    if (_yychar == '.') {
        kind = FLOAT;
        yyinp();
        scanMantissa(10);
    }

exponent:
    if (_yychar == 'e' || _yychar == 'E') {
        kind = FLOAT;
        yyinp();
        if (_yychar == '-' || _yychar == '+')
            yyinp();
        scanMantissa(10);
    }

    if (_yychar == 'i') {
        kind = IMAG;
        yyinp();
    }

exit:
    return kind;
}

unsigned Scanner::scanMantissa(unsigned base)
{
    unsigned result = 0;
    while (digitVal(_yychar) < base) {
        yyinp();
        result++;
    }
    return result;
}

unsigned Scanner::digitVal(char ch)
{
    switch (ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return ch - '0';
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            return ch - 'a' + 10;
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            return ch - 'A' + 10;
    }

    return 16;
}

void Scanner::scanString()
{
    while (true) {
        unsigned char ch = _yychar;
        if (ch == '\n') {
            _tokenError = CTE_STR_NOT_TERMINATED;
            break;
        }
        yyinp();
        if (ch == '"')
            break;
        if (ch == '\\')
            scanEscape('"');
    }
}

bool Scanner::scanEscape(char quote)
{
    if (_yychar == quote) {
        yyinp();
        return true;
    }

    int n;
    unsigned base, max;
    switch (_yychar) {
        case 'a': case 'b': case 'f': case 'n': case 'r': case 't': case 'v': case '\\':
            yyinp();
            return true;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            n = 3;
            base = 8;
            max = 255;
            break;
        case 'x':
            yyinp();
            n = 2;
            base = 16;
            max = 255;
            break;
        case 'u':
            yyinp();
            n = 4;
            base = 16;
            max = 0x10FFFF;
            break;
        case 'U':
            yyinp();
            n = 8;
            base = 16;
            max = 0x10FFFF;
            break;
        default:
            if (!_yychar) {
                _tokenError = CTE_ESC_SEQ_NOT_TERMINATED;
            } else {
                _tokenError = CTE_ESC_SEQ_UNKNOWN;
            }
            return false;
    }

    unsigned x = 0;
    while (n > 0) {
        unsigned d = digitVal(_yychar);
        if (d >= base) {
            if (!_yychar) {
                _tokenError = CTE_ESC_SEQ_NOT_TERMINATED;
            } else {
                _tokenError = CTE_ESC_SEQ_ILLEGAL_CHAR;
            }
            return false;
        }
        x = x*base + d;
        yyinp();
        n--;
    }

    if (x > max || (0xD800 <= x && x < 0xE000)) {
        _tokenError = CTE_ESC_SEQ_INVALID_UCODE;
        return false;
    }

    return true;
}

void Scanner::scanRune()
{
    bool valid = true;
    unsigned n = 0;
    while (true) {
        char ch = _yychar;
        if (ch == '\n' || !ch) {
            // only report error if we don't have one already
            if (valid) {
                _tokenError = CTE_RUNE_NOT_TERMINATED;
                valid = false;
            }
            break;
        }
        yyinp();
        if (ch == '\'')
            break;
        n++;
        if (ch == '\\') {
            if (!scanEscape('\''))
                valid = false;
            // continue to read to closing quote
        }
    }

    if (valid && n != 1)
        _tokenError = CTE_RUNE_ILLEGAL;
}

void Scanner::scanRawString()
{
    while (true) {
        char ch = _yychar;
        if (!ch) {
            _tokenError = CTE_RAWSTR_NOT_TERMINATED;
            break;
        }
        yyinp();
        if (ch == '`')
            break;
    }
}

bool Scanner::findLineEnd()
{
    // initial '/' already consumed
    // read ahead until a newline, EOF, or non-comment token is found

    const char *currentChar= _currentChar;
    unsigned currentCharUtf16= _currentCharUtf16;
    unsigned currentColCharUtf16= _currentColCharUtf16;
    unsigned char yychar = _yychar;

    while (yychar == '/' || yychar == '*') {
        if (yychar == '/') {
            //-style comment always contains a newline
            return true;
        }
        /*-style comment: look for newline */
        yyinp_utf8(currentChar, yychar, currentCharUtf16, currentColCharUtf16);
        while (yychar) {
            char ch = yychar;
            if (ch == '\n') {
                return true;
            }
            yyinp_utf8(currentChar, yychar, currentCharUtf16, currentColCharUtf16);
            if (ch == '*' && yychar == '/') {
                yyinp_utf8(currentChar, yychar, currentCharUtf16, currentColCharUtf16);
                break;
            }
        }

        // skip whitespace
        bool notWhitespace = true;
        while (notWhitespace) {
            switch (yychar) {
                case ' ':
                case '\t':
                case '\r':
                    yyinp_utf8(currentChar, yychar, currentCharUtf16, currentColCharUtf16);
                    break;
                case '\n':
                    if (_insertSemi) {
                        notWhitespace = false;
                        break;
                    }
                    yyinp_utf8(currentChar, yychar, currentCharUtf16, currentColCharUtf16);
                    break;
                default:
                    notWhitespace = false;
                    break;
            }
        }

        if (!yychar || yychar == '\n') {
            return true;
        }
        if (yychar != '/') {
            // non-comment token
            return false;
        }
        yyinp_utf8(currentChar, yychar, currentCharUtf16, currentColCharUtf16); // consume '/'
    }

    return false;
}

void Scanner::scanComment()
{
    if (_yychar == '/') {
        //-style comment
        yyinp();
        while (_yychar != '\n' && _yychar)
            yyinp();
        return;
    }

    /*-style comment */
    yyinp();
    while (_yychar) {
        char ch = _yychar;
        yyinp();
        if (ch == '*' && _yychar == '/') {
            yyinp();
            return;
        }
    }

    _tokenError = CTE_COMMENT_NOT_TERMINATED;
}

unsigned Scanner::currentLine() const
{ return _currentLine; }

}   // namespace GoTools

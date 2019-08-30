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

#include "token.h"
#include "translationunit.h"

#include <QString>

namespace GoTools {

class Scanner
{
public:
    Scanner(TranslationUnit *translationUnit, const char *firstSourceChar, const char *lastSourceChar);

    inline const char * operator()(Token *tok)
    { return scan(tok); }

    const char *scan(Token *tok);

    static void yyinp_utf8(const char *&currentSourceChar, unsigned char &yychar,
                           unsigned &utf16charCounter, unsigned &utf16charColCounter)
    {
        ++utf16charCounter;
        ++utf16charColCounter;

        // Process multi-byte UTF-8 code point (non-latin1)
        if (isByteOfMultiByteCodePoint(yychar)) {
            unsigned trailingBytesCurrentCodePoint = 1;
            for (unsigned char c = yychar << 2; isByteOfMultiByteCodePoint(c); c <<= 1)
                ++trailingBytesCurrentCodePoint;
            // Code points >= 0x00010000 are represented by two UTF-16 code units
            if (trailingBytesCurrentCodePoint >= 3) {
                ++utf16charCounter;
                ++utf16charColCounter;
            }
            yychar = *(currentSourceChar += trailingBytesCurrentCodePoint + 1);

            // Process single-byte UTF-8 code point (latin1)
        } else {
            yychar = *++currentSourceChar;
        }
    }

    unsigned currentLine() const;

private:
    void pushLineStartOffset();
    void scan_helper(Token *tok);
    void skipWhitespace();

    static bool isByteOfMultiByteCodePoint(unsigned char byte)
    { return byte & 0x80; } // Check if most significant bit is set

    void yyinp()
    {
        yyinp_utf8(_currentChar, _yychar, _currentCharUtf16, _currentColCharUtf16);
        if (_yychar == '\n')
            pushLineStartOffset();
    }

    void yyuninp() {
        --_currentCharUtf16;
        --_currentColCharUtf16;
        _yychar = *--_currentChar;
    }

private:
    bool isLetterOrDigit(unsigned &utf8bytesCount);
    void scanIdentifier(Token *tok, unsigned utf8bytesCount);
    TokenKind scanNumber(bool seenDecimalPoint);
    unsigned scanMantissa(unsigned base);
    unsigned digitVal(char ch);
    void scanString();
    bool scanEscape(char quote);
    void scanRune();
    void scanRawString();
    bool findLineEnd();
    void scanComment();

private:
    TranslationUnit *_translationUnit;
    Control *_control;

    const char *_firstChar;
    const char *_currentChar;
    const char *_lastChar;
    const char *_tokenStart;

    unsigned _currentCharUtf16;
    unsigned _currentColCharUtf16;
    unsigned _tokenStartUtf16;

    unsigned char _yychar;
    bool _insertSemi;

    unsigned _currentLine;
    const char *_tokenError;
};

}   // namespace GoTools

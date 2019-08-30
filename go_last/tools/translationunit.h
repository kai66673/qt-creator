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
#include "memorypool.h"
#include "diagnosticclient.h"
#include "token.h"

#include <vector>

namespace GoTools {

class Control;
class AST;
class FileAST;
class GoSource;

class TranslationUnit
{
    TranslationUnit(const TranslationUnit &other);
    void operator =(const TranslationUnit &other);

public:
    TranslationUnit(Control *control, const QString &fileName);
    ~TranslationUnit();

    Control *control() const;
    MemoryPool *memoryPool() const;

    QString fileName() const;

    void setSource(const char *source, unsigned size);

    unsigned tokenCount() const { return _tokens ? unsigned(_tokens->size()) : unsigned(0); }
    const Token &tokenAt(unsigned index) const
    { return _tokens && index < tokenCount() ? (*_tokens)[index] : nullToken; }

    enum ParseMode {
        Fast,
        Full
    };

    unsigned skipFunctionBody() const;
    void setSkipFunctionBody(bool onoff);

    bool isParsed() const;
    bool parse(GoSource *source, ParseMode mode);
    bool parsePackageFile(GoSource *source, const QString &packageName);

    void resetAST();
    void release();

    const char *firstSourceChar() const;
    const char *lastSourceChar() const;

    void getTokenPosition(unsigned index,
                          unsigned *line,
                          unsigned *column = 0) const;

    void warning(unsigned index, const char *fmt, ...);
    void error(unsigned index, const char *fmt, ...);
    void fatal(unsigned index, const char *fmt, ...);

    void message(DiagnosticClient::Level level, unsigned index,
                 const char *format, va_list args);

    std::vector<Token> *tokens() const;

    FileAST *fileAst() const;

private:
    void releaseTokensAndComments();

    Control *_control;
    QString _fileName;
    const char *_firstSourceChar;
    const char *_lastSourceChar;
    std::vector<Token> *_tokens;
    MemoryPool *_pool;
    FileAST *_ast;

    TranslationUnit *_previousTranslationUnit;
    struct Flags {
        unsigned _parsed: 1;
        unsigned _blockErrors: 1;
        unsigned _skipFunctionBody: 1;
    };
    union {
        unsigned _flags;
        Flags f;
    };

    static const Token nullToken;
};

}   // namespace GoTools

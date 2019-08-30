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
#include "translationunit.h"
#include "control.h"
#include "parser.h"
#include "scanner.h"
#include "ast.h"
#include "token.h"

#include <cstdarg>

#include <QDebug>

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#    define va_copy(dst, src) ((dst) = (src))
#elif defined(__INTEL_COMPILER) && !defined(va_copy)
#    define va_copy __va_copy
#endif

namespace GoTools {

const Token TranslationUnit::nullToken;

TranslationUnit::TranslationUnit(Control *control, const QString &fileName)
    : _control(control)
    , _fileName(fileName)
    , _firstSourceChar(0)
    , _lastSourceChar(0)
    , _pool(new MemoryPool())
    , _ast(0)
    , _flags(0)
{
    _tokens = new std::vector<Token>();
    _previousTranslationUnit = control->switchTranslationUnit(this);
}

TranslationUnit::~TranslationUnit()
{
    (void) _control->switchTranslationUnit(_previousTranslationUnit);
    release();
}

Control *TranslationUnit::control() const
{ return _control; }

MemoryPool *TranslationUnit::memoryPool() const
{ return _pool; }

QString TranslationUnit::fileName() const
{ return _fileName; }

void TranslationUnit::setSource(const char *source, unsigned size)
{
    _firstSourceChar = source;
    _lastSourceChar = source + size;
    f._parsed = false;
}

unsigned TranslationUnit::skipFunctionBody() const
{ return f._skipFunctionBody; }

void TranslationUnit::setSkipFunctionBody(bool onoff)
{ f._skipFunctionBody = onoff; }

bool TranslationUnit::isParsed() const
{ return f._parsed; }

bool TranslationUnit::parse(GoSource *source, TranslationUnit::ParseMode mode)
{
    if (isParsed())
        return false;

    f._parsed = true;
    f._skipFunctionBody = mode == Fast ? true : false;

    Parser parser(source);
    _ast = parser.parseFile();

    return true;
}

bool TranslationUnit::parsePackageFile(GoSource *source, const QString &packageName)
{
    if (isParsed())
        return _ast ? packageName == _ast->packageName->ident->toString() : false;

    f._parsed = true;
    f._skipFunctionBody = true;

    Parser parser(source);
    _ast = parser.parsePackageFile(packageName);
    f._parsed = _ast != 0;

    return f._parsed;
}

void TranslationUnit::resetAST()
{
    delete _pool;
    _pool = 0;
    _ast = 0;
}

void TranslationUnit::release()
{
    resetAST();
    releaseTokensAndComments();
}

const char *TranslationUnit::firstSourceChar() const
{ return _firstSourceChar; }

const char *TranslationUnit::lastSourceChar() const
{ return _lastSourceChar; }

void TranslationUnit::getTokenPosition(unsigned index,
                                       unsigned *line,
                                       unsigned *column) const
{
    const Token &tk = tokenAt(index);
    if (line)
        *line = tk.line();
    if (column)
        *column = tk.column();
}

void TranslationUnit::warning(unsigned index, const char *format, ...)
{
    if (f._blockErrors)
        return;

    va_list args, ap;
    va_start(args, format);
    va_copy(ap, args);
    message(DiagnosticClient::Warning, index, format, args);
    va_end(ap);
    va_end(args);
}

void TranslationUnit::error(unsigned index, const char *format, ...)
{
    if (f._blockErrors)
        return;

    va_list args, ap;
    va_start(args, format);
    va_copy(ap, args);
    message(DiagnosticClient::Error, index, format, args);
    va_end(ap);
    va_end(args);
}

void TranslationUnit::fatal(unsigned index, const char *format, ...)
{
    if (f._blockErrors)
        return;

    va_list args, ap;
    va_start(args, format);
    va_copy(ap, args);
    message(DiagnosticClient::Fatal, index, format, args);
    va_end(ap);
    va_end(args);
}

void TranslationUnit::message(DiagnosticClient::Level level, unsigned index, const char *format, va_list args)
{
    if (f._blockErrors)
        return;

    index = std::min(index, tokenCount() - 1);

    unsigned line = 0, column = 0;
    getTokenPosition(index, &line, &column);

    qDebug() << "LOG::MSG" << level << QString::fromUtf8(format) << "..."
             << _tokens->at(index).kindAndPos.line << _tokens->at(index).kindAndPos.column
             << fileName();

    if (DiagnosticClient *client = control()->diagnosticClient()) {
        client->report(level, 0, line, column, tokenAt(index).length(), format, args);
    }

    if (level == DiagnosticClient::Fatal)
        exit(EXIT_FAILURE);
}

void TranslationUnit::releaseTokensAndComments()
{
    delete _tokens;
    _tokens = 0;
}

FileAST *TranslationUnit::fileAst() const
{ return _ast; }

std::vector<Token> *TranslationUnit::tokens() const
{ return _tokens; }

}   // namespace GoTools

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

#include <QString>

namespace GoTools {

class Type;

class Literal
{
    Literal(const Literal &other);
    void operator =(const Literal &other);

public:
    Literal(const char *chars, unsigned size);

protected:
    const char *_chars;
    unsigned _size;
};

class StringLiteral: public Literal
{
public:
    StringLiteral(const char *chars, unsigned size);
    QString unquoted() const;
};

class Comment: public Literal
{
public:
    Comment(const char *chars, unsigned size);
    QString body() const;
};

class HashedLiteral
{
    HashedLiteral(const HashedLiteral &other);
    void operator =(const HashedLiteral &other);

public:
    typedef const char *iterator;
    typedef iterator const_iterator;

public:
    HashedLiteral(const char *chars, unsigned size);

    iterator begin() const;
    iterator end() const;

    char at(unsigned index) const;
    const char *chars() const;
    unsigned size() const;

    unsigned hashCode() const;
    static unsigned hashCode(const char *chars, unsigned size);

    bool equalTo(const HashedLiteral *other) const;
    bool equalTo(const char *chars, unsigned size) const;

    HashedLiteral *_next; // ## private

protected:
    const char *_chars;
    unsigned _size;
    unsigned _hashCode;
};

class Identifier: public HashedLiteral
{
public:
    Identifier(const char *chars, unsigned size);

    QString toString() const;

    const Type *asBuiltinType() const;
    bool isBuiltinTypeIdentifier() const;
};

}   // namespace GoTools

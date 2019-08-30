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
#include "literals.h"
#include "types.h"

#include <cstring>

namespace GoTools {

Literal::Literal(const char *chars, unsigned size)
    : _chars(chars)
    , _size(size)
{ }

StringLiteral::StringLiteral(const char *chars, unsigned size)
    : Literal(chars, size)
{ }

QString StringLiteral::unquoted() const
{
    QString result = QString::fromUtf8(_chars + 1, _size - 1);
    if (result.endsWith(QStringLiteral("\"")))
        result.chop(1);
    return result;
}

Comment::Comment(const char *chars, unsigned size)
    : Literal(chars, size)
{ }

QString Comment::body() const
{
    bool chopNeeded = _chars[1] == '*';
    QString result = QString::fromUtf8(_chars + 2, _size - 2);
    if (chopNeeded && result.endsWith(QStringLiteral("*/")))
        result.chop(2);
    return result;
}

HashedLiteral::HashedLiteral(const char *chars, unsigned size)
    : _next(0)
    , _chars(chars)
    , _size(size)
{
    _hashCode = hashCode(_chars, _size);
}

bool HashedLiteral::equalTo(const HashedLiteral *other) const
{
    if (! other)
        return false;
    else if (this == other)
        return true;
    else if (_hashCode != other->_hashCode)
        return false;
    else if (size() != other->size())
        return false;
    return ! std::strncmp(_chars, other->_chars, _size);
}

bool HashedLiteral::equalTo(const char *chars, unsigned size) const
{
    if (_size != size)
        return false;

    const char *tmp = _chars;
    for (unsigned i = 0; i < size; i++)
        if (*tmp++ != *chars++)
            return false;

    return true;
}

HashedLiteral::iterator HashedLiteral::begin() const
{ return _chars; }

HashedLiteral::iterator HashedLiteral::end() const
{ return _chars + _size; }

const char *HashedLiteral::chars() const
{ return _chars; }

char HashedLiteral::at(unsigned index) const
{ return _chars[index]; }

unsigned HashedLiteral::size() const
{ return _size; }

unsigned HashedLiteral::hashCode() const
{ return _hashCode; }

unsigned HashedLiteral::hashCode(const char *chars, unsigned size)
{
    unsigned h = 0;
    for (unsigned i = 0; i < size; ++i)
        h = (h >> 5) - h + chars[i];
    return h;
}

Identifier::Identifier(const char *chars, unsigned size)
    : HashedLiteral(chars, size)
{ }

QString Identifier::toString() const
{ return _chars[0] == '!' ? QString::fromUtf8(_chars + 1, _size - 1) : QString::fromUtf8(_chars, _size); }

const Type *Identifier::asBuiltinType() const
{
    if (_chars[0] != '!')
        return 0;

    switch (_chars[1]) {
        case 's': return Control::stringBuiltingType();
        case 'e': return Control::errorBuiltinType();
    }

    return Control::integralBuiltinType();
}

bool Identifier::isBuiltinTypeIdentifier() const
{ return _chars[0] == '!'; }

}   // namespace GoTools

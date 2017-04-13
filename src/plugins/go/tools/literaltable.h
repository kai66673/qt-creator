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

#include <cstring>

namespace GoTools {

class HashedLiteral;

template <typename Literal>
class LiteralTable
{
    LiteralTable(const LiteralTable &other);
    void operator =(const LiteralTable &other);

public:
    typedef Literal *const *iterator;

public:
    LiteralTable()
       : _literals(0),
         _buckets(0),
         _allocatedLiterals(0),
         _literalCount(-1),
         _allocatedBuckets(0)
    { }

    ~LiteralTable()
    {
        reset();
    }

    void reset()
    {
        if (_literals) {
            Literal **lastLiteral = _literals + _literalCount + 1;
            for (Literal **it = _literals; it != lastLiteral; ++it)
                delete *it;
            std::free(_literals);
        }
        if (_buckets)
            std::free(_buckets);

        _literals = 0;
        _buckets = 0;
        _allocatedLiterals = 0;
        _literalCount = -1;
        _allocatedBuckets = 0;
    }

    bool empty() const
    { return _literalCount == -1; }

    unsigned size() const
    { return _literalCount + 1; }

    const Literal *at(unsigned index) const
    { return _literals[index]; }

    iterator begin() const
    { return _literals; }

    iterator end() const
    { return _literals + _literalCount + 1; }

    const Literal *findLiteral(const char *chars, unsigned size) const
    {
        if (_buckets) {
            unsigned h = Literal::hashCode(chars, size);
            Literal *literal = _buckets[h % _allocatedBuckets];
            for (; literal; literal = static_cast<Literal *>(literal->_next)) {
                if (literal->size() == size && ! std::strncmp(literal->chars(), chars, size))
                    return literal;
            }
        }

        return 0;
    }

    const Literal *findOrInsertLiteral(const char *chars, unsigned size)
    {
        if (_buckets) {
            unsigned h = Literal::hashCode(chars, size);
            Literal *literal = _buckets[h % _allocatedBuckets];
            for (; literal; literal = static_cast<Literal *>(literal->_next)) {
                if (literal->size() == size && ! std::strncmp(literal->chars(), chars, size))
                    return literal;
            }
        }

        Literal *literal = new Literal(chars, size);

        if (++_literalCount == _allocatedLiterals) {
            if (! _allocatedLiterals)
                _allocatedLiterals = 4;
            else
                _allocatedLiterals <<= 1;

            _literals = (Literal **) std::realloc(_literals, sizeof(Literal *) * _allocatedLiterals);
        }

        _literals[_literalCount] = literal;

        if (! _buckets || _literalCount * 5 >= _allocatedBuckets * 3)
            rehash();
        else {
            unsigned h = literal->hashCode() % _allocatedBuckets;
            literal->_next = _buckets[h];
            _buckets[h] = literal;
        }

        return literal;
    }

protected:
    void rehash()
    {
       if (_buckets)
           std::free(_buckets);

       if (! _allocatedBuckets)
           _allocatedBuckets = 4;
       else
           _allocatedBuckets <<= 1;

       _buckets = (Literal **) std::calloc(_allocatedBuckets, sizeof(Literal *));

       Literal **lastLiteral = _literals + (_literalCount + 1);

       for (Literal **it = _literals; it != lastLiteral; ++it) {
           Literal *literal = *it;
           unsigned h = literal->hashCode() % _allocatedBuckets;

           literal->_next = _buckets[h];
           _buckets[h] = literal;
       }
    }

protected:
    Literal **_literals;
    Literal **_buckets;
    int _allocatedLiterals;
    int _literalCount;
    int _allocatedBuckets;
};

}   // namespace GoTools

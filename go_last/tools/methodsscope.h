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

#include "astfwd.h"
#include "literals.h"

#include <QStringList>

#include <cstring>

namespace GoTools {

class FuncDeclAST;
class Identifier;

class MethodSymbol;
class TypeSymbol;

template <typename S> class HashTable {
public:
    HashTable()
        : _symbols(0)
        , _hash(0)
        , _allocatedSymbols(0)
        , _symbolCount(-1)
        , _hashSize(0)
    { }

    ~HashTable() {
        if (_symbols)
            free(_symbols);
        if (_hash)
            free(_hash);
    }

    S *enterSymbol(const Identifier *identifier) {
        if (S *alt = lookat(identifier))
            return alt;

        if (++_symbolCount == _allocatedSymbols) {
            _allocatedSymbols <<= 1;
            if (! _allocatedSymbols)
                _allocatedSymbols = DefaultInitialSize;

            _symbols = reinterpret_cast<S **>(realloc(_symbols, sizeof(S *) * _allocatedSymbols));
            memset(_symbols + _symbolCount, 0, sizeof(S *) * (_allocatedSymbols - _symbolCount));
        }

        S *symbol = new S(identifier);

        symbol->_index = _symbolCount;
        _symbols[_symbolCount] = symbol;

        if (_symbolCount * 5 >= _hashSize * 3)
            rehash();
        else {
            const unsigned h = hashValue(symbol);
            symbol->_next = _hash[h];
            _hash[h] = symbol;
        }

        return symbol;
    }

    bool isEmpty() const { return _symbolCount == -1; }

    unsigned symbolCount() const { return _symbolCount + 1; }
    S **firstSymbol() const { return _symbols; }
    S **lastSymbol() const { return _symbols + _symbolCount + 1; }

    S *symbolAt(unsigned index) const {
        if (! _symbols)
            return 0;
        return _symbols[index];
    }

    S *lookat(const HashedLiteral *name) const {
        if (! _hash || ! name)
            return 0;

        for (S *symbol = _hash[name->hashCode() % _hashSize];
             symbol; symbol = symbol->_next) {
            if (symbol->m_identifier->equalTo(name))
                return symbol;
        }

        return 0;
    }

private:
    enum { DefaultInitialSize = 4 };

    unsigned hashValue(S *symbol) const {
        if (! symbol)
            return 0;

        return symbol->m_identifier->hashCode() % _hashSize;
    }

    void rehash() {
        _hashSize <<= 1;

        if (! _hashSize)
            _hashSize = DefaultInitialSize;

        _hash = reinterpret_cast<S **>(realloc(_hash, sizeof(S *) * _hashSize));
        std::memset(_hash, 0, sizeof(S *) * _hashSize);

        for (int index = 0; index < _symbolCount + 1; ++index) {
            S *symbol = _symbols[index];
            const unsigned h = hashValue(symbol);
            symbol->_next = _hash[h];
            _hash[h] = symbol;
        }
    }

    S **_symbols;
    S **_hash;
    int _allocatedSymbols;
    int _symbolCount;
    int _hashSize;
};

class MethodSymbol
{
    MethodSymbol(const MethodSymbol &other);
    void operator =(const MethodSymbol &other);

    friend class HashTable<MethodSymbol>;

public:
    MethodSymbol(const Identifier *id);

    void setFuncDecl(FuncDeclAST *funcDecl);
    FuncDeclAST *funcDecl() const;

private:
    unsigned _index;
    MethodSymbol *_next;

    FuncDeclAST *m_funcDecl;
    const Identifier *m_identifier;
};

class TypeSymbol
{
    TypeSymbol(const TypeSymbol &other);
    void operator =(const TypeSymbol &other);

    friend class HashTable<TypeSymbol>;

public:
    TypeSymbol(const Identifier *identifier);

    void addMethod(FuncDeclAST *funcDecl);
    MethodSymbol *lookup(const Identifier *funcId);
    QStringList typeMethods();

private:
    unsigned _index;
    TypeSymbol *_next;

    const Identifier *m_identifier;
    HashTable<MethodSymbol> m_methods;
};

class MethodsScope
{
public:
    void addMethod(const Identifier *typeId, FuncDeclAST *funcDecl);
    FuncDeclAST *lookup(const Identifier *typeId, const Identifier *funcId);
    QStringList typeMethods(const Identifier *typeId);

private:
    HashTable<TypeSymbol> m_typeToMethods;
};

}   // namespace GoTools

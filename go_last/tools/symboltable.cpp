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
#include "symboltable.h"

namespace GoTools {

SymbolTable::SymbolTable()
    : _symbols(0)
    , _hash(0)
    , _allocatedSymbols(0)
    , _symbolCount(-1)
    , _hashSize(0)
{ }

SymbolTable::~SymbolTable()
{
    if (_symbols)
        free(_symbols);
    if (_hash)
        free(_hash);
}

void SymbolTable::enterSymbol(Symbol *symbol)
{
    if (++_symbolCount == _allocatedSymbols) {
        _allocatedSymbols <<= 1;
        if (! _allocatedSymbols)
            _allocatedSymbols = DefaultInitialSize;

        _symbols = reinterpret_cast<Symbol **>(realloc(_symbols, sizeof(Symbol *) * _allocatedSymbols));
        memset(_symbols + _symbolCount, 0, sizeof(Symbol *) * (_allocatedSymbols - _symbolCount));
    }

    symbol->_index = _symbolCount;
    _symbols[_symbolCount] = symbol;

    if (_symbolCount * 5 >= _hashSize * 3)
        rehash();
    else {
        const unsigned h = hashValue(symbol);
        symbol->_next = _hash[h];
        _hash[h] = symbol;
    }
}

bool SymbolTable::isEmpty() const
{ return _symbolCount == -1; }

unsigned SymbolTable::symbolCount() const
{ return _symbolCount + 1; }

Symbol *SymbolTable::symbolAt(unsigned index) const
{
    if (! _symbols)
        return 0;
    return _symbols[index];
}

SymbolTable::iterator SymbolTable::firstSymbol() const
{ return _symbols; }

SymbolTable::iterator SymbolTable::lastSymbol() const
{ return _symbols + _symbolCount + 1; }

Symbol *SymbolTable::lookat(const HashedLiteral *name, Symbol::Kind kind) const
{
    if (! _hash || ! name)
        return 0;

    for (Symbol *symbol = _hash[name->hashCode() % _hashSize];
         symbol; symbol = symbol->_next) {
        if (symbol->kind() == kind && symbol->identifier()->equalTo(name))
            return symbol;
    }

    return 0;
}

Symbol *SymbolTable::lookat(const HashedLiteral *name) const
{
    if (! _hash || ! name)
        return 0;

    for (Symbol *symbol = _hash[name->hashCode() % _hashSize];
         symbol; symbol = symbol->_next) {
        if (symbol->identifier()->equalTo(name))
            return symbol;
    }

    return 0;
}

unsigned SymbolTable::hashValue(Symbol *symbol) const
{
    if (! symbol)
        return 0;

    return symbol->_identifier->hashCode() % _hashSize;
}

void SymbolTable::rehash()
{
    _hashSize <<= 1;

    if (! _hashSize)
        _hashSize = DefaultInitialSize;

    _hash = reinterpret_cast<Symbol **>(realloc(_hash, sizeof(Symbol *) * _hashSize));
    std::memset(_hash, 0, sizeof(Symbol *) * _hashSize);

    for (int index = 0; index < _symbolCount + 1; ++index) {
        Symbol *symbol = _symbols[index];
        const unsigned h = hashValue(symbol);
        symbol->_next = _hash[h];
        _hash[h] = symbol;
    }
}

}   // namespace GoTools

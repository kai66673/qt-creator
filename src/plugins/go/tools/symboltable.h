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

#include "scope.h"
#include "symbol.h"
#include "literals.h"

namespace GoTools {

class SymbolTable
{
    SymbolTable(const SymbolTable &other);
    void operator =(const SymbolTable &other);

public:
    typedef Symbol **iterator;

public:
    /// Constructs an empty Scope.
    SymbolTable();

    /// Destroy this scope.
    ~SymbolTable();

    /// Adds a Symbol to this Scope.
    void enterSymbol(Symbol *symbol);

    /// Returns true if this Scope is empty; otherwise returns false.
    bool isEmpty() const;

    /// Returns the number of symbols is in the scope.
    unsigned symbolCount() const;

    /// Returns the Symbol at the given position.
    Symbol *symbolAt(unsigned index) const;

    /// Returns the first Symbol in the scope.
    iterator firstSymbol() const;

    /// Returns the last Symbol in the scope.
    iterator lastSymbol() const;

    Symbol *lookat(const HashedLiteral *name, Symbol::Kind kind) const;
    Symbol *lookat(const HashedLiteral *name) const;

private:
    enum { DefaultInitialSize = 8 };

    /// Returns the hash value for the given Symbol.
    unsigned hashValue(Symbol *symbol) const;

    /// Updates the hash table.
    void rehash();

    Symbol **_symbols;
    Symbol **_hash;
    int _allocatedSymbols;
    int _symbolCount;
    int _hashSize;
};

}   // namespace GoTools

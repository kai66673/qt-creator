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

#include "goworkingcopy.h"
#include "scope.h"

namespace GoTools {

class GoPackageCache;
class GoCheckSymbols;

class ResolveContext
{
public:
    ResolveContext(GoSource *source, bool protectCache = true);
    virtual ~ResolveContext();

    Scope *switchScope(Scope *scope);
    Scope *currentScope() const;

    bool isValidResolveContext() const;
    PackageType *packageTypeForAlias(const QString &packageAlias, FileScope *fileScope = 0);
    PackageType *fileScopePackageType(const FileScope *fileScope) const;

    QList<GoSource *> sources() const;

protected:
    bool m_protectCache;
    GoPackageCache *m_cache;
    FileAST *m_initialFileAst;
    Scope *m_currentScope;
    FileScope *m_initialFileScope;
    bool m_isValid;
    bool m_workingCopyIsObtained;
    WorkingCopy m_workingCopy;
};

const Type *tryResolveNamedType(ResolveContext *resolver, ExprAST *x);
const Type *tryCheckNamedType(GoCheckSymbols *resolver, ExprAST *x);

}   // namespace GoTools

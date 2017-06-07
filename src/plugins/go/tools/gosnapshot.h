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

#include "gosource.h"
#include "gopackagetypes.h"
#include "astfwd.h"
#include "gopackage.h"
#include "scope.h"
#include "types.h"

#include <QHash>

namespace GoTools {

class PackageType: public Type
{
    friend class GoSnapshot;

public:
    PackageType();
    virtual ~PackageType();

    Symbol *lookupMethod(const Identifier *typeId, const Identifier *funcId, GoSnapshot *snapshot);
    void fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions,
                     const Identifier *typeId, GoSnapshot *snapshot);

    // LookupContext implemntation
    virtual Symbol *lookupMember(const IdentAST *ident, ExprTypeResolver *resolver) const override;
    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ExprTypeResolver *resolver, Predicate = 0) const override;

    // Type implemntation
    virtual const Type *indexType(ExprTypeResolver *) const override;
    virtual const Type *elementsType(ExprTypeResolver *) const override;
    virtual const Type *calleeType(int, ExprTypeResolver *) const override;
    virtual const Type *chanValueType() const override;

    virtual QString describe() const override { return QLatin1String("package"); }

private:
    int m_first;
    int m_last;
};

class GoSnapshot
{
    friend class PackageType;

    struct ScopeRecord {
        GoSource::Ptr source;
        FileScope *scope;
        PackageType *lookupcontext;
        QHash<QString, PackageType *> aliasToLookupContext;
    };

    typedef QList<ScopeRecord *> ScopeTable;
    typedef QList<ScopeRecord *>::iterator ScopeTableIterator;
    typedef std::function<void ()> ProtectedTask;

public:
    GoSnapshot(const QHash<GoPackageKey, GoPackage *> &packages);
    ~GoSnapshot();

    void runProtectedTask(ProtectedTask task);

    PackageType *packageTypetAt(int index) const;
    FileScope *fileScopeAt(int index) const;
    PackageType *packageTypeForAlias(int index, const QString &alias);

private:
    void bindScopes();

    QHash<GoPackageKey, PackageType *> m_packages;
    PackageType m_nullPackage;

    QList<ScopeRecord *> m_scopeTable;
};

}   // namespace GoTools

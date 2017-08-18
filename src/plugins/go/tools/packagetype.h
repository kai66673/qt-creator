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

#include "types.h"

namespace GoTools {

class ResolvedType;

class PackageType: public Type
{
public:
    PackageType(QHash<QString, GoSource::Ptr> &sources);
    virtual ~PackageType();

    Symbol *lookupMethod(const Identifier *typeId, const Identifier *funcId, ResolveContext *);
    void fillMethods(QList<TextEditor::AssistProposalItemInterface *> &completions,
                     const Identifier *typeId, ResolveContext *);
    Symbol *lookup(const Identifier *ident) const;

    // LookupContext implementation
    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *,
                                 int refLevel = 0) const override;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *,
                                       int refLevel = 0,
                                       Predicate = 0) const override;

    virtual ResolvedType indexType(ResolveContext *, int = 0) const override;
    virtual ResolvedType elementsType(ResolveContext *, int = 0) const override;
    virtual ResolvedType chanValueType(ResolveContext *, int = 0) const override;

    virtual QString describe() const override { return QLatin1String("package"); }
    virtual BuiltingKind builtinKind(ResolveContext *, int = 0) const override { return Other; }

private:
    QHash<QString, GoSource::Ptr> &m_sources;
};

}   // namespace GoTools

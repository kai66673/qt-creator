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

#include "symbol.h"
#include "astfwd.h"
#include "gosource.h"

#include <texteditor/codeassist/assistproposaliteminterface.h>

#include <functional>

namespace GoTools {

class ResolveContext;

class LookupContext
{
public:
    typedef std::function<bool (Symbol *)> Predicate;

    virtual ~LookupContext() {}

    virtual Symbol *lookupMember(const IdentAST *ident,
                                 ResolveContext *resolver,
                                 int refLevel = 0) const = 0;

    virtual void fillMemberCompletions(QList<TextEditor::AssistProposalItemInterface *> &completions,
                                       ResolveContext *resolver,
                                       int refLevel = 0,
                                       Predicate predicate = 0) const = 0;
};

}   // namespace GoTools

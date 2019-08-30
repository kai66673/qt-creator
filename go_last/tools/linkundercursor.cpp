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
#include "ast.h"
#include "linkundercursor.h"

namespace GoTools {

LinkUnderCursor::LinkUnderCursor(GoSource::Ptr doc)
    : SymbolUnderCursor(doc)
{ }

Utils::Link LinkUnderCursor::link(unsigned pos)
{
    m_pos = pos;
    defineLinkUnderCursor();

    if (m_symbol && m_token && m_symbol->owner()) {
        Utils::Link link;
        link.linkTextStart = m_token->begin();
        link.linkTextEnd = m_token->end();
        m_symbol->fileScope()->fillLink(link, m_symbol->sourceLocation());
        return link;
    }

    return Utils::Link();
}

void LinkUnderCursor::defineLinkUnderCursor()
{
    m_symbol = 0;
    m_token = 0;

    if (m_doc && isValidResolveContext())
        acceptForPosition(m_initialFileAst->decls);
}

}   // namespace GoTools

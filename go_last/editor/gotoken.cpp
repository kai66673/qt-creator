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

#include "gotoken.h"

#include <QHash>
#include <QVector>

namespace GoEditor {
namespace Internal {

GoToken::GoToken(int start, int count, TokenKind kind, const QChar *text) :
    m_start(start),
    m_count(count),
    m_kind(kind)
{
    /// TODO: prevent compare duplication
    if (text && kind == T_KEYWORD)
        parseKeyword(text + start);
}

void GoToken::parseKeyword(const QChar *text)
{
    const ushort *t = (const ushort *)text;
    switch (m_count) {
        case 2:
            if (t[0] == 'i' && t[1] == 'f')
                m_kind = T_IF;
            break;

        case 3:
            switch (t[0]) {
                case 'f':
                    if (t[1] == 'o' && t[2] == 'r')
                        m_kind = T_FOR;
                    break;
                case 'v':
                    if (t[1] == 'a' && t[2] == 'r')
                        m_kind = T_VAR;
                    break;
            }
            break;

        case 4:
            switch (t[0]) {
                case 'c':
                    if (t[1] == 'a' && t[2] == 's' && t[3] == 'e')
                        m_kind = T_CASE;
                    break;
                case 'f':
                    if (t[1] == 'u' && t[2] == 'n' && t[3] == 'c')
                        m_kind = T_FUNC;
                    break;
                case 't':
                    if (t[1] == 'y' && t[2] == 'p' && t[3] == 'e')
                        m_kind = T_TYPE;
                    break;
            }
            break;

        case 5:
            if (t[0] == 'c' && t[1] == 'o' && t[2] == 'n' && t[3] == 's' && t[4] == 't')
                m_kind = T_CONST;
            break;

        case 6:
            switch (t[0]) {
                case 'i':
                    if (t[1] == 'm' && t[2] == 'p' && t[3] == 'o' && t[4] == 'r' && t[5] == 't')
                        m_kind = T_IMPORT;
                    break;
                case 's':
                    if (t[1] == 'e') {
                        if (t[2] == 'l' && t[3] == 'e' && t[4] == 'c' && t[5] == 't')
                            m_kind = T_SELECT;
                    } else if (t[1] == 't') {
                        if (t[2] == 'r' && t[3] == 'u' && t[4] == 'c' && t[5] == 't')
                            m_kind = T_STRUCT;
                    } else if (t[1] == 'w') {
                        if (t[2] == 'i' && t[3] == 't' && t[4] == 'c' && t[5] == 'h')
                            m_kind = T_SWITCH;
                    }
                    break;
            }
            break;

        case 7:
            switch (t[0]) {
                case 'd':
                    if (t[1] == 'e' && t[2] == 'f' && t[3] == 'a' && t[4] == 'u' && t[5] == 'l' && t[6] == 't')
                        m_kind = T_DEFAULT;
                    break;
                case 'p':
                    if (t[1] == 'a' && t[2] == 'c' && t[3] == 'k' && t[4] == 'a' && t[5] == 'g' && t[6] == 'e')
                        m_kind = T_PACKAGE;
                    break;
            }
            break;

        case 9:
            if (t[0] == 'i' && t[1] == 'n' && t[2] == 't' && t[3] == 'e' && t[4] == 'r' &&
                t[5] == 'f' && t[6] == 'a' && t[7] == 'c' && t[8] == 'e')
                m_kind = T_INTERFACE;
            break;
    }
}

} // namespace Internal
} // namespace GoEditor

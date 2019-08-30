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

#include <QSettings>
#include <QString>

namespace GoLang {

class GoToolsSettings
{
public:
    GoToolsSettings();

    void toSettings(const QString &category, QSettings *s) const;
    bool fromSettings(const QString &category, QSettings *s);

    bool goImportsEnabled() const;
    void setGoImportsEnabled(bool goImportsEnabled);

    bool goFmtEnabled() const;
    void setGoFmtEnabled(bool goFmtEnabled);

    bool equals(const GoToolsSettings &other) const;

private:
    bool m_goImportsEnabled;
    bool m_goFmtEnabled;
};

inline bool operator==(const GoToolsSettings &t1, const GoToolsSettings &t2) { return t1.equals(t2); }
inline bool operator!=(const GoToolsSettings &t1, const GoToolsSettings &t2) { return !t1.equals(t2); }

}   // namespace GoLang

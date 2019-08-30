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

class GoGeneralSettings
{
public:
    GoGeneralSettings();

    void toSettings(const QString &category, QSettings *s) const;
    bool fromSettings(const QString &category, QSettings *s);

    QString goPath() const;
    void setGoPath(const QString &goPath);

    QString goRoot() const;
    void setGoRoot(const QString &goRoot);

    QString goOs() const;
    void setGoOs(const QString &goOs);

    QString goArch() const;
    void setGoArch(const QString &goArch);

    bool equals(const GoGeneralSettings &other) const;

private:
    QString m_goPath;
    QString m_goRoot;
    QString m_goOs;
    QString m_goArch;
};

inline bool operator==(const GoGeneralSettings &s1, const GoGeneralSettings &s2) { return s1.equals(s2); }
inline bool operator!=(const GoGeneralSettings &s1, const GoGeneralSettings &s2) { return !s1.equals(s2); }

}   // namespace GoLang

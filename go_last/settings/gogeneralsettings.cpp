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
#include "gogeneralsettings.h"
#include "golangconstants.h"

namespace GoLang {

GoGeneralSettings::GoGeneralSettings()
{ }

void GoGeneralSettings::toSettings(const QString &category, QSettings *s) const
{
    s->beginGroup(category);
    s->setValue(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOPATH_KEY), m_goPath);
    s->setValue(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOROOT_KEY), m_goRoot);
    s->setValue(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOOS_KEY), m_goOs);
    s->setValue(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOARCH_KEY), m_goArch);
    s->endGroup();
}

bool GoGeneralSettings::fromSettings(const QString &category, QSettings *s)
{
    s->beginGroup(category);
    m_goPath = s->value(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOPATH_KEY), "").toString();
    m_goRoot = s->value(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOROOT_KEY), "").toString();
    m_goOs = s->value(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOOS_KEY), "").toString();
    m_goArch = s->value(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOARCH_KEY), "").toString();
    s->endGroup();

    return true;
}

QString GoGeneralSettings::goPath() const
{ return m_goPath; }

void GoGeneralSettings::setGoPath(const QString &goPath)
{ m_goPath = goPath; }

QString GoGeneralSettings::goRoot() const
{ return m_goRoot; }

void GoGeneralSettings::setGoRoot(const QString &goRoot)
{ m_goRoot = goRoot; }

QString GoGeneralSettings::goOs() const
{ return m_goOs; }

void GoGeneralSettings::setGoOs(const QString &goOs)
{ m_goOs = goOs; }

QString GoGeneralSettings::goArch() const
{ return m_goArch; }

void GoGeneralSettings::setGoArch(const QString &goArch)
{ m_goArch = goArch; }

bool GoGeneralSettings::equals(const GoGeneralSettings &other) const
{
    return m_goPath == other.m_goPath && m_goRoot == other.m_goRoot &&
           m_goOs == other.m_goOs && m_goArch == other.m_goArch;
}

}   // namespace GoLang

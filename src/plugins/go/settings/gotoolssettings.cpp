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
#include "gotoolssettings.h"
#include "golangconstants.h"

namespace GoLang {

GoToolsSettings::GoToolsSettings()
    : m_goImportsEnabled(false)
    , m_goFmtEnabled(false)
{ }

void GoToolsSettings::toSettings(const QString &category, QSettings *s) const
{
    s->beginGroup(category);
    s->setValue(QLatin1String(Constants::C_GOLANG_TOOLS_SETTINGS_GOIMPORTS_KEY), m_goImportsEnabled);
    s->setValue(QLatin1String(Constants::C_GOLANG_TOOLS_SETTINGS_GOFMT_KEY), m_goFmtEnabled);
    s->endGroup();
}

bool GoToolsSettings::fromSettings(const QString &category, QSettings *s)
{
    s->beginGroup(category);
    m_goImportsEnabled = s->value(Constants::C_GOLANG_TOOLS_SETTINGS_GOIMPORTS_KEY, false).toBool();
    m_goFmtEnabled = s->value(Constants::C_GOLANG_TOOLS_SETTINGS_GOFMT_KEY, false).toBool();
    s->endGroup();

    return true;
}

bool GoToolsSettings::goImportsEnabled() const
{ return m_goImportsEnabled; }

void GoToolsSettings::setGoImportsEnabled(bool goImportsEnabled)
{ m_goImportsEnabled = goImportsEnabled; }

bool GoToolsSettings::goFmtEnabled() const
{ return m_goFmtEnabled; }

void GoToolsSettings::setGoFmtEnabled(bool goFmtEnabled)
{ m_goFmtEnabled = goFmtEnabled; }

bool GoToolsSettings::equals(const GoToolsSettings &other) const
{
    return m_goImportsEnabled == other.m_goImportsEnabled &&
           m_goFmtEnabled == other.m_goFmtEnabled;
}

}   // namespace GoLang

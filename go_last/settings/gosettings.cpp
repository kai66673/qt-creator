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
#include "gosettings.h"
#include "golangconstants.h"

#include <coreplugin/icore.h>

#include <goplugin.h>

namespace GoLang {

class GoSettingsPrivate
{
public:
    GoGeneralSettings m_generalSettings;
    GoToolsSettings m_toolsSettings;
};

static GoSettingsPrivate *d = 0;
static GoSettings *m_instance = 0;

GoSettings::GoSettings(QObject *parent)
    : QObject(parent)
{
    d = new GoSettingsPrivate;
    m_instance = this;

    QSettings *settings = Core::ICore::settings();
    d->m_generalSettings.fromSettings(Constants::C_GOLANG_GENERAL_SETTINGS_GROUP, settings);
    d->m_toolsSettings.fromSettings(Constants::C_GOLANG_TOOLS_SETTINGS_GROUP, settings);
}

GoSettings::~GoSettings()
{
    delete d;
    m_instance = 0;
}

GoSettings *GoSettings::instance()
{ return m_instance; }

const GoGeneralSettings &GoSettings::generalSettings()
{ return d->m_generalSettings; }

void GoSettings::setGeneralSettings(const GoGeneralSettings &generalSettings)
{
    if (generalSettings != d->m_generalSettings) {
        QSettings *settings = Core::ICore::settings();
        d->m_generalSettings = generalSettings;
        d->m_generalSettings.toSettings(Constants::C_GOLANG_GENERAL_SETTINGS_GROUP, settings);
        m_instance->emitGeneralSettingsChanged(generalSettings);
    }
}

const GoToolsSettings &GoSettings::toolsSettings()
{ return d->m_toolsSettings; }

void GoSettings::setToolsSettings(const GoToolsSettings &toolsSettings)
{
    if (toolsSettings != d->m_toolsSettings) {
        QSettings *settings = Core::ICore::settings();
        d->m_toolsSettings = toolsSettings;
        d->m_toolsSettings.toSettings(Constants::C_GOLANG_TOOLS_SETTINGS_GROUP, settings);
    }
}

bool GoSettings::sortedEditorDocumentOutline()
{ return Core::ICore::settings()->value(Constants::C_GOLANG_TOOLS_SORT_EDITOR_OUTLINE, false).toBool(); }

void GoSettings::setSortedEditorDocumentOutline(bool sorted)
{
    Core::ICore::settings()->setValue(Constants::C_GOLANG_TOOLS_SORT_EDITOR_OUTLINE, sorted);
    Go::Internal::GoPlugin::instance()->emitOutlineSortingChanged(sorted);
}

void GoSettings::emitGeneralSettingsChanged(const GoGeneralSettings &generalSettings)
{ emit generalSettingsChanged(generalSettings); }

}   // namespace GoLang

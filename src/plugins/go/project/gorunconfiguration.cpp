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

#include "gorunconfiguration.h"
#include "gobuildconfiguration.h"
#include "golangconstants.h"
#include "gorunconfigurationwidget.h"

#include <projectexplorer/localenvironmentaspect.h>
#include <projectexplorer/runconfigurationaspects.h>
#include <utils/environment.h>

#include <QFileInfo>
#include <QDir>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

GoRunConfiguration::GoRunConfiguration(ProjectExplorer::Target *target, Core::Id id)
    : RunConfiguration(target, id)
    , m_buildConfiguration(nullptr)
{
    addAspect<ExecutableAspect>();
    addAspect<ArgumentsAspect>();
    auto envAspect = addAspect<LocalEnvironmentAspect>(target, LocalEnvironmentAspect::BaseEnvironmentModifier());
    addAspect<WorkingDirectoryAspect>(envAspect);
    addAspect<TerminalAspect>();

    // Connect target signals
    connect(this->target(), &Target::activeBuildConfigurationChanged,
            this, &GoRunConfiguration::updateConfiguration);

    updateConfiguration();
}

QWidget *GoRunConfiguration::createConfigurationWidget()
{
    return new GoRunConfigurationWidget(this);
}

QVariantMap GoRunConfiguration::toMap() const
{
    auto result = RunConfiguration::toMap();
    result[Constants::C_GORUNCONFIGURATION_EXECUTABLE_KEY] = m_executable;
    return result;
}

bool GoRunConfiguration::fromMap(const QVariantMap &map)
{
    bool result = RunConfiguration::fromMap(map);
    if (!result)
        return result;
    m_executable = map[Constants::C_GORUNCONFIGURATION_EXECUTABLE_KEY].toString();
    return true;
}

void GoRunConfiguration::updateConfiguration()
{
    GoBuildConfiguration *buildConfiguration = qobject_cast<GoBuildConfiguration *>(activeBuildConfiguration());
    QTC_ASSERT(buildConfiguration, return);
    setActiveBuildConfiguration(buildConfiguration);
    const QFileInfo outFileInfo = buildConfiguration->outFilePath().toFileInfo();
    extraAspect<ExecutableAspect>()->setExecutable(FileName::fromString(outFileInfo.absoluteFilePath()));
    const QString workingDirectory = outFileInfo.absoluteDir().absolutePath();
    extraAspect<WorkingDirectoryAspect>()->setDefaultWorkingDirectory(FileName::fromString(workingDirectory));

    setDisplayName(outFileInfo.absoluteFilePath());
    setDefaultDisplayName(outFileInfo.absoluteFilePath());
}

void GoRunConfiguration::setActiveBuildConfiguration(GoBuildConfiguration *activeBuildConfiguration)
{
    if (m_buildConfiguration == activeBuildConfiguration)
        return;

    if (m_buildConfiguration) {
        disconnect(m_buildConfiguration, &GoBuildConfiguration::buildDirectoryChanged,
                   this, &GoRunConfiguration::updateConfiguration);
        disconnect(m_buildConfiguration, &GoBuildConfiguration::outFilePathChanged,
                   this, &GoRunConfiguration::updateConfiguration);
    }

    m_buildConfiguration = activeBuildConfiguration;

    if (m_buildConfiguration) {
        connect(m_buildConfiguration, &GoBuildConfiguration::buildDirectoryChanged,
                this, &GoRunConfiguration::updateConfiguration);
        connect(m_buildConfiguration, &GoBuildConfiguration::outFilePathChanged,
                this, &GoRunConfiguration::updateConfiguration);
    }
}

}   // namespace GoLang

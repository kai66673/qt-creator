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

#include "gobuildconfiguration.h"
#include "gobuildconfigurationwidget.h"
#include "gocompilerbuildstep.h"
#include "golangconstants.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/buildsteplist.h>
#include <utils/qtcassert.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

GoBuildConfiguration::GoBuildConfiguration(ProjectExplorer::Target *target, Core::Id id)
    : BuildConfiguration(target, id)
{ }

NamedWidget *GoBuildConfiguration::createConfigWidget()
{
    return new GoBuildConfigurationWidget(this);
}

BuildConfiguration::BuildType GoBuildConfiguration::buildType() const
{
    return BuildConfiguration::Unknown;
}

bool GoBuildConfiguration::fromMap(const QVariantMap &map)
{
    if (!BuildConfiguration::fromMap(map))
        return false;

    if (!canRestore(map))
        return false;

    const QString displayName = map[Constants::C_GOBUILDCONFIGURATION_DISPLAY_KEY].toString();
    const QString buildDirectory = map[Constants::C_GOBUILDCONFIGURATION_BUILDDIRECTORY_KEY].toString();

    setDisplayName(displayName);
    setBuildDirectory(FileName::fromString(buildDirectory));

    return true;
}

QVariantMap GoBuildConfiguration::toMap() const
{
    QVariantMap result = BuildConfiguration::toMap();
    result[Constants::C_GOBUILDCONFIGURATION_DISPLAY_KEY] = displayName();
    result[Constants::C_GOBUILDCONFIGURATION_BUILDDIRECTORY_KEY] = buildDirectory().toString();
    return result;
}

FileName GoBuildConfiguration::outFilePath() const
{
    const GoCompilerBuildStep *step = goCompilerBuildStep();
    QTC_ASSERT(step, return FileName());
    return step->outFilePath();
}

FileName GoBuildConfiguration::cacheDirectory() const
{
    return buildDirectory().appendPath(QStringLiteral("gocache"));
}

bool GoBuildConfiguration::canRestore(const QVariantMap &map)
{
    return idFromMap(map) == Constants::C_GOBUILDCONFIGURATION_ID;
}

bool GoBuildConfiguration::hasGoCompilerGetStep() const
{
    BuildStepList *steps = stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    QTC_ASSERT(steps, return false);
    return steps->contains(Constants::C_GOCOMPILERGETSTEP_ID);
}

bool GoBuildConfiguration::hasGoCompilerBuildStep() const
{
    BuildStepList *steps = stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    QTC_ASSERT(steps, return false);
    return steps->contains(Constants::C_GOCOMPILERBUILDSTEP_ID);
}

bool GoBuildConfiguration::hasGoCompilerCleanStep() const
{
    BuildStepList *steps = stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    QTC_ASSERT(steps, return false);
    return steps ? steps->contains(Constants::C_GOCOMPILERCLEANSTEP_ID) : false;
}

const GoCompilerBuildStep *GoBuildConfiguration::goCompilerBuildStep() const
{
    BuildStepList *steps = stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    QTC_ASSERT(steps, return nullptr);
    foreach (BuildStep *step, steps->steps())
        if (step->id() == Constants::C_GOCOMPILERBUILDSTEP_ID)
            return qobject_cast<GoCompilerBuildStep *>(step);
    return nullptr;
}

}   // namespace GoLang

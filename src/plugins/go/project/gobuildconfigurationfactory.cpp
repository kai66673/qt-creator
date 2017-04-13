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

#include "gobuildconfigurationfactory.h"
#include "gobuildconfiguration.h"
#include "gocompilerbuildstep.h"
#include "goproject.h"
#include "golangconstants.h"

#include <coreplugin/documentmanager.h>
#include <projectexplorer/projectmacroexpander.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/target.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/buildinfo.h>
#include <utils/qtcassert.h>
#include <utils/mimetypes/mimedatabase.h>

#include <memory>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

GoBuildConfigurationFactory::GoBuildConfigurationFactory(QObject *parent)
    : IBuildConfigurationFactory(parent)
{ }

QList<BuildInfo *> GoBuildConfigurationFactory::availableBuilds(const Target *parent) const
{
    // Retrieve the project path
    GoProject *project = qobject_cast<GoProject *>(parent->project());
    QTC_ASSERT(project, return {});

    QFileInfo projFileInfo(project->projectFilePath().toFileInfo());
    ProjectExplorer::BuildInfo *goBuild = new ProjectExplorer::BuildInfo(this);
    goBuild->displayName = tr("Default");
    goBuild->buildDirectory = Utils::FileName::fromString(projFileInfo.absolutePath());
    goBuild->kitId = parent->kit()->id();
    goBuild->typeName = QStringLiteral("Default");
    return { goBuild };
}

QList<BuildInfo *> GoBuildConfigurationFactory::availableSetups(const Kit *k, const QString &projectPath) const
{
    QFileInfo projFileInfo(projectPath);
    ProjectExplorer::BuildInfo *goBuild = new ProjectExplorer::BuildInfo(this);
    goBuild->displayName = tr("Default");
    goBuild->buildDirectory = Utils::FileName::fromString(projFileInfo.absolutePath());
    goBuild->kitId = k->id();
    goBuild->typeName = QStringLiteral("Default");
    return { goBuild };
}

BuildConfiguration *GoBuildConfigurationFactory::create(Target *parent, const BuildInfo *info) const
{
    GoProject *project = qobject_cast<GoProject *>(parent->project());
    QTC_ASSERT(project, return nullptr);

    // Create the build configuration and initialize it from build info
    GoBuildConfiguration *result = new GoBuildConfiguration(parent);
    result->setDisplayName(info->displayName);
    result->setDefaultDisplayName(info->displayName);
    result->setBuildDirectory(info->buildDirectory);

    // Add Go compiler build step
    BuildStepList *buildSteps = result->stepList(Core::Id(ProjectExplorer::Constants::BUILDSTEPS_BUILD));
    buildSteps->appendStep(new GoCompilerBuildStep(buildSteps, GoCompilerBuildStep::Get));
    buildSteps->appendStep(new GoCompilerBuildStep(buildSteps, GoCompilerBuildStep::Build));


    // Add clean step
    BuildStepList *cleanSteps = result->stepList(Core::Id(ProjectExplorer::Constants::BUILDSTEPS_CLEAN));
    cleanSteps->appendStep(new GoCompilerBuildStep(cleanSteps, GoCompilerBuildStep::Clean));

    return result;
}

bool GoBuildConfigurationFactory::canRestore(const Target *parent, const QVariantMap &map) const
{
    Q_UNUSED(parent);
    return GoBuildConfiguration::canRestore(map);
}

BuildConfiguration *GoBuildConfigurationFactory::restore(Target *parent, const QVariantMap &map)
{
    QTC_ASSERT(canRestore(parent, map), return nullptr);

    // Create the build configuration
    auto result = new GoBuildConfiguration(parent);

    // Restore from map
    bool status = result->fromMap(map);
    QTC_ASSERT(status, return nullptr);

    return result;
}

bool GoBuildConfigurationFactory::canClone(const Target *parent, BuildConfiguration *product) const
{
    QTC_ASSERT(parent, return false);
    QTC_ASSERT(product, return false);
    return product->id() == Constants::C_GOBUILDCONFIGURATION_ID;
}

BuildConfiguration *GoBuildConfigurationFactory::clone(Target *parent, BuildConfiguration *product)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(product, return nullptr);
    auto buildConfiguration = qobject_cast<GoBuildConfiguration *>(product);
    QTC_ASSERT(buildConfiguration, return nullptr);
    std::unique_ptr<GoBuildConfiguration> result(new GoBuildConfiguration(parent));
    return result->fromMap(buildConfiguration->toMap()) ? result.release() : nullptr;
}

int GoBuildConfigurationFactory::priority(const Kit *k, const QString &projectPath) const
{
    if (k && Utils::mimeTypeForFile(projectPath).matchesName(QLatin1String(Constants::C_GO_PROJECT_MIMETYPE)))
        return 0;
    return -1;
}

int GoBuildConfigurationFactory::priority(const Target *parent) const
{
    return canHandle(parent) ? 0 : -1;
}

bool GoBuildConfigurationFactory::canHandle(const Target *t) const
{
    if (!t->project()->supportsKit(t->kit()))
        return false;
    return qobject_cast<GoProject *>(t->project());
}

}   // namespace GoLang

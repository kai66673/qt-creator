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

GoBuildConfigurationFactory::GoBuildConfigurationFactory()
    : IBuildConfigurationFactory()
{
    registerBuildConfiguration<GoBuildConfiguration>(Constants::C_GOBUILDCONFIGURATION_ID);
    setSupportedProjectType(Constants::C_GOPROJECT_ID);
    setSupportedProjectMimeTypeName(Constants::C_GO_PROJECT_MIMETYPE);
}

QList<BuildInfo *> GoBuildConfigurationFactory::availableBuilds(const Target *parent) const
{
    // Retrieve the project path
    GoProject *project = qobject_cast<GoProject *>(parent->project());
    QTC_ASSERT(project, return {});

    // Create the build info
    BuildInfo *info = createBuildInfo(parent->kit(), project->projectFilePath().toString());

    info->displayName.clear(); // ask for a name
    info->buildDirectory.clear(); // This depends on the displayName

    return {info};
}

QList<BuildInfo *> GoBuildConfigurationFactory::availableSetups(const Kit *k, const QString &projectPath) const
{
    return { createBuildInfo(k, projectPath) };
}

BuildInfo *GoBuildConfigurationFactory::createBuildInfo(const Kit *k, const QString &projectPath) const
{
    QFileInfo projFileInfo(projectPath);

    ProjectExplorer::BuildInfo *goBuild = new ProjectExplorer::BuildInfo(this);
    goBuild->buildType = ProjectExplorer::BuildConfiguration::Debug;
    goBuild->displayName = ProjectExplorer::BuildConfiguration::buildTypeName(ProjectExplorer::BuildConfiguration::Debug);
    goBuild->buildDirectory = Utils::FileName::fromString(projFileInfo.absolutePath());
    goBuild->kitId = k->id();
    goBuild->typeName = tr("Default");
    return  goBuild;
}

}   // namespace GoLang

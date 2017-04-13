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

#include "gocompilercleanstepfactory.h"
#include "gobuildconfiguration.h"
#include "gocompilerbuildstep.h"
#include "golangconstants.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <memory>

using namespace ProjectExplorer;

namespace GoLang {

GoCompilerCleanStepFactory::GoCompilerCleanStepFactory(QObject *parent)
    : IBuildStepFactory(parent)
{ }

QList<BuildStepInfo> GoCompilerCleanStepFactory::availableSteps(BuildStepList *parent) const
{
    if (parent->id() != ProjectExplorer::Constants::BUILDSTEPS_CLEAN)
        return {};

    auto bc = qobject_cast<GoBuildConfiguration *>(parent->parent());
    if (!bc || bc->hasGoCompilerCleanStep())
        return {};

    return {{ Constants::C_GOCOMPILERCLEANSTEP_ID, tr("Go Compiler Clean Step") }};
}

BuildStep *GoCompilerCleanStepFactory::create(BuildStepList *parent, Core::Id)
{
    return new GoCompilerBuildStep(parent, GoCompilerBuildStep::Clean);
}

BuildStep *GoCompilerCleanStepFactory::clone(BuildStepList *parent, BuildStep *buildStep)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(buildStep, return nullptr);
    std::unique_ptr<GoCompilerBuildStep> result(new GoCompilerBuildStep(parent, GoCompilerBuildStep::Clean));
    return result->fromMap(buildStep->toMap()) ? result.release() : nullptr;
}

}   // namespace GoLang

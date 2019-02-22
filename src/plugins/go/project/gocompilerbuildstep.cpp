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

#include "gocompilerbuildstep.h"
#include "gobuildconfiguration.h"
#include "gocompilerbuildstepconfigwidget.h"
#include "golangconstants.h"
#include "gooutputparser.h"

#include "coreplugin/icore.h"
#include <projectexplorer/task.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/ioutputparser.h>
#include <projectexplorer/processparameters.h>
#include <utils/qtcassert.h>

#include <QDir>


using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

BaseGoCompilerStep::BaseGoCompilerStep(ProjectExplorer::BuildStepList *parentList, GoOption option)
    : AbstractProcessStep(parentList,
                          option == Clean ? Constants::C_GOCOMPILERCLEANSTEP_ID
                                          : (option == Build ? Constants::C_GOCOMPILERBUILDSTEP_ID : Constants::C_GOCOMPILERGETSTEP_ID))
    , m_goOption(option)
{
    switch (option) {
        case Get:
            setDefaultDisplayName(tr(Constants::C_GOCOMPILERGETSTEP_DISPLAY));
            setDisplayName(tr(Constants::C_GOCOMPILERGETSTEP_DISPLAY));
            break;
        case Build:
            setDefaultDisplayName(tr(Constants::C_GOCOMPILERBUILDSTEP_DISPLAY));
            setDisplayName(tr(Constants::C_GOCOMPILERBUILDSTEP_DISPLAY));
            break;
        case Clean:
            setDefaultDisplayName(tr(Constants::C_GOCOMPILERCLEANSTEP_DISPLAY));
            setDisplayName(tr(Constants::C_GOCOMPILERCLEANSTEP_DISPLAY));
            break;
    }

    GoBuildConfiguration *bc = qobject_cast<GoBuildConfiguration *>(buildConfiguration());
    connect(bc, &GoBuildConfiguration::buildDirectoryChanged,
            this, &BaseGoCompilerStep::updateProcessParameters);
    connect(bc, &BuildConfiguration::environmentChanged,
            this, &BaseGoCompilerStep::updateEnvironment);
    if (m_goOption == Build)
        connect(this, &BaseGoCompilerStep::outFilePathChanged,
                bc, &GoBuildConfiguration::outFilePathChanged);
    updateProcessParameters();
}

ProjectExplorer::BuildStepConfigWidget *BaseGoCompilerStep::createConfigWidget()
{
    return new GoCompilerBuildStepConfigWidget(this);
}

bool BaseGoCompilerStep::init()
{
    setOutputParser(new GoOutputParser());
    if (IOutputParser *parser = target()->kit()->createOutputParser())
        appendOutputParser(parser);
    outputParser()->setWorkingDirectory(processParameters()->workingDirectory());
    return AbstractProcessStep::init();
}

void BaseGoCompilerStep::doRun()
{
    ProcessParameters *parameters = processParameters();
    Environment env = parameters->environment();

    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GROUP));
    QString goPath = settings->value(QLatin1String(Constants::C_GOLANG_GENERAL_SETTINGS_GOPATH_KEY)).toString();
    settings->endGroup();

    env.set(QStringLiteral("GOPATH"), goPath);
    parameters->setEnvironment(env);
    AbstractProcessStep::run();
}

bool BaseGoCompilerStep::fromMap(const QVariantMap &map)
{
    AbstractProcessStep::fromMap(map);
    switch (m_goOption) {
        case Get:
            m_userCompilerOptions = map[Constants::C_GOCOMPILERGETSTEP_USERCOMPILEROPTIONS].toString().split(QLatin1Char('|'));
            break;
        case Build:
            m_userCompilerOptions = map[Constants::C_GOCOMPILERBUILDSTEP_USERCOMPILEROPTIONS].toString().split(QLatin1Char('|'));
            break;
        case Clean:
            m_userCompilerOptions = map[Constants::C_GOCOMPILERCLEANSTEP_USERCOMPILEROPTIONS].toString().split(QLatin1Char('|'));
            break;
    }

    updateProcessParameters();
    return true;
}

QVariantMap BaseGoCompilerStep::toMap() const
{
    QVariantMap result = AbstractProcessStep::toMap();

    switch (m_goOption) {
        case Get:
            result[Constants::C_GOCOMPILERGETSTEP_USERCOMPILEROPTIONS] = m_userCompilerOptions.join(QLatin1Char('|'));
            break;
        case Build:
            result[Constants::C_GOCOMPILERBUILDSTEP_USERCOMPILEROPTIONS] = m_userCompilerOptions.join(QLatin1Char('|'));
            break;
        case Clean:
            result[Constants::C_GOCOMPILERCLEANSTEP_USERCOMPILEROPTIONS] = m_userCompilerOptions.join(QLatin1Char('|'));
            break;
    }

    return result;
}

QStringList BaseGoCompilerStep::userCompilerOptions() const
{
    return m_userCompilerOptions;
}

void BaseGoCompilerStep::setUserCompilerOptions(const QStringList &options)
{
    m_userCompilerOptions = options;
    emit userCompilerOptionsChanged(options);
    updateProcessParameters();
}

void BaseGoCompilerStep::updateProcessParameters()
{
    updateOutFilePath();
    updateCommand();
    updateArguments();
    updateWorkingDirectory();
    updateEnvironment();
    emit processParametersChanged();
}

void BaseGoCompilerStep::updateCommand()
{
    processParameters()->setCommand(QStringLiteral("go"));
}

void BaseGoCompilerStep::updateWorkingDirectory()
{
    auto bc = qobject_cast<GoBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);
    processParameters()->setWorkingDirectory(bc->buildDirectory().toString());
}

void BaseGoCompilerStep::updateArguments()
{
    auto bc = qobject_cast<GoBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);

    QStringList arguments;
    switch (m_goOption) {
        case Get:
            arguments << QStringLiteral("get");
            break;
        case Build:
            arguments << QStringLiteral("build");
            break;
        case Clean:
            arguments << QStringLiteral("clean");
            arguments << QStringLiteral("-i");
            break;
    }

    arguments << m_userCompilerOptions;

    // Remove empty args
    auto predicate = [](const QString &str) { return str.isEmpty(); };
    auto it = std::remove_if(arguments.begin(), arguments.end(), predicate);
    arguments.erase(it, arguments.end());

    processParameters()->setArguments(arguments.join(QChar::Space));
}

void BaseGoCompilerStep::updateEnvironment()
{
    auto bc = qobject_cast<GoBuildConfiguration *>(buildConfiguration());
    QTC_ASSERT(bc, return);
    processParameters()->setEnvironment(bc->environment());
}

Utils::FileName BaseGoCompilerStep::outFilePath() const
{
    return m_outFilePath;
}

void BaseGoCompilerStep::setOutFilePath(const Utils::FileName &outFilePath)
{
    if (outFilePath == m_outFilePath)
        return;
    m_outFilePath = outFilePath;
    emit outFilePathChanged(outFilePath);
}

BaseGoCompilerStep::GoOption BaseGoCompilerStep::goOption() const
{ return m_goOption; }

void BaseGoCompilerStep::updateOutFilePath()
{
    if (m_goOption == Build) {
        GoBuildConfiguration *bc = qobject_cast<GoBuildConfiguration *>(buildConfiguration());
        QTC_ASSERT(bc, return);
        QDir wd(bc->buildDirectory().toString());
        const QString targetName = Utils::HostOsInfo::withExecutableSuffix(wd.absolutePath() + '/' + wd.dirName());
        setOutFilePath(FileName::fromString(targetName));
    }
}

GoCompilerGetStep::GoCompilerGetStep(BuildStepList *parentList)
    : BaseGoCompilerStep(parentList, BaseGoCompilerStep::Get)
{ }

GoCompilerBuildStep::GoCompilerBuildStep(BuildStepList *parentList)
    : BaseGoCompilerStep(parentList, BaseGoCompilerStep::Build)
{ }

GoCompilerCleanStep::GoCompilerCleanStep(BuildStepList *parentList)
    : BaseGoCompilerStep(parentList, BaseGoCompilerStep::Clean)
{ }

}   // namespace GoLang

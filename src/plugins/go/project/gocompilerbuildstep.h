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

#include <projectexplorer/abstractprocessstep.h>
#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/buildstep.h>
#include <projectexplorer/buildsteplist.h>

namespace GoLang {

class BaseGoCompilerStep : public ProjectExplorer::AbstractProcessStep
{
    Q_OBJECT

public:
    enum GoOption {
        Get,
        Build,
        Clean
    };

    BaseGoCompilerStep(ProjectExplorer::BuildStepList *parentList, GoOption option);

    ProjectExplorer::BuildStepConfigWidget *createConfigWidget() override;
    bool init() override;
    void doRun() override;

    bool fromMap(const QVariantMap &map) override;
    QVariantMap toMap() const override;

    QStringList userCompilerOptions() const;
    void setUserCompilerOptions(const QStringList &options);

    GoOption goOption() const;

    Utils::FileName outFilePath() const;
    void setOutFilePath(const Utils::FileName &outFilePath);

signals:
    void userCompilerOptionsChanged(const QStringList &options);
    void processParametersChanged();
    void outFilePathChanged(const Utils::FileName &outFilePath);

private:
    void updateOutFilePath();
    void updateProcessParameters();
    void updateCommand();
    void updateWorkingDirectory();
    void updateArguments();
    void updateEnvironment();

    QStringList m_userCompilerOptions;
    GoOption m_goOption;

    Utils::FileName m_outFilePath;
};

class GoCompilerGetStep: public BaseGoCompilerStep
{
    Q_OBJECT

public:
    GoCompilerGetStep(ProjectExplorer::BuildStepList *parentList);
};

class GoCompilerBuildStep: public BaseGoCompilerStep
{
    Q_OBJECT

public:
    GoCompilerBuildStep(ProjectExplorer::BuildStepList *parentList);
};

class GoCompilerCleanStep: public BaseGoCompilerStep
{
    Q_OBJECT

public:
    GoCompilerCleanStep(ProjectExplorer::BuildStepList *parentList);
};

}   // namespace GoLang

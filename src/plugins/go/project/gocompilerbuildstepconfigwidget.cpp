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

#include "gocompilerbuildstepconfigwidget.h"
#include "ui_gocompilerbuildstepconfigwidget.h"
#include "gocompilerbuildstep.h"
#include "golangconstants.h"
#include "goproject.h"

#include <utils/qtcassert.h>

#include <QComboBox>
#include <QLineEdit>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

GoCompilerBuildStepConfigWidget::GoCompilerBuildStepConfigWidget(BaseGoCompilerStep *buildStep)
    : BuildStepConfigWidget()
    , m_buildStep(buildStep)
    , m_ui(new Ui::GoCompilerBuildStepConfigWidget())
{
    m_ui->setupUi(this);

    // Connect the project signals
    auto project = static_cast<GoProject *>(m_buildStep->project());
    connect(project, &GoProject::fileListChanged,
            this, &GoCompilerBuildStepConfigWidget::updateUi);

    // Connect build step signals
    connect(m_buildStep, &GoCompilerBuildStep::processParametersChanged,
            this, &GoCompilerBuildStepConfigWidget::updateUi);

    // Connect UI signals
    connect(m_ui->additionalArgumentsLineEdit, &QLineEdit::textEdited,
            this, &GoCompilerBuildStepConfigWidget::onAdditionalArgumentsTextEdited);

    updateUi();
}

GoCompilerBuildStepConfigWidget::~GoCompilerBuildStepConfigWidget()
{ }

QString GoCompilerBuildStepConfigWidget::summaryText() const
{
    QString result;

    switch (m_buildStep->goOption()) {
        case BaseGoCompilerStep::Get:
            result = Constants::C_GOCOMPILERGETSTEPWIDGET_SUMMARY;
            break;
        case BaseGoCompilerStep::Build:
            result = Constants::C_GOCOMPILERBUILDSTEPWIDGET_SUMMARY;
            break;
        case BaseGoCompilerStep::Clean:
            result = Constants::C_GOCOMPILERCLEANSTEPWIDGET_SUMMARY;
            break;
    }

    return result;
}

QString GoCompilerBuildStepConfigWidget::displayName() const
{
    QString result;

    switch (m_buildStep->goOption()) {
        case BaseGoCompilerStep::Get:
            result = Constants::C_GOCOMPILERGETSTEPWIDGET_DISPLAY;
            break;
        case BaseGoCompilerStep::Build:
            result = Constants::C_GOCOMPILERBUILDSTEPWIDGET_DISPLAY;
            break;
        case BaseGoCompilerStep::Clean:
            result = Constants::C_GOCOMPILERCLEANSTEPWIDGET_DISPLAY;
            break;
    }

    return result;
}

void GoCompilerBuildStepConfigWidget::updateUi()
{
    updateCommandLineText();
    updateAdditionalArgumentsLineEdit();
}

void GoCompilerBuildStepConfigWidget::updateCommandLineText()
{
    ProcessParameters *parameters = m_buildStep->processParameters();

    QStringList command;
    command << parameters->command();
    command << parameters->arguments();

    // Remove empty args
    auto predicate = [](const QString & str) { return str.isEmpty(); };
    auto it = std::remove_if(command.begin(), command.end(), predicate);
    command.erase(it, command.end());

    m_ui->commandTextEdit->setText(command.join(QChar::LineFeed));
}

void GoCompilerBuildStepConfigWidget::updateAdditionalArgumentsLineEdit()
{
    const QString text = m_buildStep->userCompilerOptions().join(QChar::Space);
    m_ui->additionalArgumentsLineEdit->setText(text);
}

void GoCompilerBuildStepConfigWidget::onAdditionalArgumentsTextEdited(const QString &text)
{
    m_buildStep->setUserCompilerOptions(text.split(QChar::Space));
}

}   // namespace GoLang

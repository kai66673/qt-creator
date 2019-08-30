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
#include "gotoolssettingspagewidget.h"
#include "gosettings.h"
#include "ui_gotoolssettingspagewidget.h"

namespace GoLang {

GoToolsSettingsPageWidget::GoToolsSettingsPageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GoToolsSettingsPageWidget)
{
    ui->setupUi(this);
    readSettings();
}

GoToolsSettingsPageWidget::~GoToolsSettingsPageWidget()
{
    delete ui;
}

void GoToolsSettingsPageWidget::readSettings()
{
    const GoToolsSettings &toolsSettings = GoSettings::toolsSettings();
    ui->goImportsCheckBox->setChecked(toolsSettings.goImportsEnabled());
    ui->goFmtCheckBox->setChecked(toolsSettings.goFmtEnabled());
}

void GoToolsSettingsPageWidget::writeSettings()
{
    GoToolsSettings toolsSettings;
    toolsSettings.setGoImportsEnabled(ui->goImportsCheckBox->isChecked());
    toolsSettings.setGoFmtEnabled(ui->goFmtCheckBox->isChecked());
    GoSettings::setToolsSettings(toolsSettings);
}

}   // namespace GoLang

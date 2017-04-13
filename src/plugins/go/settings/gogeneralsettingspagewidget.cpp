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

#include "gogeneralsettingspagewidget.h"
#include "gosettings.h"
#include "ui_gogeneralsettingspagewidget.h"

#include <QDir>
#include <QProcess>

namespace GoLang {

GoGeneralSettingsPageWidget::GoGeneralSettingsPageWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GoGeneralSettingsPageWidget)
{
    ui->setupUi(this);
    readSettings();

    ui->detectPushButton->setToolTip(QStringLiteral("Try auto detect environment by command \"go env\""));

    connect(ui->goRootChooser, &Utils::PathChooser::pathChanged,
            this, &GoGeneralSettingsPageWidget::goRootChanged);
    connect(ui->detectPushButton, &QPushButton::clicked,
            this,&GoGeneralSettingsPageWidget::detect);
}

GoGeneralSettingsPageWidget::~GoGeneralSettingsPageWidget()
{
    delete ui;
}

void GoGeneralSettingsPageWidget::readSettings()
{
    const GoGeneralSettings &generalSettings = GoSettings::generalSettings();
    ui->goPathChooser->setPath(generalSettings.goPath());
    ui->goRootChooser->setPath(generalSettings.goRoot());
    updateOsArchVariants(generalSettings.goRoot());
    selectOsArch(generalSettings.goOs(), generalSettings.goArch());
}

void GoGeneralSettingsPageWidget::writeSettings()
{
    GoGeneralSettings generalSetting;
    generalSetting.setGoPath(ui->goPathChooser->path());
    generalSetting.setGoRoot(ui->goRootChooser->path());
    if (ui->goOsArchComboBox->currentIndex() != -1) {
        QStringList parts = ui->goOsArchComboBox->currentText().split('/');
        if (parts.size() == 2) {
            generalSetting.setGoOs(parts[0]);
            generalSetting.setGoArch(parts[1]);
        }
    }

    GoSettings::setGeneralSettings(generalSetting);
}

void GoGeneralSettingsPageWidget::goRootChanged(const QString &goRoot)
{
    updateOsArchVariants(goRoot);
    ui->goOsArchComboBox->setCurrentIndex(-1);
    if (ui->goOsArchComboBox->count() == 1)
        ui->goOsArchComboBox->setCurrentIndex(0);
}

void GoGeneralSettingsPageWidget::detect()
{
    ui->detectStateLabel->setText("");
    QProcess goEnvProcess;

    goEnvProcess.start(QStringLiteral("go"), QStringList() << QStringLiteral("env"));
    if (!goEnvProcess.waitForFinished(500)) {
        switch (goEnvProcess.error()) {
            case QProcess::FailedToStart:
                reportError(QStringLiteral("Start process \"go env\" filed. Go isnt installed?"));
                return;
            case QProcess::Crashed:
                reportError(QStringLiteral("Start process \"go env\" crashed"));
                return;
            case QProcess::Timedout:
                reportError(QStringLiteral("Process \"go env\" execution timed out"));
                return;
            default:
                reportError(QStringLiteral("Unknown error during execution process \"go env\""));
                return;
        }
    }

    if (goEnvProcess.exitCode() != 0) {
        reportError(QString("Process \"go env\" terminated with code %1 (%2)")
                    .arg(goEnvProcess.exitCode())
                    .arg(QString::fromUtf8(goEnvProcess.readAllStandardError()).trimmed()));
        return;
    }

    QByteArray response = goEnvProcess.readAllStandardOutput();
    goEnvProcess.closeReadChannel(QProcess::StandardOutput);

    QByteArrayList goSettings = response.split('\n');
    QString goPath = "";
    QString goRoot = "";
    QString goOs = "";
    QString goArch = "";
    foreach (const QByteArray &s, goSettings) {
        QByteArray ba = s;

        // for Windows-host crop "set "-prefix
        if (s.startsWith("set "))
            ba = s.right(s.length() - 4);

        QByteArrayList l = ba.split('=');
        if (l.size() == 2) {
            if (l[0] == "GOPATH") {
                if (l[1].startsWith("\"") && l[1].endsWith("\""))
                    goPath = QString::fromUtf8(l[1].mid(1, l[1].length() - 2));
                else
                    goPath = QString::fromUtf8(l[1]);
                continue;
            }
            if (l[0] == "GOROOT") {
                if (l[1].startsWith("\"") && l[1].endsWith("\""))
                    goRoot = QString::fromUtf8(l[1].mid(1, l[1].length() - 2));
                else
                    goRoot = QString::fromUtf8(l[1]);
                continue;
            }
            if (l[0] == "GOOS") {
                if (l[1].startsWith("\"") && l[1].endsWith("\""))
                    goOs = QString::fromUtf8(l[1].mid(1, l[1].length() - 2));
                else
                    goOs = QString::fromUtf8(l[1]);
                continue;
            }
            if (l[0] == "GOARCH") {
                if (l[1].startsWith("\"") && l[1].endsWith("\""))
                    goArch = QString::fromUtf8(l[1].mid(1, l[1].length() - 2));
                else
                    goArch = QString::fromUtf8(l[1]);
                continue;
            }
        }
    }
    ui->goPathChooser->setPath(goPath);
    ui->goRootChooser->setPath(goRoot);
    updateOsArchVariants(goRoot);
    selectOsArch(goOs, goArch);
}

void GoGeneralSettingsPageWidget::updateOsArchVariants(const QString &goRoot)
{
    ui->goOsArchComboBox->clear();

    QDir dir(goRoot);
    if (dir.exists()) {
        if (dir.cd(QStringLiteral("pkg"))) {
            foreach (const QString &entry, dir.entryList(QDir::Dirs)) {
                QStringList parts = entry.split('_');
                if (parts.size() == 2)
                    ui->goOsArchComboBox->addItem(parts[0] + QStringLiteral("/") + parts[1]);
            }
        }
    }
}

void GoGeneralSettingsPageWidget::selectOsArch(const QString &goOs, const QString &goArch)
{
    ui->goOsArchComboBox->setCurrentIndex(-1);

    for (int i = 0; i < ui->goOsArchComboBox->count(); i++) {
        QStringList parts = ui->goOsArchComboBox->itemText(i).split('/');
        if (parts.size() == 2 && parts[0] == goOs && parts[1] == goArch) {
            ui->goOsArchComboBox->setCurrentIndex(i);
            break;
        }
    }
}

void GoGeneralSettingsPageWidget::reportError(const QString &message)
{ ui->detectStateLabel->setText(message); }

}   // namespace GoLang

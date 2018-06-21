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

#include <projectexplorer/runconfiguration.h>

namespace ProjectExplorer {
class WorkingDirectoryAspect;
class ArgumentsAspect;
class TerminalAspect;
class LocalEnvironmentAspect;
}

namespace GoLang {

class GoBuildConfiguration;

class GoRunConfiguration : public ProjectExplorer::RunConfiguration
{
    Q_OBJECT

public:
    GoRunConfiguration(ProjectExplorer::Target *parent, Core::Id id);

    QWidget *createConfigurationWidget() override;
    QVariantMap toMap() const override;
    bool fromMap(const QVariantMap &map) override;

signals:
    void executableChanged(const QString &args);

private:
    void updateConfiguration();
    void setActiveBuildConfiguration(GoBuildConfiguration *activeBuildConfiguration);

    QString m_executable;
    GoBuildConfiguration *m_buildConfiguration;
};

}   // namespace GoLang

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

#include "gobuildconfigurationwidget.h"
#include "gobuildconfiguration.h"
#include "golangconstants.h"

#include <utils/pathchooser.h>
#include <utils/detailswidget.h>

#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

GoBuildConfigurationWidget::GoBuildConfigurationWidget(GoBuildConfiguration *bc)
    : NamedWidget(nullptr)
    , m_bc(bc)
{
    // Build UI
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    auto detailsWidget = new DetailsWidget();
    detailsWidget->setState(DetailsWidget::NoSummary);
    mainLayout->addWidget(detailsWidget);

    auto detailsInnerWidget = new QWidget();
    auto formLayout = new QFormLayout(detailsInnerWidget);
    detailsWidget->setWidget(detailsInnerWidget);

    m_buildDirectoryChooser = new PathChooser();
    formLayout->addRow(tr("Build directory:"), m_buildDirectoryChooser);

    // Connect signals
    connect(bc, &GoBuildConfiguration::buildDirectoryChanged,
            this, &GoBuildConfigurationWidget::updateUi);
    connect(m_buildDirectoryChooser, &PathChooser::pathChanged,
            this, &GoBuildConfigurationWidget::onPathEdited);

    setDisplayName(tr(Constants::C_GOBUILDCONFIGURATIONWIDGET_DISPLAY));
    updateUi();
}

GoBuildConfigurationWidget::~GoBuildConfigurationWidget()
{ }

void GoBuildConfigurationWidget::updateUi()
{
    m_buildDirectoryChooser->setPath(m_bc->buildDirectory().toUserOutput());
}

void GoBuildConfigurationWidget::onPathEdited(const QString &path)
{
     m_bc->setBuildDirectory(FileName::fromUserInput(path));
}

}   // namespace GoLang

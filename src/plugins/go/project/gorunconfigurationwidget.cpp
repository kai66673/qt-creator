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

#include "gorunconfigurationwidget.h"
#include "gorunconfiguration.h"

#include <projectexplorer/runconfigurationaspects.h>

#include <QFormLayout>

using namespace ProjectExplorer;

namespace GoLang {

GoRunConfigurationWidget::GoRunConfigurationWidget(GoRunConfiguration *rc, QWidget *parent)
    : QWidget(parent)
    , m_rc(rc)
{
    QTC_ASSERT(rc, return);
    QFormLayout *fl = new QFormLayout(this);
    fl->setMargin(0);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    rc->extraAspect<ArgumentsAspect>()->addToConfigurationLayout(fl);
    rc->extraAspect<TerminalAspect>()->addToConfigurationLayout(fl);
    connect(rc, &GoRunConfiguration::enabledChanged, this, &GoRunConfigurationWidget::updateUi);
    updateUi();
}

void GoRunConfigurationWidget::updateUi()
{
    setEnabled(m_rc->isEnabled());
}

}   // namespace GoLang

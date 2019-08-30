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

#include "gogeneralsettingspage.h"
#include "golangconstants.h"

#include <QWidget>

namespace GoLang {

GoGeneralSettingsPage::GoGeneralSettingsPage(QWidget *parent)
    : Core::IOptionsPage(parent)
    , m_widget(nullptr)
{
    setId(Constants::C_GOLANGGENERALSETTINGSPAGE_ID);
    setDisplayName(tr(Constants::C_GOLANGGENERAKSETTINGSPAGE_DISPLAY));
    setCategory(Constants::C_GOLANGSETTINGSPAGE_CATEGORY);
    setDisplayCategory(tr(Constants::C_GOLANGSETTINGSPAGE_CATEGORY_DISPLAY));
    setCategoryIcon(Utils::Icon(Constants::C_GOLANG_ICON_PATH));
}

GoGeneralSettingsPage::~GoGeneralSettingsPage()
{
    deleteWidget();
}

QWidget *GoGeneralSettingsPage::widget()
{
    if (!m_widget)
        m_widget = new GoGeneralSettingsPageWidget;
    return m_widget;
}

void GoGeneralSettingsPage::apply()
{
    if (m_widget)
        m_widget->writeSettings();
}

void GoGeneralSettingsPage::finish()
{
    deleteWidget();
}

void GoGeneralSettingsPage::deleteWidget()
{
    if (m_widget) {
        delete m_widget;
        m_widget = nullptr;
    }
}

}   // namespace GoLang

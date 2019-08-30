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

#include <projectexplorer/toolchainconfigwidget.h>
#include <projectexplorer/abiwidget.h>
#include <utils/pathchooser.h>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QFormLayout;
class QGridLayout;
class QLineEdit;
class QLabel;
QT_END_NAMESPACE

namespace GoLang {

class GoToolChain;
// --------------------------------------------------------------------------
// ToolChainConfigWidget
// --------------------------------------------------------------------------

class GoToolChainConfigWidget : public ProjectExplorer::ToolChainConfigWidget
{
    Q_OBJECT

public:
    GoToolChainConfigWidget(GoToolChain *);

protected:
    void applyImpl() override;
    void discardImpl() override;
    bool isDirtyImpl() const override;
    void makeReadOnlyImpl() override;

private:
    void setFromToolchain();
    void handleCompilerCommandChange();

    Utils::PathChooser *m_compilerCommand;
    ProjectExplorer::AbiWidget *m_abiWidget;
    QLabel *m_goVersion;

    GoToolChain *m_toolChain;
    bool m_isReadOnly = false;
};

} // namespace GoLang

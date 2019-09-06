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

#include "gotoolchainconfigwidget.h"
#include "gotoolchain.h"

#include <utils/qtcassert.h>
#include <utils/fileutils.h>
#include <utils/environment.h>

#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFileInfo>
#include <QDir>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang {

GoToolChainConfigWidget::GoToolChainConfigWidget(GoToolChain *tc)
    : ToolChainConfigWidget(tc)
    , m_compilerCommand(new PathChooser)
    , m_abiWidget(new AbiWidget)
    , m_goVersion(nullptr)
    , m_toolChain(tc)
{
    QTC_CHECK(tc);

    const QStringList versionArgs = QStringList(QLatin1String("version"));
    m_compilerCommand->setExpectedKind(PathChooser::ExistingCommand);
    m_compilerCommand->setCommandVersionArguments(versionArgs);
    m_compilerCommand->setHistoryCompleter(QLatin1String("PE.Go.Command.History"));
    m_mainLayout->addRow(tr("Compiler:"), m_compilerCommand);

    m_mainLayout->addRow(tr("&ABI:"), m_abiWidget);
    m_abiWidget->setEnabled(false);

    m_goVersion = new QLabel(this);
    m_mainLayout->addRow(tr("Version"), m_goVersion);

    addErrorLabel();

    setFromToolchain();

    connect(m_compilerCommand, &PathChooser::rawPathChanged,
            this, &GoToolChainConfigWidget::handleCompilerCommandChange);
    connect(m_abiWidget, &AbiWidget::abiChanged, this, &ToolChainConfigWidget::dirty);
}

void GoToolChainConfigWidget::applyImpl()
{
    QFileInfo goFileInfo(m_compilerCommand->fileName().toString());
    QDir goRoot(goFileInfo.absoluteDir());
    goRoot.cd("..");    // remove bin from path
    m_toolChain->setCompilerCommand(m_compilerCommand->fileName(),
                                    FilePath::fromString(goRoot.absolutePath()));
    m_toolChain->setSupportedAbis(m_abiWidget->supportedAbis());
    m_toolChain->setTargetAbi(m_abiWidget->currentAbi());
    m_toolChain->setOriginalTargetTriple(m_toolChain->detectSupportedAbis().originalTargetTriple);
}

void GoToolChainConfigWidget::discardImpl()
{
    setFromToolchain();
}

bool GoToolChainConfigWidget::isDirtyImpl() const
{
    return m_compilerCommand->fileName() != m_toolChain->compilerCommand() ||
           m_abiWidget->currentAbi() != m_toolChain->targetAbi();
}

void GoToolChainConfigWidget::makeReadOnlyImpl()
{
    m_compilerCommand->setReadOnly(true);
    m_abiWidget->setEnabled(false);
    m_isReadOnly = true;
}

void GoToolChainConfigWidget::setFromToolchain()
{
    bool blocked = blockSignals(true);

    m_compilerCommand->setFileName(m_toolChain->compilerCommand());
    m_goVersion->setText(m_toolChain->version());
    m_abiWidget->setAbis(m_toolChain->supportedAbis(), m_toolChain->targetAbi());
    if (!m_isReadOnly && !m_compilerCommand->path().isEmpty())
        m_abiWidget->setEnabled(true);

    blockSignals(blocked);
}

void GoToolChainConfigWidget::handleCompilerCommandChange()
{
    bool haveCompiler = false;
    Abi currentAbi = m_abiWidget->currentAbi();
    bool customAbi = m_abiWidget->isCustomAbi();
    FileName path = m_compilerCommand->fileName();
    QVector<Abi> abiList;

    if (!path.isEmpty()) {
        QFileInfo fi(path.toFileInfo());
        haveCompiler = fi.isExecutable() && fi.isFile();
    }
    if (haveCompiler) {
        Environment env = Environment::systemEnvironment();
        GoToolChain::addCommandPathToEnvironment(path, env);
        QFileInfo goFileInfo(m_compilerCommand->fileName().toString());
        QDir goRoot(goFileInfo.absoluteDir());
        goRoot.cd("..");    // remove bin from path
        abiList = GoToolChain::detectGoAbi(goRoot.absolutePath());

        bool blocked = blockSignals(true);
        m_goVersion->setText(GoToolChain::detectVersionForPath(path));
        blockSignals(blocked);
    }
    m_abiWidget->setEnabled(haveCompiler);

    // Find a good ABI for the new compiler:
    Abi newAbi;
    if (customAbi)
        newAbi = currentAbi;
    else if (abiList.contains(currentAbi))
        newAbi = currentAbi;

    m_abiWidget->setAbis(abiList, newAbi);
    emit dirty();
}

} // namespace GoLang

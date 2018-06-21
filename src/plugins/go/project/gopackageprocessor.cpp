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

#include "gopackageprocessor.h"
#include "goproject.h"
#include "gotoolchain.h"
#include "golangconstants.h"

#include <projectexplorer/kitinformation.h>
#include <projectexplorer/target.h>

namespace GoLang {

GoPackageProcessor::GoPackageProcessor(GoProject *project)
    : QObject(project)
    , m_project(project)
    , m_toolChain(nullptr)
{ }

QList<QString> GoPackageProcessor::packagesForSuffix(const QString &suffix)
{
    checkToolChain();

    QList<QString> result;

    QString suffixWithSkash(suffix);
    suffixWithSkash.prepend(QLatin1Char('/'));

    foreach (const QString &package, m_packages) {
        QString tmp(package);
        tmp.prepend(QLatin1Char('/'));
        if (tmp.endsWith(suffixWithSkash))
            result << package;
    }

    return result;
}

QList<QString> GoPackageProcessor::packages()
{
    checkToolChain();
    return m_packages;
}

void GoPackageProcessor::checkToolChain()
{
    bool needUpdatePackages = false;
    GoToolChain *toolChain = nullptr;
    if (ProjectExplorer::Target *activeTarget = m_project->activeTarget())
        if (ProjectExplorer::Kit *kit = activeTarget->kit())
            toolChain = static_cast<GoToolChain *>(ProjectExplorer::ToolChainKitInformation::toolChain(kit, Constants::C_GOLANGUAGE_ID));

    if (m_toolChain != toolChain) {
        m_toolChain = toolChain;
        if (!toolChain) {
            m_goRoot.clear();
            cleanPackages();
            return;
        }
        m_goRoot = toolChain->goRoot();
        needUpdatePackages = true;
    } else if (toolChain && toolChain->goRoot() != m_goRoot) {
        needUpdatePackages = true;
        m_goRoot = toolChain->goRoot();
    }

    if (needUpdatePackages)
        updatePackages();
}

void GoPackageProcessor::cleanPackages()
{
    m_packages.clear();
}

void GoPackageProcessor::updatePackages()
{
    QString packagesPath(m_goRoot.toString());
    packagesPath.append(QLatin1String("/pkg/"));

    ProjectExplorer::Abi abi = m_toolChain->targetAbi();

    switch (abi.os()) {
        case ProjectExplorer::Abi::BsdOS:
        case ProjectExplorer::Abi::LinuxOS:
        case ProjectExplorer::Abi::DarwinOS:
        case ProjectExplorer::Abi::UnixOS:
            packagesPath.append(QStringLiteral("linux_"));
            break;
        case ProjectExplorer::Abi::WindowsOS:
            packagesPath.append(QStringLiteral("windows_"));
            break;
        case ProjectExplorer::Abi::VxWorks:
        case ProjectExplorer::Abi::QnxOS:
        case ProjectExplorer::Abi::BareMetalOS:
        case ProjectExplorer::Abi::UnknownOS:
            cleanPackages();
            return;
    }

    switch (abi.architecture()) {
        case ProjectExplorer::Abi::ArmArchitecture:
            packagesPath.append(QStringLiteral("arm"));
            break;
        case ProjectExplorer::Abi::X86Architecture:{
            if(abi.wordWidth() == 64)
                packagesPath.append(QStringLiteral("amd64"));
            else
                packagesPath.append(QStringLiteral("386"));
            break;
        }
        case ProjectExplorer::Abi::UnknownArchitecture: // fall through!
        default:
            cleanPackages();
            return;
    }

    QDir pckDir(packagesPath);
    if (!pckDir.exists()) {
        cleanPackages();
        return;
    }

    cleanPackages();

    recursiveScanDirectory(pckDir, QStringLiteral(""));
}

void GoPackageProcessor::recursiveScanDirectory(const QDir &dir, const QString &prefix)
{
    for (const QFileInfo &info : dir.entryInfoList(QDir::AllDirs |
                                                   QDir::Files |
                                                   QDir::NoDotAndDotDot |
                                                   QDir::NoSymLinks |
                                                   QDir::CaseSensitive)) {
        if (info.isDir())
            recursiveScanDirectory(QDir(info.filePath()), prefix + info.fileName() + QLatin1Char('/'));
        else if (info.suffix() == QStringLiteral("a"))
            m_packages << prefix + info.completeBaseName();
    }
}

}   // namespace GoLang

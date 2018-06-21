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

#include "goproject.h"
#include "golangconstants.h"
#include "goprojectnode.h"
#include "gopackageprocessor.h"
#include "gotoolchain.h"
#include "gopackage.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <coreplugin/vcsmanager.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/toolchain.h>
#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/qtcassert.h>
#include <utils/runextensions.h>

#include <QQueue>

namespace GoLang {

const int MIN_TIME_BETWEEN_PROJECT_SCANS = 4500;

GoProject::GoProject(const Utils::FileName &fileName)
    : ProjectExplorer::Project(Constants::C_GO_PROJECT_MIMETYPE, fileName)
    , m_packageProcessor(new GoPackageProcessor(this))
{
    setId(Constants::C_GOPROJECT_ID);
    setDisplayName(fileName.toFileInfo().completeBaseName());

    m_projectScanTimer.setSingleShot(true);
    connect(&m_projectScanTimer, &QTimer::timeout, this, &GoProject::collectProjectFiles);
    connect(&m_futureWatcher, &QFutureWatcher<QList<ProjectExplorer::FileNode *>>::finished,
            this, &GoProject::updateProject);

    connect(GoTools::GoPackageCache::instance(), &GoTools::GoPackageCache::packageCacheCleaned,
            this, &GoProject::reindexProjectFiles);

    collectProjectFiles();
}

bool GoProject::needsConfiguration() const
{
    return targets().empty();
}

Utils::FileNameList GoProject::goFiles() const
{
    return files([](const ProjectExplorer::Node *fn) {
        return AllFiles(fn) && fn->filePath().endsWith(".go");
    });
}

QList<QString> GoProject::packagesForSuffix(const QString &suffix)
{ return m_packageProcessor->packagesForSuffix(suffix); }

QList<QString> GoProject::packages()
{ return m_packageProcessor->packages(); }

QVariantMap GoProject::toMap() const
{
    QVariantMap result = Project::toMap();
    result[Constants::C_GOPROJECT_EXCLUDEDFILES] = m_excludedFiles;
    return result;
}

ProjectExplorer::Project::RestoreResult GoProject::fromMap(const QVariantMap &map, QString *errorMessage)
{
    m_excludedFiles = map.value(Constants::C_GOPROJECT_EXCLUDEDFILES).toStringList();
    return Project::fromMap(map, errorMessage);
}

bool GoProject::addFiles(const QStringList &filePaths)
{
    m_excludedFiles = Utils::filtered(m_excludedFiles, [&](const QString &f) { return !filePaths.contains(f); });
    scheduleProjectScan();
    return true;
}

bool GoProject::removeFiles(const QStringList &filePaths)
{
    m_excludedFiles.append(filePaths);
    m_excludedFiles = Utils::filteredUnique(m_excludedFiles);
    scheduleProjectScan();
    return true;
}

bool GoProject::renameFile(const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(filePath)
    m_excludedFiles.removeOne(newFilePath);
    scheduleProjectScan();
    return true;
}

void GoProject::scheduleProjectScan()
{
    auto elapsedTime = m_lastProjectScan.elapsed();
    if (elapsedTime < MIN_TIME_BETWEEN_PROJECT_SCANS) {
        if (!m_projectScanTimer.isActive()) {
            m_projectScanTimer.setInterval(MIN_TIME_BETWEEN_PROJECT_SCANS - elapsedTime);
            m_projectScanTimer.start();
        }
    } else {
        collectProjectFiles();
    }
}

void GoProject::collectProjectFiles()
{
    m_lastProjectScan.start();
    QTC_ASSERT(!m_futureWatcher.future().isRunning(), return);
    Utils::FileName prjDir = projectDirectory();
    const QList<Core::IVersionControl *> versionControls = Core::VcsManager::versionControls();
    QFuture<QList<ProjectExplorer::FileNode *>> future = Utils::runAsync([prjDir, versionControls] {
        return ProjectExplorer::FileNode::scanForFilesWithVersionControls(prjDir, [](const Utils::FileName &fn) {
            return new ProjectExplorer::FileNode(fn, ProjectExplorer::FileType::Source, false);
        }, versionControls);
    });

    m_futureWatcher.setFuture(future);
    Core::ProgressManager::addTask(future, tr("Scanning for Go files"), "Go.Project.Scan");
}

void GoProject::updateProject()
{
    emitParsingStarted();
    const QStringList oldFiles = m_files;
    m_files.clear();

    std::vector<std::unique_ptr<ProjectExplorer::FileNode>> fileNodes
            = Utils::transform<std::vector>(m_futureWatcher.future().result(),
                                            [](ProjectExplorer::FileNode *fn) {
            return std::unique_ptr<ProjectExplorer::FileNode>(fn);
    });
    std::remove_if(std::begin(fileNodes), std::end(fileNodes),
                   [this](const std::unique_ptr<ProjectExplorer::FileNode> &fn) {
        const Utils::FileName path = fn->filePath();
        const QString fileName = path.fileName();
        return m_excludedFiles.contains(path.toString())
                || fileName.endsWith(".goproject", Utils::HostOsInfo::fileNameCaseSensitivity())
                || fileName.contains(".goproject.user", Utils::HostOsInfo::fileNameCaseSensitivity());
    });

    m_files = Utils::transform<QList>(fileNodes, [](const std::unique_ptr<ProjectExplorer::FileNode> &fn) {
        return fn->filePath().toString();
    });
    Utils::sort(m_files, [](const QString &a, const QString &b) { return a < b; });

    if (oldFiles == m_files)
        return;

    auto newRoot = std::make_unique<GoProjectNode>(*this, projectDirectory());
    newRoot->setDisplayName(displayName());
    newRoot->addNestedNodes(std::move(fileNodes));
    setRootProjectNode(std::move(newRoot));

    emitParsingFinished(true);

    QSet<QString> oldFilesSet = oldFiles.toSet();
    QSet<QString> newFilesSet = m_files.toSet();
    GoTools::GoPackageCache::instance()->indexGoFiles(displayName(), oldFilesSet - newFilesSet, newFilesSet - oldFilesSet);
}

void GoProject::reindexProjectFiles()
{
    GoTools::GoPackageCache::instance()->indexGoFiles(displayName(), QSet<QString>(), m_files.toSet());
}

}   // GoLang

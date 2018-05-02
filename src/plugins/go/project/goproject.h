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

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>

#include <QFileSystemWatcher>
#include <QElapsedTimer>
#include <QFutureWatcher>
#include <QTimer>

namespace TextEditor { class TextDocument; }
namespace ProjectExplorer { class Kit; }

namespace GoLang {

class GoProjectManager;
class GoProjectNode;
class GoPackageProcessor;

class GoProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    GoProject(const Utils::FileName &fileName);

    bool needsConfiguration() const override;
    bool supportsKit(const ProjectExplorer::Kit *k, QString *errorMessage) const override;
    Utils::FileNameList goFiles() const;

    QList<QString> packagesForSuffix(const QString &suffix);
    QList<QString> packages();

    QVariantMap toMap() const override;

    bool addFiles(const QStringList &filePaths);
    bool removeFiles(const QStringList &filePaths);
    bool renameFile(const QString &filePath, const QString &newFilePath);

protected:
    RestoreResult fromMap(const QVariantMap &map, QString *errorMessage) override;

private:
    void scheduleProjectScan();
    void collectProjectFiles();
    void updateProject();
    void reindexProjectFiles();

    GoPackageProcessor *m_packageProcessor;

    QStringList m_files;
    QStringList m_excludedFiles;
    QFutureWatcher<QList<ProjectExplorer::FileNode *>> m_futureWatcher;
    QElapsedTimer m_lastProjectScan;
    QTimer m_projectScanTimer;
};

}   // GoLang

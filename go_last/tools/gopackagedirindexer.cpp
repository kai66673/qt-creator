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
#include "gopackagedirindexer.h"
#include "goconstants.h"
#include "gocodemodelmanager.h"
#include "gosettings.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/runextensions.h>

#include <QDir>

namespace GoTools {

static void recursiveScanDirectory(const QDir &dir, const QString &prefix,
                                   GoPackageDirIndexer::PackageDirs &result)
{
    for (const QFileInfo &info : dir.entryInfoList(QDir::AllDirs |
                                                   QDir::Files |
                                                   QDir::NoDotAndDotDot |
                                                   QDir::NoSymLinks |
                                                   QDir::CaseSensitive)) {
        if (info.isDir())
            recursiveScanDirectory(QDir(info.filePath()), prefix + QLatin1Char('/') + info.fileName(), result);
        else if (info.suffix() == QStringLiteral("a"))
            result.append({prefix, info.completeBaseName()});
    }
}

static void indexPackageDirs(QFutureInterface<GoPackageDirIndexer::PackageDirs> &future,
                             const QString &goRoot,
                             const QString &goOs,
                             const QString &goArch)
{
    GoPackageDirIndexer::PackageDirs result;

    QString packagesPath = goRoot + QLatin1String("/pkg/") + goOs + QLatin1String("_") + goArch;

    QDir pkgDir(packagesPath);
    if (!pkgDir.exists()) {
        future.reportFinished(&result);
        return;
    }

    QFileInfoList dirs = pkgDir.entryInfoList(QDir::AllDirs |
                                              QDir::NoDotAndDotDot |
                                              QDir::NoSymLinks |
                                              QDir::CaseSensitive);
    future.setProgressRange(0, dirs.size() + 1);
    future.setProgressValue(0);

    int progressValue = 0;

    for (const QFileInfo &file: pkgDir.entryInfoList(QDir::Files |
                                                     QDir::NoDotAndDotDot |
                                                     QDir::NoSymLinks |
                                                     QDir::CaseSensitive)) {
        if (file.suffix() == QStringLiteral("a"))
            result.append({QString(), file.completeBaseName()});
    }
    future.setProgressValue(++progressValue);

    for (const QFileInfo &dir: dirs) {
        recursiveScanDirectory(QDir(dir.filePath()), dir.fileName(), result);
        future.setProgressValue(++progressValue);
    }

    future.reportFinished(&result);
}

GoPackageDirIndexer::GoPackageDirIndexer()
{ }

GoPackageDirIndexer::~GoPackageDirIndexer()
{
    if (m_watcher) {
        disconnectWatcher();
        m_watcher->cancel();
        m_watcher->waitForFinished();
    }
}

void GoPackageDirIndexer::reindex()
{
    m_packages.clear();

    if (m_watcher) {
        disconnectWatcher();
        m_watcher->cancel();
    }

    m_watcher.reset(new QFutureWatcher<PackageDirs>());
    connectWatcher();

    const GoLang::GoGeneralSettings &settings = GoLang::GoSettings::generalSettings();
    QFuture<PackageDirs> future = Utils::runAsync(GoCodeModelManager::instance()->sharedThreadPool(),
                                                  indexPackageDirs,
                                                  settings.goRoot(),
                                                  settings.goOs(),
                                                  settings.goArch());
    m_watcher->setFuture(future);

    Core::ProgressManager::addTask(future, QString("Indexing Go package dirs"),
                                   Go::Constants::GO_INDEX_TASK);
}

void GoPackageDirIndexer::connectWatcher()
{
    connect(m_watcher.data(), &Watcher::finished,
            this, &GoPackageDirIndexer::onIndexingFinished);
}

void GoPackageDirIndexer::disconnectWatcher()
{
    disconnect(m_watcher.data(), &Watcher::finished,
               this, &GoPackageDirIndexer::onIndexingFinished);
}

void GoPackageDirIndexer::onIndexingFinished()
{
    QMutexLocker lock(&m_mutex);
    m_packages = m_watcher->future().result();
}

GoPackageDirIndexer::PackageDirs GoPackageDirIndexer::packages() const
{
    QMutexLocker lock(&m_mutex);
    return m_packages;
}

}   // namespace GoTools

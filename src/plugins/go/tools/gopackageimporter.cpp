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
#include "gopackageimporter.h"
#include "gopackage.h"
#include "gocodemodelmanager.h"
#include "goconstants.h"
#include "goeditordocumenthandle.h"
#include "gosettings.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/runextensions.h>

#include <QDir>

namespace GoTools {

QStringList C_GO_FILES_FILTER({QStringLiteral("*.go")});
QString C_GO_TEST_FILE(QStringLiteral("_test.go"));

struct SourceWithRevision
{
    SourceWithRevision(GoEditorDocumentHandle *docHandle)
        : fileName(docHandle->filePath())
        , source(docHandle->contents())
        , revision(docHandle->revision())
    { }

    QString fileName;
    QByteArray source;
    unsigned revision;
};

void parsePackageFiles(QFutureInterface<GoPackage *> &future, const QString &location,
                       const QStringList &notOpenedGoFiles,
                       const QList<SourceWithRevision> &openedGoDocs,
                       GoSource::Ptr doc,
                       const QString &packageName,
                       const QString &goRoot, const QString &goPath)
{
    future.setProgressRange(0, notOpenedGoFiles.size() + openedGoDocs.size() + (doc ? 1 : 0));

    int progressValue = 0;
    GoPackage *pkg = new GoPackage(location, packageName);

    if (doc) {
        pkg->insertGoSource(doc->fileName(), doc);
        future.setProgressValue(++progressValue);
    }

    // proceed opened files
    for (const SourceWithRevision &swr: openedGoDocs) {
        GoSource::Ptr doc = GoSource::create(swr.fileName);
        doc->setSource(swr.source);
        doc->setRevision(swr.revision);
        if (doc->parsePackageFile(packageName)) {
            doc->resolveImportsAndPackageName(goRoot, goPath);
            pkg->insertGoSource(swr.fileName, doc);
        }
        future.setProgressValue(++progressValue);
    }

    // proceed not opened files
    for (const QString &goFile: notOpenedGoFiles) {
        QFile file(goFile);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray source = file.readAll();
            for (int i = 0; i < source.size(); i++) {
                if (source[i] == '\r') {
                    if (source[i+1] == '\n')
                        source[i++] = ' ';
                    else
                        source[i] = '\n';
                }
            }
            GoSource::Ptr doc = GoSource::create(goFile);
            doc->setSource(source);
            if (doc->parsePackageFile(packageName)) {
                doc->resolveImportsAndPackageName(goRoot, goPath);
                pkg->insertGoSource(goFile, doc);
            }
        }
        future.setProgressValue(++progressValue);
    }

    future.reportFinished(&pkg);
}

GoPackageImporter::GoPackageImporter(GoPackageCache *cache, QObject *parent)
    : QObject(parent)
    , m_cache(cache)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &GoPackageImporter::proceedTasks);
}

void GoPackageImporter::importPackages(const GoPackageKeySet &imports)
{
    for (const GoPackageKey &task: imports) {
        int before = m_allTasks.size();
        QSet<GoPackageKey>::iterator it = m_allTasks.insert(task);
        int after = m_allTasks.size();
        if (after != before)
            m_tasksToProceed.append(*it);
    }

    m_timer->start(10);
}

void GoPackageImporter::importDocPackage(GoSource::Ptr doc)
{
    GoPackageKey docKey{doc->location(), doc->packageName()};
    m_tasksToProceed.prepend(docKey);
    m_directlyUpdatingDocs.insert(doc->fileName(), doc);

    m_timer->start();
}

void GoPackageImporter::clean()
{
    m_timer->stop();
    m_allTasks.clear();
    m_tasksToProceed.clear();
    m_directlyUpdatingDocs.clear();


    while (true) {
        QList<QFuture<GoPackage *> > futures = m_synchronizer.futures();
        m_synchronizer.clearFutures();

        for (const QFuture<GoPackage *> &future: futures) {
            if (future.isFinished()) {
                if (GoPackage *pkg = future.result()) {
                    delete pkg;
                }
            } else if (!future.isCanceled()) {
                m_synchronizer.addFuture(future);
            }
        }

        if (m_synchronizer.futures().isEmpty())
            break;

        QThread::msleep(15);
    }
}

bool GoPackageImporter::runImportTask(const QString &location, const QString &name)
{
    QDir pkgDir(location);
    if (pkgDir.exists()) {
        pkgDir.setNameFilters(C_GO_FILES_FILTER);
        QStringList goFiles = pkgDir.entryList(QDir::Files);
        if (!goFiles.isEmpty()) {
            QStringList notOpenedGoFiles;
            QList<SourceWithRevision> openedGoDocs;
            const QMap<QString, GoEditorDocumentHandle *> *goDocs = GoCodeModelManager::instance()->goEditorDocuments();

            int candidatesCount = 0;
            QString directlyUpdatingDocName = "";
            for (const QString &goFile: goFiles) {
                if (goFile.endsWith(C_GO_TEST_FILE, Qt::CaseSensitive))
                    continue;
                const QString goFilePath = location + QStringLiteral("/") + goFile;
                const QMap<QString, GoEditorDocumentHandle *>::const_iterator it = goDocs->constFind(goFilePath);
                if (it != goDocs->constEnd()) {
                    if (m_directlyUpdatingDocs.constFind(goFilePath) != m_directlyUpdatingDocs.constEnd()) {
                        directlyUpdatingDocName = goFilePath;
                        candidatesCount--;
                    } else {
                        openedGoDocs.append(SourceWithRevision(it.value()));
                    }
                }
                else
                    notOpenedGoFiles.append(goFilePath);
                candidatesCount++;
            }

            if (candidatesCount || !directlyUpdatingDocName.isEmpty()) {
                const GoLang::GoGeneralSettings &settings = GoLang::GoSettings::generalSettings();
                GoSource::Ptr doc;
                if (!directlyUpdatingDocName.isEmpty())
                    doc = m_directlyUpdatingDocs.take(directlyUpdatingDocName);
                QFuture<GoPackage *> result = Utils::runAsync(GoCodeModelManager::instance()->sharedThreadPool(),
                                                              parsePackageFiles, location,
                                                              notOpenedGoFiles, openedGoDocs, doc, name,
                                                              settings.goRoot(), settings.goPath());
                m_synchronizer.addFuture(result);

                Core::ProgressManager::addTask(result, QString("Importing package \"%1\"").arg(name),
                                               Go::Constants::GO_IMPORT_TASK);
                return true;
            }
        }
    }

    return false;
}


void GoPackageImporter::proceedTasks()
{
    QList<QFuture<GoPackage *> > futures = m_synchronizer.futures();
    m_synchronizer.clearFutures();
    for (const QFuture<GoPackage *> &future: futures) {
        if (future.isFinished()) {
            if (GoPackage *pkg = future.result()) {
                qDebug() << "   <- package " << pkg->location();
                GoPackageKeySet unimported = m_cache->insertPackage(pkg);
                for (const GoPackageKey &task: unimported) {
                    int before = m_allTasks.size();
                    QSet<GoPackageKey>::iterator it = m_allTasks.insert(task);
                    int after = m_allTasks.size();
                    if (after != before)
                        m_tasksToProceed.append(*it);
                }
            }
        } else if (!future.isCanceled()) {
            m_synchronizer.addFuture(future);
        }
    }

    while (!m_tasksToProceed.isEmpty() && m_synchronizer.futures().size() <= 10) {
        GoPackageKey task = m_tasksToProceed.takeFirst();
        qDebug() << " -> starting task" << task.first << task.second;
        if (!runImportTask(task.first, task.second))
            m_cache->insertEmptyPackage(new GoPackage(task.first, task.second));
    }

    if (!m_tasksToProceed.isEmpty() || !m_synchronizer.futures().isEmpty()) {
        m_timer->start(25);
        return;
    }

    emit packageCacheUpdated();
}

}   // namespace GoTools

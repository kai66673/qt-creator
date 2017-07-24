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
#include "gopackage.h"
#include "gocodemodelmanager.h"
#include "gosource.h"
#include "gosettings.h"
#include "goconstants.h"
#include "packagetype.h"

#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/runextensions.h>

#include <QDir>

namespace GoTools {

QStringList C_GO_FILES_FILTER({QStringLiteral("*.go")});

GoPackageCache *GoPackageCache::m_instance = 0;

GoPackage::GoPackage(const QString &location, const QString &name)
    : m_location(location)
    , m_name(name)
    , m_type(new PackageType(m_sources))
{ }

GoPackage::~GoPackage()
{
    delete m_type;
}

QString GoPackage::location() const
{ return m_location; }

QString GoPackage::name() const
{ return m_name; }

void GoPackage::insertGoSource(const GoSource::Ptr &doc)
{
    QMutexLocker lock(&m_sourcesMutex);
    m_sources[doc->fileName()] = doc;
    doc->setPackage(this);
}

void GoPackage::removeGoSource(const QString &path)
{
    QMutexLocker lock(&m_sourcesMutex);
    m_sources.remove(path);
}

QHash<QString, GoSource::Ptr> GoPackage::sources() const
{
    QMutexLocker lock(&m_sourcesMutex);
    return m_sources;
}

PackageType *GoPackage::type() const
{ return m_type; }

GoPackageCache::GoPackageCache(GoCodeModelManager *model, QObject *parent)
    : QObject(parent)
    , m_model(model)
    , m_mutex(QMutex::Recursive)
{
    m_instance = this;
    connect(&m_indexingFutureWatcher, &QFutureWatcher<IndexingTaskListResult>::finished,
            this, &GoPackageCache::indexingTaskFinished);

}

GoPackageCache::~GoPackageCache()
{ clean(); }

void GoPackageCache::update(const GoSource::Ptr &doc)
{
    if (!doc)
        return;

    QMutexLocker lock(&m_mutex);

    // remove doc from package if
    // 1. doc is invalid
    // 2. package name changed
    QHash<QString, GoPackage *>::iterator filesIterator = m_fileToPackageHash.find(doc->fileName());
    if (filesIterator != m_fileToPackageHash.end()) {
        GoPackage *pkg = filesIterator.value();
        if (!doc->translationUnit()->fileAst() || doc->packageName() != pkg->name()) {
            pkg->removeGoSource(doc->fileName());
            m_fileToPackageHash.erase(filesIterator);
        }
    }

    QHash<GoPackageKey, GoPackage *>::iterator pkgIt = m_packages.find({doc->location(), doc->packageName()});
    if (pkgIt == m_packages.end())
        importPackage({doc->location(), doc->packageName()}, GoCodeModelManager::instance()->buildWorkingCopy(), doc);
    else {
        pkgIt.value()->insertGoSource(doc);
        m_fileToPackageHash[doc->fileName()] = pkgIt.value();
    }
}

bool GoPackageCache::initializeDocument(const WorkingCopy &workingCopy, GoSource::Ptr &doc)
{
    const QString filePath = doc->fileName();

    if (workingCopy.contains(filePath)) {
        const QPair<QByteArray, unsigned> entry = workingCopy.get(filePath);
        doc->setSource(entry.first);
        doc->setRevision(entry.second);
        return true;
    }

    QFile file(filePath);
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
        doc->setSource(source);
        doc->setRevision(0);
        return true;
    }

    return false;
}

GoPackage *GoPackageCache::importPackage(const GoPackageKey &pkgKey, const WorkingCopy &workingCopy, const GoSource::Ptr &currentDoc)
{
    const GoLang::GoGeneralSettings &settings = GoLang::GoSettings::generalSettings();
    const QString goRoot = settings.goRoot();
    const QString goPath = settings.goPath();

    GoPackage *newPackage = new GoPackage(pkgKey.first, pkgKey.second);
    if (!currentDoc.isNull()) {
        newPackage->insertGoSource(currentDoc);
        m_fileToPackageHash[currentDoc->fileName()] = newPackage;
    }

    QDir pkgDir(pkgKey.first);
    if (pkgDir.exists()) {
        pkgDir.setNameFilters(C_GO_FILES_FILTER);
        QStringList goFiles = pkgDir.entryList(QDir::Files);
        if (!currentDoc.isNull()) {
            const QString currentDocName = currentDoc->fileName().mid(pkgKey.first.length() + 1);
            goFiles.removeOne(currentDocName);
        }

        for (const QString &goFile: goFiles) {
            GoSource::Ptr newDoc = GoSource::create(pkgKey.first + "/" + goFile);
            if (initializeDocument(workingCopy, newDoc)) {
                if (newDoc->parsePackageFile(pkgKey.second)) {
                    newDoc->resolveImportsAndPackageName(goRoot, goPath);
                    newPackage->insertGoSource(newDoc);
                    m_fileToPackageHash[newDoc->fileName()] = newPackage;
                }
            }
        }
    }

    m_packages.insert(pkgKey, newPackage);
    return newPackage;
}

void GoPackageCache::addIndexingTask(const IndexingTask &task)
{
    m_indexingTodo.push_back(task);
    indexingWakeUp();
}

void GoPackageCache::runIndexing(QFutureInterface<IndexingTaskListResult> &future, const QSet<QString> &files, const QString &goRoot, const QString &goPath)
{
    IndexingTaskListResult result;

    future.setProgressRange(0, files.size());

    int progress = 0;
    for (const QString &fileName: files) {
        if (future.isCanceled())
            break;
        if (fileName.endsWith(QLatin1String(".go"))) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly)) {
                GoSource::Ptr newDoc = GoSource::create(fileName);
                QByteArray source = file.readAll();
                for (int i = 0; i < source.size(); i++) {
                    if (source[i] == '\r') {
                        if (source[i+1] == '\n')
                            source[i++] = ' ';
                        else
                            source[i] = '\n';
                    }
                }
                newDoc->setSource(source);
                newDoc->setRevision(0);
                newDoc->parse(TranslationUnit::Full);
                newDoc->resolveImportsAndPackageName(goRoot, goPath);
                result[{newDoc->location(), newDoc->packageName()}].append(newDoc);
            }
        }
        future.setProgressValue(++progress);
    }

    future.reportFinished(&result);
}

void GoPackageCache::indexingWakeUp()
{
    if (m_indexingTodo.isEmpty() || m_indexingFutureWatcher.isRunning())
        return;

    const GoLang::GoGeneralSettings &settings = GoLang::GoSettings::generalSettings();
    const QString goRoot = settings.goRoot();
    const QString goPath = settings.goPath();

    const IndexingTask task = m_indexingTodo.takeFirst();
    QFuture<IndexingTaskListResult> future = Utils::runAsync(GoCodeModelManager::instance()->sharedThreadPool(),
                                                             &GoPackageCache::runIndexing, this, task.second, goRoot, goPath);
    m_indexingFutureWatcher.setFuture(future);
    if (task.first.isEmpty())
        Core::ProgressManager::addTask(future, tr("Indexing Go files"), "Go.Files.Index");
    else
        Core::ProgressManager::addTask(future, tr("Indexing project %1 Go files").arg(task.first), "Go.Files.Index");
}

void GoPackageCache::cancelIndexing()
{
    m_indexingTodo.clear();
    if (m_indexingFutureWatcher.isRunning()) {
        m_indexingFutureWatcher.cancel();
        m_indexingFutureWatcher.waitForFinished();
    }
}

void GoPackageCache::indexingTaskFinished()
{
    IndexingTaskListResult result = m_indexingFutureWatcher.future().result();
    {
        QMutexLocker lock(&m_mutex);
        for (IndexingTaskListResult::const_iterator pkg_it = result.constBegin(); pkg_it != result.constEnd(); ++pkg_it) {
            GoPackageKey pkgKey = pkg_it.key();
            QHash<GoPackageKey, GoPackage *>::iterator findPackageIt = m_packages.find(pkgKey);
            GoPackage *package;
            if (findPackageIt == m_packages.end()) {
                package = new GoPackage(pkgKey.first, pkgKey.second);
                m_packages.insert(pkgKey, package);
            } else {
                package = findPackageIt.value();
            }
            for (const GoSource::Ptr &doc: pkg_it.value()) {
                package->insertGoSource(doc);
                m_fileToPackageHash[doc->fileName()] = package;
            }
        }
    }

    indexingWakeUp();
}

GoPackageCache *GoPackageCache::instance()
{ return m_instance; }

void GoPackageCache::clean()
{
    cancelIndexing();

    {
        QMutexLocker lock(&m_mutex);

        qDeleteAll(m_packages.values());
        m_packages.clear();
        m_fileToPackageHash.clear();
    }

    emit packageCacheCleaned();
}

void GoPackageCache::indexGoFiles(const QString &projectName, const QSet<QString> &deletedFiles, const QSet<QString> &addedFiles)
{
    {
        QMutexLocker lock(&m_mutex);
        for (const QString &fileToDelete: deletedFiles) {
            QHash<QString, GoPackage *>::iterator filesIterator = m_fileToPackageHash.find(fileToDelete);
            if (filesIterator != m_fileToPackageHash.end()) {
                GoPackage *pkg = filesIterator.value();
                pkg->removeGoSource(fileToDelete);
                m_fileToPackageHash.erase(filesIterator);
            }
        }
    }

    addIndexingTask({projectName, addedFiles});
}

void GoPackageCache::acquireCache()
{ m_mutex.lock(); }

void GoPackageCache::releaseCache()
{ m_mutex.unlock(); }

}   // namespace GoTools

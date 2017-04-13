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
#include "gosnapshot.h"

namespace GoTools {

GoPackageCache *GoPackageCache::m_instance = 0;

GoPackage::GoPackage(const QString &location, const QString &name, unsigned revision)
    : m_location(location)
    , m_name(name)
    , m_revision(revision)
{ }

QString GoPackage::location() const
{ return m_location; }

QString GoPackage::name() const
{ return m_name; }

GoPackageKeySet GoPackage::depencies() const
{
    GoPackageKeySet result;

    for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
         it != m_sources.constEnd(); ++it)
        result += it.value()->importTasks();

    return result;
}

QStringList GoPackage::files() const
{
    QStringList result;

    for (QHash<QString, GoSource::Ptr>::const_iterator it = m_sources.constBegin();
         it != m_sources.constEnd(); ++it)
        result << it.value()->fileName();

    return result;
}

void GoPackage::insertGoSource(const QString &path, GoSource::Ptr doc)
{ m_sources[path] = doc; }

void GoPackage::removeGoSource(const QString &path)
{ m_sources.remove(path); }

QHash<QString, GoSource::Ptr> GoPackage::sources() const
{ return m_sources; }

GoPackageCache::GoPackageCache(GoCodeModelManager *model, QObject *parent)
    : QObject(parent)
    , m_importer(this)
    , m_model(model)
    , m_snapshot(0)
    , m_snapshotRefCount(0)
    , m_dirty(false)
{
    m_instance = this;
    connect(&m_importer, &GoPackageImporter::packageCacheUpdated,
            this, &GoPackageCache::onPackageCacheUpdated);
}

GoPackageCache::~GoPackageCache()
{ clean(); }

void GoPackageCache::update(GoSource::Ptr doc)
{
    if (!doc)
        return;

    // remove doc from package if
    // 1. doc is invalid
    // 2. package name changed
    QHash<QString, GoPackage *>::iterator filesIterator = m_fileToPackageHash.find(doc->fileName());
    if (filesIterator != m_fileToPackageHash.end()) {
        GoPackage *pkg = filesIterator.value();
        if (!doc->translationUnit()->fileAst() || doc->packageName() != pkg->name())
            pkg->removeGoSource(doc->fileName());
    }

    GoPackageKey packageKey({doc->location(), doc->packageName()});

    QHash<GoPackageKey, GoPackage *>::iterator pkgIt = m_packages.find(packageKey);
    if (pkgIt == m_packages.end())
        m_importer.importDocPackage(doc);
    else {
        pkgIt.value()->insertGoSource(doc->fileName(), doc);
        importPackages(doc->importTasks());
    }
}

void GoPackageCache::importPackages(const GoPackageKeySet &packages)
{
    m_importer.importPackages(packages - m_packageSet);
}

void GoPackageCache::insertEmptyPackage(GoPackage *pkg)
{
    auto key = qMakePair(pkg->location(), pkg->name());
    m_packages[key] = pkg;
    m_packageSet.insert(key);
}

GoPackageKeySet GoPackageCache::insertPackage(GoPackage *pkg)
{
    auto key = qMakePair(pkg->location(), pkg->name());
    m_packages[key] = pkg;
    m_packageSet.insert(key);

    for (const QString &filePath: pkg->files())
        m_fileToPackageHash[filePath] = pkg;

    return pkg->depencies() - m_packageSet;
}

GoPackageCache *GoPackageCache::instance()
{ return m_instance; }

GoSnapshot *GoPackageCache::getSnapshot()
{
    QMutexLocker lock(&m_snapshotMutex);

    if (m_snapshot)
        m_snapshotRefCount++;
    return m_snapshot;
}

void GoPackageCache::releaseSnapshot()
{
    bool needEmit = false;

    {
        QMutexLocker lock(&m_snapshotMutex);

        if (m_snapshotRefCount)
            m_snapshotRefCount--;
        if (!m_snapshotRefCount) {
            if (m_dirty) {
                m_dirty = false;
                if (m_snapshot)
                    delete m_snapshot;
                m_snapshot = new GoSnapshot(m_packages);
                needEmit = true;
            }
        }
    }

    if (needEmit)
        emit packageCacheUpdated();
}

void GoPackageCache::clean()
{
    m_importer.clean();

    {
        QMutexLocker lock(&m_snapshotMutex);

        while (m_snapshotRefCount)
            QThread::msleep(25);
        if (m_snapshot)
            delete m_snapshot;
        m_snapshot = 0;
    }

    qDeleteAll(m_packages.values());
    m_packages.clear();
    m_packageSet.clear();
    m_fileToPackageHash.clear();

    emit packageCacheCleaned();
}

void GoPackageCache::onPackageCacheUpdated()
{
    bool needEmit = false;

    {
        QMutexLocker lock(&m_snapshotMutex);

        if (!m_snapshotRefCount) {
            if (m_snapshot)
                delete m_snapshot;
            m_snapshot = new GoSnapshot(m_packages);
            needEmit = true;
        } else {
            m_dirty = true;
        }
    }

    if (needEmit)
        emit packageCacheUpdated();
}

}   // namespace GoTools

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

#include "gosource.h"
#include "gopackageimporter.h"
#include "gopackagetypes.h"

#include <QString>
#include <QHash>
#include <QMutex>

namespace GoTools {

class GoPackageCache;
class GoCodeModelManager;
class GoSnapshot;

class GoPackage
{
public:
    GoPackage(const QString &location, const QString &name, unsigned revision = 0);

    QString location() const;
    QString name() const;

    GoPackageKeySet depencies() const;
    QStringList files() const;
    void insertGoSource(const QString &path, GoSource::Ptr doc);
    void removeGoSource(const QString &path);

    QHash<QString, GoSource::Ptr> sources() const;

private:
    QString m_location;
    QString m_name;
    unsigned m_revision;
    QHash<QString, GoSource::Ptr> m_sources;
};

class GoPackageCache: public QObject
{
    Q_OBJECT

public:
    GoPackageCache(GoCodeModelManager *model, QObject *parent = 0);
    virtual ~GoPackageCache();

    void update(GoSource::Ptr doc);
    void importPackages(const GoPackageKeySet &packages);
    void insertEmptyPackage(GoPackage *pkg);
    GoPackageKeySet insertPackage(GoPackage *pkg);

    static GoPackageCache *instance();

    GoSnapshot *getSnapshot();
    void releaseSnapshot();

    void clean();

signals:
    void packageCacheUpdated();
    void packageCacheCleaned();

private:
    void onPackageCacheUpdated();

private:
    static GoPackageCache *m_instance;

    GoPackageImporter m_importer;
    GoCodeModelManager *m_model;

    QHash<GoPackageKey, GoPackage *> m_packages;
    GoPackageKeySet m_packageSet;
    QHash<QString, GoPackage *> m_fileToPackageHash;

    QMutex m_snapshotMutex;
    GoSnapshot *m_snapshot;
    unsigned m_snapshotRefCount;
    bool m_dirty;
};

}   // namespace GoTools

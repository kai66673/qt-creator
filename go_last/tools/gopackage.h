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
#include "gopackagetypes.h"
#include "goeditordocumenthandle.h"
#include "goworkingcopy.h"

#include <QString>
#include <QHash>
#include <QMutex>
#include <QFutureWatcher>

namespace GoTools {

class GoPackageCache;
class GoCodeModelManager;
class PackageType;

class GoPackage
{
    friend class ResolveContext;

public:
    GoPackage(const QString &location, const QString &name);
    ~GoPackage();

    QString location() const;
    QString name() const;

    void insertGoSource(const GoSource::Ptr &doc);
    void removeGoSource(const QString &path);

    QHash<QString, GoSource::Ptr> sources() const;

    PackageType *type() const;

private:
    QString m_location;
    QString m_name;

    mutable QMutex m_sourcesMutex;
    QHash<QString, GoSource::Ptr> m_sources;
    PackageType *m_type;
};

class GoPackageCache: public QObject
{
    Q_OBJECT

    friend class ResolveContext;

public:
    GoPackageCache(GoCodeModelManager *model, QObject *parent = 0);
    virtual ~GoPackageCache();

    void update(const GoSource::Ptr &doc);

    static GoPackageCache *instance();

    void clean();
    void indexGoFiles(const QString &projectName, const QSet<QString> &deletedFiles, const QSet<QString> &addedFiles);

private:
    void acquireCache();
    void releaseCache();
    bool initializeDocument(const WorkingCopy &workingCopy, GoSource::Ptr &doc);
    GoPackage *importPackage(const GoPackageKey &pkgKey, const WorkingCopy &workingCopy, const GoSource::Ptr &currentDoc = GoSource::Ptr());

    typedef QPair<QString, QSet<QString> > IndexingTask;
    typedef QList<IndexingTask> IndexingTaskList;
    typedef QHash<GoPackageKey, QList<GoSource::Ptr> > IndexingTaskListResult;

    void addIndexingTask(const IndexingTask &task);
    void runIndexing(QFutureInterface<IndexingTaskListResult> &future, const QSet<QString> &files, const QString &goRoot, const QString &goPath);
    void indexingWakeUp();
    void cancelIndexing();
    void indexingTaskFinished();

    IndexingTaskList m_indexingTodo;
    QFutureWatcher<IndexingTaskListResult> m_indexingFutureWatcher;

signals:
    void packageCacheUpdated();
    void packageCacheCleaned();

private:
    static GoPackageCache *m_instance;

    GoCodeModelManager *m_model;

    QMutex m_mutex;
    QHash<GoPackageKey, GoPackage *> m_packages;
    QHash<QString, GoPackage *> m_fileToPackageHash;
};

}   // namespace GoTools

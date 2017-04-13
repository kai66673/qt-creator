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
#include "gocodemodelmanager.h"
#include "goeditordocumenthandle.h"

#include <QMutex>
#include <QMap>

namespace GoTools {

static QMutex m_instanceMutex;
static GoCodeModelManager *m_instance;

class GoCodeModelManagerPrivate
{
public:
    // Editor integration
    mutable QMutex m_goEditorDocumentsMutex;
    QMap<QString, GoEditorDocumentHandle *> m_goEditorDocuments;

    QThreadPool m_threadPool;
};

GoCodeModelManager::GoCodeModelManager(QObject *parent)
    : QObject(parent)
    , d(new GoCodeModelManagerPrivate)
    , m_packageCache(this)
{ }

GoCodeModelManager::~GoCodeModelManager()
{ delete d; }

GoCodeModelManager *GoCodeModelManager::instance()
{
    if (m_instance)
        return m_instance;

    QMutexLocker locker(&m_instanceMutex);
    if (!m_instance)
        m_instance = new GoCodeModelManager;

    return m_instance;
}

void GoCodeModelManager::registerGoEditorDocument(GoEditorDocumentHandle *goEditorDocument)
{
    if (goEditorDocument) {
        QMutexLocker locker(&d->m_goEditorDocumentsMutex);
        d->m_goEditorDocuments.insert(goEditorDocument->filePath(), goEditorDocument);
    }
}

void GoCodeModelManager::unregisterGoEditorDocument(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    QMutexLocker locker(&d->m_goEditorDocumentsMutex);
    d->m_goEditorDocuments.remove(filePath);
}

QThreadPool *GoCodeModelManager::sharedThreadPool()
{ return &d->m_threadPool; }

const QMap<QString, GoEditorDocumentHandle *> *GoCodeModelManager::goEditorDocuments() const
{ return &d->m_goEditorDocuments; }

void GoCodeModelManager::cleanPackageCache()
{
    m_packageCache.clean();
    indexPackageDirs();
}

void GoCodeModelManager::indexPackageDirs()
{ m_packageIndexer.reindex(); }

GoPackageDirIndexer::PackageDirs GoCodeModelManager::indexedPackageDirs() const
{ return m_packageIndexer.packages(); }

}   // namespace GoTools

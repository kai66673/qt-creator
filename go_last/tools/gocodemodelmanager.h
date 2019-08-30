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

#include "gopackage.h"
#include "gopackagedirindexer.h"
#include "goworkingcopy.h"

#include <QObject>
#include <QThreadPool>

namespace GoTools {

class GoCodeModelManagerPrivate;
class GoEditorDocumentHandle;

class GoCodeModelManager : public QObject
{
    Q_OBJECT

public:
    explicit GoCodeModelManager(QObject *parent = 0);
    virtual ~GoCodeModelManager();

    static GoCodeModelManager *instance();

    void registerGoEditorDocument(GoEditorDocumentHandle *goEditorDocument);
    void unregisterGoEditorDocument(const QString &filePath);

    QThreadPool *sharedThreadPool();
    const QMap<QString, GoEditorDocumentHandle *> *goEditorDocuments() const;
    WorkingCopy buildWorkingCopy() const;

    void cleanPackageCache();
    void indexPackageDirs();
    GoPackageDirIndexer::PackageDirs indexedPackageDirs() const;

    void findReferences(GoSource::Ptr source, int pos) const;
    void renameSymbolUnderCursor(GoSource::Ptr source, int pos) const;

private:
    GoCodeModelManagerPrivate *d;
    GoPackageCache m_packageCache;
    GoPackageDirIndexer m_packageIndexer;
};

}   // namespace GoTools

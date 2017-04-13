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

#include <QAction>
#include <QObject>
#include <QSortFilterProxyModel>

namespace TextEditor { class TextEditorWidget; }
namespace Utils { class TreeViewComboBox; }

namespace GoTools {

class GoOverviewModel;

class GoEditorOutline : public QObject
{
    Q_OBJECT

public:
    explicit GoEditorOutline(TextEditor::TextEditorWidget *editorWidget);

    void update(GoTools::GoSource::Ptr doc);

    GoOverviewModel *model() const;
    QModelIndex modelIndex();

    QWidget *widget() const; // Must be deleted by client.

signals:
    void modelIndexChanged(const QModelIndex &index);

public slots:
    void updateIndex();
    void setSorted(bool sort);

private:
    void updateNow();
    void updateIndexNow();
    void gotoSymbolInEditor();

    GoEditorOutline();

    bool isSorted() const;
    QModelIndex indexForPosition(int pos, const QModelIndex &rootIndex = QModelIndex()) const;

private:
    TextEditor::TextEditorWidget *m_editorWidget;
    GoSource::Ptr m_doc;

    Utils::TreeViewComboBox *m_combo; // Not owned
    GoOverviewModel *m_model;
    QSortFilterProxyModel *m_proxyModel;
    QModelIndex m_modelIndex;
    QAction *m_sortAction;
    QTimer *m_updateTimer;
    QTimer *m_updateIndexTimer;
};

}   // namespace GoTools

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

#include <texteditor/ioutlinewidget.h>
#include <utils/navigationtreeview.h>

#include <QSortFilterProxyModel>

namespace GoTools {
class GoOverviewModel;
}

namespace GoEditor {
namespace Internal {

class GoEditorWidget;

class GoOutlineTreeView: public Utils::NavigationTreeView
{
    Q_OBJECT
public:
    GoOutlineTreeView(QWidget *parent);

    void contextMenuEvent(QContextMenuEvent *event) override;
};

class GoOutlineFilterModel: public QSortFilterProxyModel
{
    Q_OBJECT

public:
    GoOutlineFilterModel(GoTools::GoOverviewModel *sourceModel, QObject *parent);

    // QSortFilterProxyModel
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    Qt::DropActions supportedDragActions() const override;

private:
    GoTools::GoOverviewModel *m_sourceModel;
};

class GoOutlineWidget: public TextEditor::IOutlineWidget
{
    Q_OBJECT

public:
    GoOutlineWidget(GoEditorWidget *editor);

    // IOutlineWidget
    virtual QList<QAction*> filterMenuActions() const override;
    virtual void setCursorSynchronization(bool syncWithCursor) override;

private:
    void modelUpdated();
    void updateSelectionInTree(const QModelIndex &index);
    void updateTextCursor(const QModelIndex &proxyIndex);
    void onItemActivated(const QModelIndex &index);
    bool syncCursor();

private:
    GoEditorWidget *m_editor;
    GoOutlineTreeView *m_treeView;
    GoTools::GoOverviewModel *m_model;
    GoOutlineFilterModel *m_proxyModel;

    bool m_enableCursorSync;
    bool m_blockCursorSync;
};

class GoOutlineWidgetFactory: public TextEditor::IOutlineWidgetFactory
{
    Q_OBJECT

public:
    bool supportsEditor(Core::IEditor *editor) const override;
    TextEditor::IOutlineWidget *createWidget(Core::IEditor *editor) override;
};

}   // namespace Internal
}   // namespace GoEditor


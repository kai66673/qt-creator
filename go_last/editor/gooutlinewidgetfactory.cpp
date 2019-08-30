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
#include "gooutlinewidgetfactory.h"
#include "goeditor.h"
#include "gooverviewmodel.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/find/itemviewfind.h>
#include <utils/qtcassert.h>

#include <QBoxLayout>
#include <QMenu>
#include <QDebug>

namespace GoEditor {
namespace Internal {

enum {
    debug = false
};

GoOutlineTreeView::GoOutlineTreeView(QWidget *parent)
    : Utils::NavigationTreeView(parent)
{
    setExpandsOnDoubleClick(false);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
}

void GoOutlineTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    if (!event)
        return;

    QMenu contextMenu;

    QAction *action = contextMenu.addAction(tr("Expand All"));
    connect(action, &QAction::triggered, this, &QTreeView::expandAll);
    action = contextMenu.addAction(tr("Collapse All"));
    connect(action, &QAction::triggered, this, &QTreeView::collapseAll);

    contextMenu.exec(event->globalPos());

    event->accept();
}

GoOutlineFilterModel::GoOutlineFilterModel(GoTools::GoOverviewModel *sourceModel, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_sourceModel(sourceModel)

{
    setSourceModel(m_sourceModel);
}

bool GoOutlineFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    // ignore artifical "<Select Symbol>" entry
    if (!sourceParent.isValid() && sourceRow == 0)
        return false;

    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

Qt::DropActions GoOutlineFilterModel::supportedDragActions() const
{ return sourceModel()->supportedDragActions(); }

GoOutlineWidget::GoOutlineWidget(GoEditorWidget *editor)
    : TextEditor::IOutlineWidget()
    , m_editor(editor)
    , m_treeView(new GoOutlineTreeView(this))
    , m_model(m_editor->outline()->model())
    , m_proxyModel(new GoOutlineFilterModel(m_model, this))
    , m_enableCursorSync(true)
    , m_blockCursorSync(false)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(Core::ItemViewFind::createSearchableWrapper(m_treeView));

    setLayout(layout);
    m_treeView->setModel(m_proxyModel);
    setFocusProxy(m_treeView);

    connect(m_model, &QAbstractItemModel::modelReset, this, &GoOutlineWidget::modelUpdated);
    modelUpdated();

    connect(m_editor->outline(), &GoTools::GoEditorOutline::modelIndexChanged,
            this, &GoOutlineWidget::updateSelectionInTree);
    connect(m_treeView, &QAbstractItemView::activated,
            this, &GoOutlineWidget::onItemActivated);
}

QList<QAction *> GoOutlineWidget::filterMenuActions() const
{ return QList<QAction *>(); }

void GoOutlineWidget::setCursorSynchronization(bool syncWithCursor)
{
    m_enableCursorSync = syncWithCursor;
    if (m_enableCursorSync)
        updateSelectionInTree(m_editor->outline()->modelIndex());
}

void GoOutlineWidget::modelUpdated()
{ m_treeView->expandAll(); }

void GoOutlineWidget::updateSelectionInTree(const QModelIndex &index)
{
    if (!syncCursor())
        return;

    QModelIndex proxyIndex = m_proxyModel->mapFromSource(index);

    m_blockCursorSync = true;
    if (debug)
        qDebug() << "GoOutline - updating selection due to cursor move";

    m_treeView->setCurrentIndex(proxyIndex);
    m_treeView->scrollTo(proxyIndex);
    m_blockCursorSync = false;
}

void GoOutlineWidget::updateTextCursor(const QModelIndex &proxyIndex)
{
    QModelIndex index = m_proxyModel->mapToSource(proxyIndex);
    if (GoTools::GoOverviewTreeItem *item = m_model->itemFromIndex(index)) {
        m_blockCursorSync = true;

        if (debug)
            qDebug() << "GoOutline - moving cursor to position" << item->position();

        Core::EditorManager::cutForwardNavigationHistory();
        Core::EditorManager::addCurrentPositionToNavigationHistory();

        QTextCursor tc = m_editor->textCursor();
        tc.setPosition(item->position());
        m_editor->setTextCursor(tc);

        m_blockCursorSync = false;
    }
}

void GoOutlineWidget::onItemActivated(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    updateTextCursor(index);
    m_editor->setFocus();
}

bool GoOutlineWidget::syncCursor()
{ return m_enableCursorSync && !m_blockCursorSync; }

bool GoOutlineWidgetFactory::supportsEditor(Core::IEditor *editor) const
{ return qobject_cast<GoEditor *>(editor) ? true : false; }

TextEditor::IOutlineWidget *GoOutlineWidgetFactory::createWidget(Core::IEditor *editor)
{
    GoEditor *goEditor = qobject_cast<GoEditor *>(editor);
    GoEditorWidget *goEditorWidget = qobject_cast<GoEditorWidget *>(goEditor->widget());
    QTC_ASSERT(goEditorWidget, return 0);

    return new GoOutlineWidget(goEditorWidget);
}

}   // namespace Internal
}   // namespace GoEditor

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
#include "goeditoroutline.h"
#include "gooverviewmodel.h"
#include "gosettings.h"

#include <coreplugin/editormanager/editormanager.h>
#include <texteditor/texteditor.h>
#include <utils/treeviewcombobox.h>

#include <QTimer>

enum { UpdateOutlineIntervalInMs = 500 };

namespace {

QTimer *newSingleShotTimer(QObject *parent, int msInternal, const QString &objectName)
{
    QTimer *timer = new QTimer(parent);
    timer->setObjectName(objectName);
    timer->setSingleShot(true);
    timer->setInterval(msInternal);
    return timer;
}

} // anonymous namespace

namespace GoTools {

GoEditorOutline::GoEditorOutline(TextEditor::TextEditorWidget *editorWidget)
    : QObject(editorWidget)
    , m_editorWidget(editorWidget)
    , m_combo(new Utils::TreeViewComboBox)
    , m_model(new GoOverviewModel(this))
    , m_proxyModel(new QSortFilterProxyModel(this))
{
    // Set up proxy model
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->sort(GoLang::GoSettings::sortedEditorDocumentOutline() ? 0 : -1, Qt::AscendingOrder);
    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    // Set up combo box
    m_combo->setModel(m_proxyModel);

    m_combo->setMinimumContentsLength(22);
    QSizePolicy policy = m_combo->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Expanding);
    m_combo->setSizePolicy(policy);
    m_combo->setMaxVisibleItems(40);

    m_combo->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_sortAction = new QAction(tr("Sort Alphabetically"), m_combo);
    m_sortAction->setCheckable(true);
    m_sortAction->setChecked(isSorted());
    connect(m_sortAction, &QAction::toggled,
            GoLang::GoSettings::instance(),
            &GoLang::GoSettings::setSortedEditorDocumentOutline);
    m_combo->addAction(m_sortAction);

    connect(m_combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &GoEditorOutline::gotoSymbolInEditor);

    // Set up timers
    m_updateTimer = newSingleShotTimer(this, UpdateOutlineIntervalInMs,
                                       QLatin1String("GoEditorOutline::m_updateTimer"));
    connect(m_updateTimer, &QTimer::timeout, this, &GoEditorOutline::updateNow);

    m_updateIndexTimer = newSingleShotTimer(this, UpdateOutlineIntervalInMs,
                                            QLatin1String("GoEditorOutline::m_updateIndexTimer"));
    connect(m_updateIndexTimer, &QTimer::timeout, this, &GoEditorOutline::updateIndexNow);
}

void GoEditorOutline::update(GoTools::GoSource::Ptr doc)
{
    m_doc = doc;
    m_updateTimer->start();
}

GoOverviewModel *GoEditorOutline::model() const
{  return m_model; }

QModelIndex GoEditorOutline::modelIndex()
{
    if (m_modelIndex.isValid())
        return m_modelIndex;

    m_modelIndex = indexForPosition(m_editorWidget->position());
    emit modelIndexChanged(m_modelIndex);

    return m_modelIndex;
}

QWidget *GoEditorOutline::widget() const
{ return m_combo; }

void GoEditorOutline::updateIndex()
{ m_updateIndexTimer->start(); }

void GoEditorOutline::setSorted(bool sort)
{
    if (sort != isSorted()) {
        if (sort)
            m_proxyModel->sort(0, Qt::AscendingOrder);
        else
            m_proxyModel->sort(-1, Qt::AscendingOrder);
        bool block = m_sortAction->blockSignals(true);
        m_sortAction->setChecked(m_proxyModel->sortColumn() == 0);
        m_sortAction->blockSignals(block);
        updateIndexNow();
    }
}

void GoEditorOutline::updateNow()
{
    if (!m_doc)
        return;

    if (m_doc->revision() != m_editorWidget->document()->revision()) {
        m_updateTimer->start();
        return;
    }

    m_model->rebuild(m_doc);
    m_combo->view()->expandAll();
    updateIndexNow();
}

void GoEditorOutline::updateIndexNow()
{
    if (!m_doc)
        return;

    if (m_doc->revision() != m_editorWidget->document()->revision()) {
        m_updateIndexTimer->start();
        return;
    }

    m_updateIndexTimer->stop();

    m_modelIndex = QModelIndex(); //invalidate
    QModelIndex comboIndex = modelIndex();

    if (comboIndex.isValid()) {
        bool blocked = m_combo->blockSignals(true);
        m_combo->setCurrentIndex(m_proxyModel->mapFromSource(comboIndex));
        m_combo->blockSignals(blocked);
    }
}

void GoEditorOutline::gotoSymbolInEditor()
{
    QModelIndex modelIndex = m_combo->view()->currentIndex();
    const QModelIndex sourceIndex = m_proxyModel->mapToSource(modelIndex);
    if (GoOverviewTreeItem *item = m_model->itemFromIndex(sourceIndex)) {
        Core::EditorManager::cutForwardNavigationHistory();
        Core::EditorManager::addCurrentPositionToNavigationHistory();
        int line;
        int column;
        m_editorWidget->convertPosition(item->position(), &line, &column);
        m_editorWidget->gotoLine(line, column);
        m_editorWidget->activateEditor();
    }
}

bool GoEditorOutline::isSorted() const
{ return m_proxyModel->sortColumn() == 0; }

QModelIndex GoEditorOutline::indexForPosition(int pos, const QModelIndex &rootIndex) const
{
    QModelIndex lastIndex = rootIndex;

    const int rowCount = m_model->rowCount(rootIndex);
    for (int row = 0; row < rowCount; row++) {
        const QModelIndex index = m_model->index(row, 0, rootIndex);
        GoOverviewTreeItem *item = m_model->itemFromIndex(index);
        if (item && item->position() > pos)
            break;
        lastIndex = index;
    }

    if (lastIndex != rootIndex)
        lastIndex = indexForPosition(pos, lastIndex);

    return lastIndex;
}

}   //

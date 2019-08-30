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

#include <QAbstractItemModel>

namespace GoTools {

class GoOverviewTreeItem
{
public:
    enum Kind {
        Root = 0,
        SelectHint,
        ConstantDeclaration,
        TypeDeclaration,
        VariableDeclaration,
        FunctionDeclaration
    };

    GoOverviewTreeItem(const QString &name, int position, Kind kind, GoOverviewTreeItem *parent);
    GoOverviewTreeItem(const QString &name, const QString &toolTip, int position, Kind kind, GoOverviewTreeItem *parent);
    virtual ~GoOverviewTreeItem();

    GoOverviewTreeItem *parent() const;
    QString name() const;
    QString toolTip() const;
    Kind kind() const;
    int position() const;

    int childCount() const;
    GoOverviewTreeItem *childAt(int index);
    void appandChild(GoOverviewTreeItem *child);
    int row() const;


private:
    QList<GoOverviewTreeItem *> m_childrens;
    GoOverviewTreeItem *m_parent;

    QString m_name;
    QString m_toolTip;
    int m_position;
    Kind m_kind;
};

class GoOverviewModel : public QAbstractItemModel
{
public:
    GoOverviewModel(QObject *parent = 0);
    virtual ~GoOverviewModel();

    // Header:
    QVariant headerData(int, Qt::Orientation, int = Qt::DisplayRole) const;

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void rebuild(GoSource::Ptr doc);
    GoOverviewTreeItem *itemFromIndex(const QModelIndex &index);

    unsigned revision() const;

private:
    unsigned m_revision;
    GoOverviewTreeItem *m_rootItem;
};

}   // namespace GoTools

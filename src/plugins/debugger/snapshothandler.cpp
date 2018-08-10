/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "snapshothandler.h"

#include "debuggerinternalconstants.h"
#include "debuggericons.h"
#include "debuggercore.h"
#include "debuggerruncontrol.h"

#include <utils/qtcassert.h>

#include <QDebug>
#include <QFile>

namespace Debugger {
namespace Internal {

#if 0
SnapshotData::SnapshotData()
{}

void SnapshotData::clear()
{
    m_frames.clear();
    m_location.clear();
    m_date = QDateTime();
}

QString SnapshotData::function() const
{
    if (m_frames.isEmpty())
        return QString();
    const StackFrame &frame = m_frames.at(0);
    return frame.function + QLatin1Char(':') + QString::number(frame.line);
}

QString SnapshotData::toString() const
{
    QString res;
    QTextStream str(&res);
/*    str << SnapshotHandler::tr("Function:") << ' ' << function() << ' '
        << SnapshotHandler::tr("File:") << ' ' << m_location << ' '
        << SnapshotHandler::tr("Date:") << ' ' << m_date.toString(); */
    return res;
}

QString SnapshotData::toToolTip() const
{
    QString res;
    QTextStream str(&res);
    str << "<html><body><table>"
/*
        << "<tr><td>" << SnapshotHandler::tr("Function:")
            << "</td><td>" << function() << "</td></tr>"
        << "<tr><td>" << SnapshotHandler::tr("File:")
            << "</td><td>" << QDir::toNativeSeparators(m_location) << "</td></tr>"
        << "</table></body></html>"; */
    return res;
}

QDebug operator<<(QDebug d, const  SnapshotData &f)
{
    QString res;
    QTextStream str(&res);
    str << f.location();
/*
    str << "level=" << f.level << " address=" << f.address;
    if (!f.function.isEmpty())
        str << ' ' << f.function;
    if (!f.location.isEmpty())
        str << ' ' << f.location << ':' << f.line;
    if (!f.from.isEmpty())
        str << " from=" << f.from;
    if (!f.to.isEmpty())
        str << " to=" << f.to;
*/
    d.nospace() << res;
    return d;
}
#endif

////////////////////////////////////////////////////////////////////////
//
// SnapshotHandler
//
////////////////////////////////////////////////////////////////////////

/*!
    \class Debugger::Internal::SnapshotHandler
    \brief The SnapshotHandler class provides a model to represent the
    snapshots in a QTreeView.

    A snapshot represents a debugging session.
*/

SnapshotHandler::SnapshotHandler() = default;

SnapshotHandler::~SnapshotHandler()
{
    for (int i = m_snapshots.size(); --i >= 0; ) {
        if (DebuggerRunTool *runTool = at(i)) {
            const DebuggerRunParameters &rp = runTool->runParameters();
            if (rp.isSnapshot && !rp.coreFile.isEmpty())
                QFile::remove(rp.coreFile);
        }
    }
}

int SnapshotHandler::rowCount(const QModelIndex &parent) const
{
    // Since the stack is not a tree, row count is 0 for any valid parent
    return parent.isValid() ? 0 : m_snapshots.size();
}

int SnapshotHandler::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant SnapshotHandler::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_snapshots.size())
        return QVariant();

    const DebuggerRunTool *runTool = at(index.row());

    if (role == SnapshotCapabilityRole)
        return runTool && runTool->activeEngine()->hasCapability(SnapshotCapability);

    if (!runTool)
        return QLatin1String("<finished>");

    const DebuggerRunParameters &rp = runTool->runParameters();

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return rp.displayName;
        case 1:
            return rp.coreFile.isEmpty() ? rp.inferior.executable : rp.coreFile;
        }
        return QVariant();

    case Qt::ToolTipRole:
        return QVariant();

    case Qt::DecorationRole:
        // Return icon that indicates whether this is the active stack frame.
        if (index.column() == 0)
            return (index.row() == m_currentIndex) ? Icons::LOCATION.icon() : Icons::EMPTY.icon();

    default:
        break;
    }
    return QVariant();
}

QVariant SnapshotHandler::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("File");
        };
    }
    return QVariant();
}

Qt::ItemFlags SnapshotHandler::flags(const QModelIndex &index) const
{
    if (index.row() >= m_snapshots.size())
        return nullptr;
    if (index.row() == m_snapshots.size())
        return QAbstractTableModel::flags(index);
    return true ? QAbstractTableModel::flags(index) : Qt::ItemFlags({});
}

void SnapshotHandler::activateSnapshot(int index)
{
    beginResetModel();
    m_currentIndex = index;
    //qDebug() << "ACTIVATING INDEX: " << m_currentIndex << " OF " << size();
    Internal::displayDebugger(at(index));
    endResetModel();
}

void SnapshotHandler::createSnapshot(int index)
{
    DebuggerRunTool *runTool = at(index);
    QTC_ASSERT(runTool, return);
    runTool->engine()->createSnapshot();
}

void SnapshotHandler::removeSnapshot(int index)
{
    DebuggerRunTool *runTool = at(index);
    //qDebug() << "REMOVING " << runTool;
    QTC_ASSERT(runTool, return);
#if 0
    // See http://sourceware.org/bugzilla/show_bug.cgi?id=11241.
    setState(EngineSetupRequested);
    postCommand("set stack-cache off");
#endif
    //QString fileName = runTool->startParameters().coreFile;
    //if (!fileName.isEmpty())
    //    QFile::remove(fileName);
    beginResetModel();
    m_snapshots.removeAt(index);
    if (index == m_currentIndex)
        m_currentIndex = -1;
    else if (index < m_currentIndex)
        --m_currentIndex;
    //runTool->quitDebugger();
    endResetModel();
}


void SnapshotHandler::removeAll()
{
    beginResetModel();
    m_snapshots.clear();
    m_currentIndex = -1;
    endResetModel();
}

void SnapshotHandler::appendSnapshot(DebuggerRunTool *runTool)
{
    beginResetModel();
    m_snapshots.append(runTool);
    m_currentIndex = size() - 1;
    endResetModel();
}

void SnapshotHandler::removeSnapshot(DebuggerRunTool *runTool)
{
    // Could be that the run controls died before it was appended.
    int index = m_snapshots.indexOf(runTool);
    if (index != -1)
        removeSnapshot(index);
}

void SnapshotHandler::setCurrentIndex(int index)
{
    beginResetModel();
    m_currentIndex = index;
    endResetModel();
}

DebuggerRunTool *SnapshotHandler::at(int i) const
{
    return m_snapshots.at(i).data();
}

} // namespace Internal
} // namespace Debugger

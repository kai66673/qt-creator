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

#include "goprojectnode.h"
#include "goproject.h"

namespace GoLang {

GoProjectNode::GoProjectNode(GoProject &project, const Utils::FilePath &projectFilePath)
    : ProjectExplorer::ProjectNode(projectFilePath)
    , m_project(project)
{ }

bool GoProjectNode::supportsAction(ProjectExplorer::ProjectAction action, const Node *node) const
{
    if (node->asFileNode()) {
        return action == ProjectExplorer::ProjectAction::Rename
            || action == ProjectExplorer::ProjectAction::RemoveFile;
    }
    if (node->isFolderNodeType() || node->isProjectNodeType()) {
        return action == ProjectExplorer::ProjectAction::AddNewFile
            || action == ProjectExplorer::ProjectAction::RemoveFile
            || action == ProjectExplorer::ProjectAction::AddExistingFile;
    }
    return ProjectNode::supportsAction(action, node);
}

bool GoProjectNode::addFiles(const QStringList &filePaths, QStringList *)
{ return m_project.addFiles(filePaths); }

bool GoProjectNode::removeFiles(const QStringList &filePaths, QStringList *)
{ return m_project.removeFiles(filePaths); }

bool GoProjectNode::deleteFiles(const QStringList &)
{ return true; }

bool GoProjectNode::renameFile(const QString &filePath, const QString &newFilePath)
{ return m_project.renameFile(filePath, newFilePath); }


}   // GoLang

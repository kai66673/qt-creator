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

GoProjectNode::GoProjectNode(GoProject &project, const Utils::FileName &projectFilePath)
    : ProjectExplorer::ProjectNode(projectFilePath)
    , m_project(project)
{ }

QList<ProjectExplorer::ProjectAction> GoProjectNode::supportedActions(ProjectExplorer::Node *node) const
{
    static const QList<ProjectExplorer::ProjectAction> fileActions = { ProjectExplorer::ProjectAction::Rename,
                                                                       ProjectExplorer::ProjectAction::RemoveFile
                                                                     };
    static const QList<ProjectExplorer::ProjectAction> folderActions = { ProjectExplorer::ProjectAction::AddNewFile,
                                                                         ProjectExplorer::ProjectAction::RemoveFile,
                                                                         ProjectExplorer::ProjectAction::AddExistingFile
                                                                       };
    switch (node->nodeType()) {
    case ProjectExplorer::NodeType::File:
        return fileActions;
    case ProjectExplorer::NodeType::Folder:
    case ProjectExplorer::NodeType::Project:
        return folderActions;
    default:
        return ProjectNode::supportedActions(node);
    }
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

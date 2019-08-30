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

#include "gooutputparser.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

using ProjectExplorer::Task;

namespace GoLang {

GoOutputParser::GoOutputParser()
{
    m_regexpError.setPattern(QLatin1String("^([^:]+):([\\d]+):"));
    m_regexpError.setMinimal(true);
    QTC_CHECK(m_regexpError.isValid());
}

void GoOutputParser::stdError(const QString &line)
{
    parseErrorLine(line);
}

void GoOutputParser::stdOutput(const QString &line)
{
    Q_UNUSED(line)
}

void GoOutputParser::setWorkingDirectory(const QString &workingDirectory)
{ m_workingDirectory = workingDirectory; }

void GoOutputParser::doFlush()
{
    if (m_currentTask.isNull())
        return;
    Task t = m_currentTask;
    m_currentTask.clear();
    emit addTask(t);
}

void GoOutputParser::parseErrorLine(const QString &line)
{
    if (-1 != m_regexpError.indexIn(line)) {
        const int lineNo = m_regexpError.cap(2).toInt();
        const QString description = line;
        QString filepath = m_workingDirectory + QLatin1String("/") + m_regexpError.cap(1);
        doFlush();
        m_currentTask = Task(Task::Error, description, Utils::FileName::fromString(filepath),
                             lineNo, ProjectExplorer::Constants::TASK_CATEGORY_COMPILE);
    } else if (line.startsWith(QLatin1Char('\t')) && !m_currentTask.isNull()) {
        m_currentTask.description.append(line.mid(1));
    }
}

} // namespace GoLang

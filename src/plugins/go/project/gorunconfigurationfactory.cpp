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

#include "gorunconfigurationfactory.h"
#include "goproject.h"
#include "gorunconfiguration.h"
#include "golangconstants.h"

#include <projectexplorer/target.h>

#include <memory>

namespace GoLang {

GoRunConfigurationFactory::GoRunConfigurationFactory()
{ }

QList<Core::Id> GoRunConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent,
                                                                ProjectExplorer::IRunConfigurationFactory::CreationMode mode) const
{
    Q_UNUSED(mode);

    if (!canHandle(parent))
        return {};

    return { Constants::C_GORUNCONFIGURATION_ID };
}

QString GoRunConfigurationFactory::displayNameForId(Core::Id id) const
{
    return id.toString();
}

bool GoRunConfigurationFactory::canCreate(ProjectExplorer::Target *parent, Core::Id id) const
{
    Q_UNUSED(id);
    return canHandle(parent);
}

bool GoRunConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    Q_UNUSED(map);
    return canHandle(parent);
}

bool GoRunConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *product) const
{
    QTC_ASSERT(parent, return false);
    QTC_ASSERT(product, return false);
    return true;
}

ProjectExplorer::RunConfiguration *GoRunConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *product)
{
    QTC_ASSERT(parent, return nullptr);
    QTC_ASSERT(product, return nullptr);
    std::unique_ptr<GoRunConfiguration> result(new GoRunConfiguration(parent, Constants::C_GORUNCONFIGURATION_ID));
    return result->fromMap(product->toMap()) ? result.release() : nullptr;
}

bool GoRunConfigurationFactory::canHandle(ProjectExplorer::Target *parent) const
{
    return qobject_cast<GoProject *>(parent->project());
}

ProjectExplorer::RunConfiguration *GoRunConfigurationFactory::doCreate(ProjectExplorer::Target *parent, Core::Id id)
{
    auto result = new GoRunConfiguration(parent, id);
    return result;
}

ProjectExplorer::RunConfiguration *GoRunConfigurationFactory::doRestore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    auto result = new GoRunConfiguration(parent, ProjectExplorer::idFromMap(map));
    result->fromMap(map);
    return result;
}

}   // namespace GoLang

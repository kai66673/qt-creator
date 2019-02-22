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

#include "goconfigurations.h"
#include "gotoolchain.h"
#include "golangconstants.h"

#include <debugger/debuggeritem.h>
#include <debugger/debuggeritemmanager.h>
#include <debugger/debuggerkitinformation.h>
#include <projectexplorer/kit.h>
#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/toolchainmanager.h>
#include <utils/algorithm.h>
#include <utils/qtcassert.h>

using namespace ProjectExplorer;
using namespace Debugger;

namespace GoLang {

static GoConfigurations *m_instance = nullptr;

static GoToolChain *autoDetectedGoToolChain()
{
    for (ToolChain *tc: ToolChainManager::toolChains()) {
        if (tc->isAutoDetected() && tc->typeId() == Constants::GO_TOOLCHAIN_ID)
            return static_cast<GoToolChain *>(tc);
    }

    return nullptr;
}

static QSet<Kit *> existingAutoDetectedGoKits()
{
    return Utils::filtered(KitManager::kits(), [](Kit *kit) -> bool {
        if (!kit->isAutoDetected())
            return false;
        return ToolChainKitAspect::toolChain(kit, Constants::C_GOLANGUAGE_ID) != nullptr;
    }).toSet();
}

GoConfigurations *GoConfigurations::instance()
{ return m_instance; }

void GoConfigurations::initialize()
{
    QTC_CHECK(m_instance == nullptr);
    m_instance = new GoConfigurations();
}

void GoConfigurations::updateAutomaticKitList()
{
    GoToolChain *goToolChain = autoDetectedGoToolChain();
    QSet<Kit *> existingGoKits = existingAutoDetectedGoKits();

    if (goToolChain == nullptr) {
        // Remove autodetected go kits
        foreach (Kit *kit, existingGoKits)
            KitManager::deregisterKit(kit);
        return;
    }

    if (!existingGoKits.empty())
        return;

    // Setting Up new Go kit
    const DebuggerItem *possibleDebugger = DebuggerItemManager::findByEngineType(GdbEngineType);
    const QVariant debuggerId = (possibleDebugger ? possibleDebugger->id() : QVariant());

    auto newKit = std::make_unique<Kit>();

    Kit *kit = newKit.get();
    kit->blockNotification();

    kit->setAutoDetected(true);
    kit->setUnexpandedDisplayName("Go-Desktop");
    DeviceTypeKitAspect::setDeviceTypeId(kit, ProjectExplorer::Constants::DESKTOP_DEVICE_TYPE);
    ToolChainKitAspect::setToolChain(kit, goToolChain);
    if (debuggerId.isValid())
        DebuggerKitAspect::setDebugger(kit, debuggerId);

    kit->setMutable(DeviceKitAspect::id(), true);
    kit->setSticky(ToolChainKitAspect::id(), true);
    kit->setSticky(DeviceTypeKitAspect::id(), true);
    kit->setSticky(SysRootKitAspect::id(), true);
    kit->setSticky(DebuggerKitAspect::id(), true);

    kit->unblockNotification();
    KitManager::registerKit(std::move(newKit));
}

GoConfigurations::GoConfigurations(QObject *parent)
    : QObject(parent)
{ }

}   // namespace GoLang

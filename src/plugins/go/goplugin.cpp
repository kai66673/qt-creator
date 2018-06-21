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
#include "goplugin.h"
#include "goiconprovider.h"
#include "goconstants.h"
#include "golangconstants.h"
#include "goeditor.h"
#include "gotoolchain.h"
#include "gobuildconfigurationfactory.h"
#include "gorunconfigurationfactory.h"
#include "gocompilerbuildstepfactory.h"
#include "gocompilercleanstepfactory.h"
#include "gocompilergetstepfactory.h"
#include "gogeneralsettingspage.h"
#include "gotoolssettingspage.h"
#include "golangjsextensions.h"
#include "golangconstants.h"
#include "gosettings.h"
#include "gosource.h"
#include "gocodemodelmanager.h"
#include "goproject.h"
#include "gooutlinewidgetfactory.h"
#include "goeditorconstants.h"
#include "gorunconfiguration.h"
#include "goconfigurations.h"

#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/jsexpander.h>
#include <coreplugin/iwizardfactory.h>
#include <projectexplorer/kitmanager.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/toolchainmanager.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/snippets/snippetprovider.h>
#include <utils/mimetypes/mimedatabase.h>

#include <QMenu>

namespace GoLang {

class GoLangPluginFeatureProvider : public Core::IFeatureProvider
{
public:
    virtual QSet<Core::Id> availableFeatures(Core::Id /*id*/) const override;
    virtual QSet<Core::Id> availablePlatforms() const override { return QSet<Core::Id>(); }
    virtual QString displayNameForPlatform(Core::Id /*id*/) const override { return QString(); }
};

QSet<Core::Id> GoLangPluginFeatureProvider::availableFeatures(Core::Id) const
{
    return QSet<Core::Id>() << Core::Id(GoLang::Constants::GO_SUPPORT_FEATURE);
}

}   // namespace GoLang

namespace Go {
namespace Internal {

using namespace GoEditor::Internal;

class GoPluginPrivate
{
public:
    GoPluginPrivate() : iconProvider(nullptr) { }

    GoIconProvider *iconProvider;
    GoLang::GoSettings *settings;

    QAction *m_findUsagesAction;
    QAction *m_renameSymbolUnderCursorAction;

    GoEditorFactory editorFactory;
    GoOutlineWidgetFactory outlineWidgetFactory;
    GoLang::GoToolChainFactory toolChainFactory;
    GoLang::GoBuildConfigurationFactory buildConfigurationFactory;
    GoLang::GoRunConfigurationFactory runConfigurationFactory;
    GoLang::GoCompilerGetStepFactory compilerGetStepFactory;
    GoLang::GoCompilerBuildStepFactory compilerBuildStepFactory;
    GoLang::GoCompilerCleanStepFactory compilerCleanStepFactory;
    GoLang::GoGeneralSettingsPage generalSettingsPage;
    GoLang::GoToolsSettingsPage toolsSettingsPage;
};

GoPlugin *GoPlugin::m_instance(nullptr);

GoPlugin::GoPlugin()
    : d(nullptr)
{
    m_instance = this;
}

GoPlugin::~GoPlugin()
{
    delete d->iconProvider;
    delete d;

    m_instance = nullptr;
}

bool GoPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    d = new GoPluginPrivate;

    GoLang::GoConfigurations::initialize();

    qRegisterMetaType<GoTools::GoSource::Ptr>("GoTools::GoSource::Ptr");

    ProjectExplorer::ToolChainManager::registerLanguage(GoLang::Constants::C_GOLANGUAGE_ID,
                                                        GoLang::Constants::C_GOLANGUAGE_NAME);

    auto constraint = [](ProjectExplorer::RunConfiguration *runConfig) {
        return qobject_cast<GoLang::GoRunConfiguration *>(runConfig) != nullptr;
    };

    ProjectExplorer::RunControl::registerWorker<ProjectExplorer::SimpleTargetRunner>
            (ProjectExplorer::Constants::NORMAL_RUN_MODE, constraint);

    TextEditor::SnippetProvider::registerGroup(Go::Constants::GO_SNIPPETS_GROUP_ID,
                                               tr("Go", "GoEditor::GoSnippetProvider"),
                                               &::GoEditor::Internal::GoEditor::decorateEditor);

    d->iconProvider = new GoIconProvider;

    ProjectExplorer::ProjectManager::registerProjectType<GoLang::GoProject>(GoLang::Constants::C_GO_PROJECT_MIMETYPE);

    Core::IWizardFactory::registerFeatureProvider(new GoLang::GoLangPluginFeatureProvider);
    Core::JsExpander::registerQObjectForJs(QLatin1String("GoLang"), new GoLang::GoLangJsUtils);

    readSettings();

    GoTools::GoCodeModelManager *codeModel = GoTools::GoCodeModelManager::instance();
    codeModel->setParent(this);
    d->settings = new GoLang::GoSettings(this);
    connect(d->settings, &GoLang::GoSettings::generalSettingsChanged,
            codeModel, &GoTools::GoCodeModelManager::cleanPackageCache);

    createActions();

    return true;
}

void GoPlugin::extensionsInitialized()
{
    GoTools::GoCodeModelManager::instance()->indexPackageDirs();
    connect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitsLoaded,
            this, &GoPlugin::kitsRestored);
}

ExtensionSystem::IPlugin::ShutdownFlag GoPlugin::aboutToShutdown()
{
    writeSettings();
    return SynchronousShutdown;
}

void GoPlugin::readSettings()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(Constants::GOPLUGIN_GROUP));

    // Use settings->value() method in order to restore setting

    settings->endGroup();
}

void GoPlugin::writeSettings()
{
    QSettings *settings = Core::ICore::settings();
    settings->beginGroup(QLatin1String(Constants::GOPLUGIN_GROUP));

    // Use settings->setValue() method in order to save setting

    settings->endGroup();
}

GoPlugin *GoPlugin::instance()
{ return m_instance; }

void GoPlugin::emitOutlineSortingChanged(bool sorted)
{ emit outlineSortingChanged(sorted); }

void GoPlugin::findUsages()
{
    if (Core::IEditor *currentEditor = Core::EditorManager::currentEditor())
        if (GoEditorWidget *editorWidget = qobject_cast<GoEditorWidget *>(currentEditor->widget()))
            editorWidget->findUsages();
}

void GoPlugin::renameSymbolUnderCursor()
{
    if (Core::IEditor *currentEditor = Core::EditorManager::currentEditor())
        if (GoEditorWidget *editorWidget = qobject_cast<GoEditorWidget *>(currentEditor->widget()))
            editorWidget->renameSymbolUnderCursor();
}

void GoPlugin::createActions()
{
    Core::Context context(::GoEditor::Constants::GOEDITOR_ID);
    Core::Command *cmd;

    Core::ActionContainer *contextMenu = Core::ActionManager::createMenu(::GoEditor::Constants::M_CONTEXT);

    Core::ActionContainer *mtools = Core::ActionManager::actionContainer(Core::Constants::M_TOOLS);
    Core::ActionContainer *goToolsMenu = Core::ActionManager::createMenu(::GoEditor::Constants::M_TOOLS_GO);
    QMenu *menu = goToolsMenu->menu();
    menu->setTitle(tr("&Go"));
    menu->setEnabled(true);
    mtools->addMenu(goToolsMenu);

    cmd = Core::ActionManager::command(TextEditor::Constants::FOLLOW_SYMBOL_UNDER_CURSOR);
    goToolsMenu->addAction(cmd);
    contextMenu->addAction(cmd);

    d->m_findUsagesAction = new QAction(tr("Find Usages"), this);
    cmd = Core::ActionManager::registerAction(d->m_findUsagesAction, ::GoEditor::Constants::FIND_USAGES, context);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+U")));
    connect(d->m_findUsagesAction, &QAction::triggered, this, &GoPlugin::findUsages);
    contextMenu->addAction(cmd);
    goToolsMenu->addAction(cmd);

    d->m_renameSymbolUnderCursorAction = new QAction(tr("Rename Symbol Under Cursor"), this);
    cmd = Core::ActionManager::registerAction(d->m_renameSymbolUnderCursorAction, ::GoEditor::Constants::RENAME_SYMBOL_UNDER_CURSOR, context);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+R")));
    connect(d->m_renameSymbolUnderCursorAction, &QAction::triggered, this, &GoPlugin::renameSymbolUnderCursor);
    contextMenu->addAction(cmd);
    goToolsMenu->addAction(cmd);
}

void GoPlugin::kitsRestored()
{
    disconnect(ProjectExplorer::KitManager::instance(), &ProjectExplorer::KitManager::kitsLoaded,
               this, &GoPlugin::kitsRestored);
    GoLang::GoConfigurations::updateAutomaticKitList();
}

}   // namespace Internal
}   // namespace Go

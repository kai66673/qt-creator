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
#include "golexer.h"
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

#include <coreplugin/icore.h>
#include <coreplugin/jsexpander.h>
#include <coreplugin/iwizardfactory.h>
#include <projectexplorer/projectmanager.h>
#include <projectexplorer/toolchainmanager.h>
#include <texteditor/snippets/snippetprovider.h>
#include <utils/mimetypes/mimedatabase.h>

namespace GoLang {

class GoLangPluginFeatureProvider : public Core::IFeatureProvider
{
public:
    virtual QSet<Core::Id> availableFeatures(Core::Id /*id*/) const override {
        return QSet<Core::Id>() << Core::Id(GoLang::Constants::GO_SUPPORT_FEATURE);
    }
    virtual QSet<Core::Id> availablePlatforms() const override { return QSet<Core::Id>(); }
    virtual QString displayNameForPlatform(Core::Id /*id*/) const override { return QString(); }
};

}   // namespace GoLang

namespace Go {
namespace Internal {

using namespace GoEditor::Internal;

class GoPluginPrivate
{
public:
    GoPluginPrivate()
        : lexer(0)
        , iconProvider(0)
    { }

    GoLexer *lexer;
    GoIconProvider *iconProvider;
    GoLang::GoSettings *settings;
};

GoPlugin *GoPlugin::m_instance(nullptr);

GoPlugin::GoPlugin()
    : d(new GoPluginPrivate)
{
    m_instance = this;
}

GoPlugin::~GoPlugin()
{
    delete d->lexer;
    delete d->iconProvider;
    delete d;

    m_instance = nullptr;
}

bool GoPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)

    qRegisterMetaType<GoTools::GoSource::Ptr>("GoTools::GoSource::Ptr");

    ProjectExplorer::ToolChainManager::registerLanguage(GoLang::Constants::C_GOLANGUAGE_ID,
                                                        GoLang::Constants::C_GOLANGUAGE_NAME);
    TextEditor::SnippetProvider::registerGroup(Go::Constants::GO_SNIPPETS_GROUP_ID,
                                               tr("Go", "GoEditor::GoSnippetProvider"),
                                               &::GoEditor::Internal::GoEditor::decorateEditor);

    d->lexer = new GoLexer;
    d->iconProvider = new GoIconProvider;

    addAutoReleasedObject(new GoEditorFactory);
    addAutoReleasedObject(new GoOutlineWidgetFactory);

    addAutoReleasedObject(new GoLang::GoToolChainFactory);
    addAutoReleasedObject(new GoLang::GoBuildConfigurationFactory);
    addAutoReleasedObject(new GoLang::GoRunConfigurationFactory);
    addAutoReleasedObject(new GoLang::GoCompilerGetStepFactory);
    addAutoReleasedObject(new GoLang::GoCompilerBuildStepFactory);
    addAutoReleasedObject(new GoLang::GoCompilerCleanStepFactory);
    addAutoReleasedObject(new GoLang::GoGeneralSettingsPage);

    addAutoReleasedObject(new GoLang::GoToolsSettingsPage);

    ProjectExplorer::ProjectManager::registerProjectType<GoLang::GoProject>(GoLang::Constants::C_GO_PROJECT_MIMETYPE);

    Core::IWizardFactory::registerFeatureProvider(new GoLang::GoLangPluginFeatureProvider);
    Core::JsExpander::registerQObjectForJs(QLatin1String("GoLang"), new GoLang::GoLangJsUtils);

    readSettings();

    GoTools::GoCodeModelManager *codeModel = GoTools::GoCodeModelManager::instance();
    codeModel->setParent(this);
    d->settings = new GoLang::GoSettings(this);
    connect(d->settings, &GoLang::GoSettings::generalSettingsChanged,
            codeModel, &GoTools::GoCodeModelManager::cleanPackageCache);

    return true;
}

void GoPlugin::extensionsInitialized()
{ GoTools::GoCodeModelManager::instance()->indexPackageDirs();}

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

}   // namespace Internal
}   // namespace Go

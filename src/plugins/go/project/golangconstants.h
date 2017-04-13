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

namespace GoLang {
namespace Constants {

const char C_GOPROJECT_ID[] = "Go.GoProject";

const char GO_TOOLCHAIN_TYPEID[] = "ProjectExplorer.ToolChain.Go";

const char GO_PROJECT_MIMETYPE[] = "application/x-goproject";
const char GO_PROJECT_ID[]       = "GoProjectManager.GoProject";
const char GO_PROJECT_PROJECTCONTEXT[] = "GoProject.ProjectContext";
const char GO_PROJECT_SUFFIX[] = ".goproject";
const char LANG_GO[]           = "GOLANG";
const char GO_TOOLCHAIN_ID[]   = "GoLang.Toolchain";
const char TOOLCHAIN_SETTINGS_PAGE_ID[] = "GoLang.SettingsPage";
const char GO_BUILDCONFIGURATION_ID[] = "GoLang.Buildconfiguration";
const char GO_GOSTEP_ID[] = "GoLang.BuildConfiguration.GoStep";
const char GO_RUNCONFIG_ID[] = "GoLang.GoRunConfiguration";
const char GO_SUPPORT_FEATURE[] = "GoLang.GoSupport";

// NimProject
const char C_GO_PROJECT_MIMETYPE[] = "text/x-go-project";
const char C_GOPROJECT_EXCLUDEDFILES[] = "Go.GoProjectExcludedFiles";

// GoRunConfiguration
const char C_GORUNCONFIGURATION_ID[] = "GoLang.GoRunConfiguration";
const char C_GORUNCONFIGURATION_DISPLAY[] = QT_TRANSLATE_NOOP("GoRunConfiguration", "Current Build Target");
const char C_GORUNCONFIGURATION_DEFAULT_DISPLAY[] = QT_TRANSLATE_NOOP("GoRunConfiguration", "Current Build Target");
const QString C_GORUNCONFIGURATION_EXECUTABLE_KEY = QStringLiteral("GoLang.GoRunConfiguration.Executable");
const QString C_GORUNCONFIGURATION_WORKINGDIRECTORYASPECT_ID = QStringLiteral("GoLang.GoRunConfiguration.WorkingDirectoryAspect");
const QString C_GORUNCONFIGURATION_ARGUMENTASPECT_ID = QStringLiteral("GoLang.GoRunConfiguration.ArgumentAspect");
const QString C_GORUNCONFIGURATION_TERMINALASPECT_ID = QStringLiteral("GoLang.GoRunConfiguration.TerminalAspect");

// NimBuildConfigurationWidget
const char C_GOBUILDCONFIGURATIONWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("GoBuildConfigurationWidget","General");

const char C_GOLANGUAGE_ID[] = "Go";
const char C_GOLANGUAGE_NAME[] = QT_TRANSLATE_NOOP("GoToolsSettingsPage", "Go");


// GoBuildConfiguration
const char C_GOBUILDCONFIGURATION_ID[] = "Go.GoBuildConfiguration";
const QString C_GOBUILDCONFIGURATION_DISPLAY_KEY = QStringLiteral("Go.GoBuildConfiguration.Display");
const QString C_GOBUILDCONFIGURATION_BUILDDIRECTORY_KEY = QStringLiteral("Go.GoBuildConfiguration.BuildDirectory");

// NimCompilerBuildStepWidget
const char C_GOCOMPILERGETSTEPWIDGET_SUMMARY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStepConfigWidget", "Go get step");
const char C_GOCOMPILERGETSTEPWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStepConfigWidget", "Go get step");
const char C_GOCOMPILERBUILDSTEPWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStepConfigWidget", "Go build step");
const char C_GOCOMPILERBUILDSTEPWIDGET_SUMMARY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStepConfigWidget", "Go build step");
const char C_GOCOMPILERCLEANSTEPWIDGET_DISPLAY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStepConfigWidget", "Go clean step");
const char C_GOCOMPILERCLEANSTEPWIDGET_SUMMARY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStepConfigWidget", "Go clean step");

// GoCompilerGetStep
const char C_GOCOMPILERGETSTEP_ID[] = "Go.GoCompilerGetStep";
const char C_GOCOMPILERGETSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("GoCompilerGetStep", "Go Compiler Get Step");
const QString C_GOCOMPILERGETSTEP_USERCOMPILEROPTIONS = QStringLiteral("Go.GoCompilerGetStep.UserCompilerOptions");

// GoCompilerBuildStep
const char C_GOCOMPILERBUILDSTEP_ID[] = "Go.GoCompilerBuildStep";
const char C_GOCOMPILERBUILDSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("GoCompilerBuildStep", "Go Compiler Build Step");
const QString C_GOCOMPILERBUILDSTEP_USERCOMPILEROPTIONS = QStringLiteral("Go.GoCompilerBuildStep.UserCompilerOptions");

// GoCompilerCleanStep
const char C_GOCOMPILERCLEANSTEP_ID[] = "Go.GoCompilerCleanStep";
const char C_GOCOMPILERCLEANSTEP_DISPLAY[] = QT_TRANSLATE_NOOP("GoCompilerCleanStepFactory", "Go Compiler Clean Step");
const QString C_GOCOMPILERCLEANSTEP_USERCOMPILEROPTIONS = QStringLiteral("Go.GoCompilerCleanStep.UserCompilerOptions");

// GoLangSettingsPage
const char C_GOLANGSETTINGSPAGE_CATEGORY[]          = "Z.GoLang";
const char C_GOLANGSETTINGSPAGE_CATEGORY_DISPLAY[]  = QT_TRANSLATE_NOOP("GoLangSettingsPage", "Go");
const char C_GOLANG_ICON_PATH[]                     = ":/go/images/go.png";
// GoGeneralSettingsPage
const char C_GOLANGGENERALSETTINGSPAGE_ID[]         = QT_TRANSLATE_NOOP("GoGeneralSettingsPage", "GoLang.GoGeneralSettingsPage");
const char C_GOLANGGENERAKSETTINGSPAGE_DISPLAY[]    = QT_TRANSLATE_NOOP("GoGeneralSettingsPage", "General");
const char C_GOLANG_GENERAL_SETTINGS_GROUP[]        = "GoGeneralSettingsPage";
const char C_GOLANG_GENERAL_SETTINGS_GOPATH_KEY[]   = "GOPATH";
const char C_GOLANG_GENERAL_SETTINGS_GOROOT_KEY[]   = "GOROOT";
const char C_GOLANG_GENERAL_SETTINGS_GOOS_KEY[]     = "GOOS";
const char C_GOLANG_GENERAL_SETTINGS_GOARCH_KEY[]   = "GOARCH";
// GoToolsSettingsPage
const char C_GOLANGTOOLSSETTINGSPAGE_ID[]           = QT_TRANSLATE_NOOP("GoToolsSettingsPage", "GoLang.GoToolsSettingsPage");
const char C_GOLANGTOOLSSETTINGSPAGE_DISPLAY[]      = QT_TRANSLATE_NOOP("GoToolsSettingsPage", "Tools");
const char C_GOLANG_TOOLS_SETTINGS_GROUP[]          = "GoToolsSettingsPage";
const char C_GOLANG_TOOLS_SETTINGS_GOIMPORTS_KEY[]  = "GoImports";
const char C_GOLANG_TOOLS_SETTINGS_GOFMT_KEY[]      = "GoFmt";
// Sort editor outline
const char C_GOLANG_TOOLS_SORT_EDITOR_OUTLINE[]     = "GoLang.EditorSettings/SortOutline";

} // namespace Go
} // namespace Constants

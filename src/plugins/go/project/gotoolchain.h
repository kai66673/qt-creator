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

#include <projectexplorer/toolchain.h>
#include <projectexplorer/abi.h>
#include <projectexplorer/headerpath.h>

namespace GoLang {

class GoToolChain : public ProjectExplorer::ToolChain
{
public:
    GoToolChain(Core::Id typeId, Detection d);
    GoToolChain(Detection d);

    QString version() const;
    QList<ProjectExplorer::Abi> supportedAbis () const override;
    void setSupportedAbis(const QList<ProjectExplorer::Abi> &abis);
    void setTargetAbi(const ProjectExplorer::Abi &abi);
    void setOriginalTargetTriple(const QString &targetTriple);
    void setCompilerCommand(const Utils::FileName &path, const Utils::FileName &goRoot);
    virtual QString defaultDisplayName() const;

    virtual Utils::FileName goRoot() const;

    // ToolChain interface
    QString typeDisplayName() const override;
    ProjectExplorer::Abi targetAbi() const override;
    QString originalTargetTriple() const override;
    bool isValid() const override;
    PredefinedMacrosRunner createPredefinedMacrosRunner() const override;
    ProjectExplorer::Macros predefinedMacros(const QStringList &) const override;
    CompilerFlags compilerFlags(const QStringList &) const override;
    ProjectExplorer::WarningFlags warningFlags(const QStringList &) const override;
    BuiltInHeaderPathsRunner createBuiltInHeaderPathsRunner() const override;
    ProjectExplorer::HeaderPaths builtInHeaderPaths(const QStringList &flags,
                                                    const Utils::FileName &sysRoot) const final;
    void addToEnvironment(Utils::Environment &env) const override;
    QString makeCommand(const Utils::Environment &) const override;
    virtual Utils::FileName compilerCommand() const override;
    ProjectExplorer::IOutputParser *outputParser() const override;
    std::unique_ptr<ProjectExplorer::ToolChainConfigWidget> createConfigurationWidget() final;
    bool canClone() const override;
    ToolChain *clone() const override final;
    QVariantMap toMap() const override;
    bool fromMap(const QVariantMap &data) override;
    QList<ProjectExplorer::Task> validateKit(const ProjectExplorer::Kit *k) const override;

    static void addCommandPathToEnvironment(const Utils::FileName &command, Utils::Environment &env);
    static QString toString(ProjectExplorer::Abi::Architecture arch, int width);
    static QList<ProjectExplorer::Abi> detectGoAbi(const QString &goRoot);

    class DetectedAbisResult {
    public:
        DetectedAbisResult() = default;
        DetectedAbisResult(const QList<ProjectExplorer::Abi> &supportedAbis,
                           const QString &originalTargetTriple = QString()) :
            supportedAbis(supportedAbis),
            originalTargetTriple(originalTargetTriple)
        { }

        QList<ProjectExplorer::Abi> supportedAbis;
        QString originalTargetTriple;
    };

protected:
    GoToolChain(const GoToolChain &other);
    virtual DetectedAbisResult detectSupportedAbis() const;
    virtual QString detectVersion() const;
    static QString detectVersionForPath(const Utils::FileName &goPath);

private:
    Utils::FileName m_compilerCommand;
    Utils::FileName m_goRoot;

    ProjectExplorer::Abi m_targetAbi;
    mutable QList<ProjectExplorer::Abi> m_supportedAbis;
    mutable QString m_originalTargetTriple;
    mutable QString m_version;

    friend class GoToolChainFactory;
    friend class GoToolChainConfigWidget;
};

class GoToolChainFactory : public ProjectExplorer::ToolChainFactory
{
    Q_OBJECT
public:
    GoToolChainFactory ();

    virtual QList<ProjectExplorer::ToolChain *> autoDetect(const QList<ProjectExplorer::ToolChain *> &alreadyKnown) override;
    virtual QList<ProjectExplorer::ToolChain *> autoDetect(const Utils::FileName &compilerPath, const Core::Id &language) override final;

    virtual bool canCreate() override final;
    virtual ProjectExplorer::ToolChain *create(Core::Id l) override final;

    virtual bool canRestore(const QVariantMap &data) override final;
    virtual ProjectExplorer::ToolChain *restore(const QVariantMap &data) override final;

    virtual QSet<Core::Id> supportedLanguages() const override final;

protected:
    virtual GoToolChain *createToolChain(Core::Id l, bool autoDetect);
};


}   // namespace GoLang

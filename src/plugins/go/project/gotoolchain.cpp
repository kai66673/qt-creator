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

#include "gotoolchain.h"
#include "gotoolchainconfigwidget.h"
#include "golangconstants.h"

#include <projectexplorer/kitinformation.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/fileutils.h>
#include <utils/environment.h>
#include <utils/synchronousprocess.h>
#include <utils/algorithm.h>

#include <QProcess>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

using namespace ProjectExplorer;
using namespace Utils;

namespace GoLang{

static const char compilerCommandKeyC[] = "GoLang.GoToolChain.Path";
static const char goRootKeyC[] = "GoLang.GoToolChain.GoRoot";
static const char targetAbiKeyC[] = "GoLang.GoToolChain.TargetAbi";
static const char supportedAbisKeyC[] = "GoLang.GoToolChain.SupportedAbis";

static QByteArray runGo(const FilePath &goGc, const QStringList &arguments, const QStringList &env)
{
    if (goGc.isEmpty() || !goGc.toFileInfo().isExecutable())
        return QByteArray();

    QProcess go;
    // Force locale: This function is used only to detect settings inside the tool chain, so this is save.
    QStringList environment(env);
    environment.append(QLatin1String("LC_ALL=C"));

    go.setEnvironment(environment);
    go.start(goGc.toString(), arguments);
    if (!go.waitForStarted()) {
        qWarning("%s: Cannot start '%s': %s", Q_FUNC_INFO, qPrintable(goGc.toUserOutput()),
                 qPrintable(go.errorString()));
        return QByteArray();
    }
    go.closeWriteChannel();
    if (!go.waitForFinished(10000)) {
        SynchronousProcess::stopProcess(go);
        qWarning("%s: Timeout running '%s'.", Q_FUNC_INFO, qPrintable(goGc.toUserOutput()));
        return QByteArray();
    }
    if (go.exitStatus() != QProcess::NormalExit) {
        qWarning("%s: '%s' crashed.", Q_FUNC_INFO, qPrintable(goGc.toUserOutput()));
        return QByteArray();
    }

    const QByteArray stdErr = go.readAllStandardError();
    if (go.exitCode() != 0) {
        qWarning().nospace()
                << Q_FUNC_INFO << ": " << goGc.toUserOutput() << ' '
                << arguments.join(QLatin1String(" ")) << " returned exit code "
                << go.exitCode() << ": " << stdErr;
        return QByteArray();
    }

    QByteArray data = go.readAllStandardOutput();
    if (!data.isEmpty() && !data.endsWith('\n'))
        data.append('\n');
    data.append(stdErr);
    return data;
}

static QMap<QString, QString> getGoEnv (const FilePath &goGc, const QStringList &env)
{
    QStringList arguments = QStringList()<<QStringLiteral("env");
    QByteArray output = runGo(goGc, arguments,env);

    QMap<QString, QString> goEnv;
    if(output.isEmpty())
        return goEnv;

    QRegularExpression regExp(HostOsInfo::hostOs() == OsTypeWindows
                              ? QStringLiteral("\\s?(\\w+)\\s?=\\s?(.*)")
                              : QStringLiteral("\\s?(\\w+)\\s?=\\s?\"(.*)\""));

    QList<QByteArray> lines = output.split('\n');
    foreach(const QByteArray &line , lines) {
        QRegularExpressionMatch match = regExp.match(QString::fromLatin1(line));
        if(!match.hasMatch())
            continue;

        if(match.lastCapturedIndex() != 2)
            continue;

        if(!goEnv.contains(match.captured(1)))
            goEnv.insert(match.captured(1),match.captured(2));
    }
    return goEnv;
}

static QVector<Abi> guessGoAbi (const QString &goRoot)
{
    QVector<Abi> result;

    QDir pkgDir(goRoot+QStringLiteral("/pkg"));
    if(!pkgDir.exists())
        return result;

    QStringList abis = pkgDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(const QString &abi,abis) {
        QStringList parts = abi.split(QChar::fromLatin1('_'));

        if(parts.size() == 0 || parts.size() > 2)
            continue;

        Abi::Architecture arch = Abi::UnknownArchitecture;
        Abi::OS os = Abi::UnknownOS;
        Abi::OSFlavor flavor = Abi::UnknownFlavor;
        Abi::BinaryFormat format = Abi::UnknownFormat;
        unsigned char width = 0;
        int unknownCount = 0;

        //copied from gcc code, need to check how all the architectures are named in go
        foreach (const QString &p, parts) {
            if (p == QLatin1String("386") || p == QLatin1String("486") || p == QLatin1String("586")
                    || p == QLatin1String("686") || p == QLatin1String("x86")) {
                arch = Abi::X86Architecture;
                width = 32;
            } else if (p.startsWith(QLatin1String("arm"))) {
                arch = Abi::ArmArchitecture;
                width = 32;
            } /*else if (p == QLatin1String("mipsel")) {
                arch = Abi::MipsArchitecture;
                width = 32;
            } */else if (p == QLatin1String("x86_64") || p == QLatin1String("amd64")) {
                arch = Abi::X86Architecture;
                width = 64;
            } /*else if (p == QLatin1String("powerpc64")) {
                arch = Abi::PowerPCArchitecture;
                width = 64;
            } else if (p == QLatin1String("powerpc")) {
                arch = Abi::PowerPCArchitecture;
                width = 32;
            } */ else if (p == QLatin1String("linux")) {
                os = Abi::LinuxOS;
                if (flavor == Abi::UnknownFlavor)
                    flavor = Abi::GenericFlavor;
                format = Abi::ElfFormat;
            } else if (p.startsWith(QLatin1String("freebsd"))) {
                os = Abi::BsdOS;
                if (flavor == Abi::UnknownFlavor)
                    flavor = Abi::FreeBsdFlavor;
                format = Abi::ElfFormat;
            } else if (p == QLatin1String("windows")) {
                arch = Abi::X86Architecture;
                os = Abi::WindowsOS;
                flavor = Abi::WindowsMSysFlavor;
                format = Abi::PEFormat;
            } /*else if (p == QLatin1String("apple")) {
                os = Abi::MacOS;
                flavor = Abi::GenericMacFlavor;
                format = Abi::MachOFormat;
            } */else {
                ++unknownCount;
            }
        }

        if (arch == Abi::UnknownArchitecture)
            continue;

        result.append(Abi(arch, os, flavor, format, width));
    }
    return result;
}

static QVector<Abi> guessGoAbi (const FileName &goGc, const QStringList &env)
{
    QMap<QString, QString> goEnv = getGoEnv(goGc, env);

    QString rootDirKey = QStringLiteral("GOROOT");
    if(goEnv.isEmpty() || !goEnv.contains(rootDirKey))
        return QVector<Abi>();

    return guessGoAbi(goEnv[rootDirKey]);
}

static QString goVersion(const FileName &path, const QStringList &env)
{
    QStringList arguments(QLatin1String("version"));
    return QString::fromLocal8Bit(runGo(path, arguments, env)).trimmed();
}

GoToolChain::GoToolChain()
    : GoToolChain(Constants::GO_TOOLCHAIN_ID)
{ }

GoToolChain::GoToolChain(Core::Id typeId)
    : ToolChain(typeId)
    , m_compilerCommand(FilePath())
{
    setLanguage(Constants::C_GOLANGUAGE_ID);
}

GoToolChain::GoToolChain(const GoToolChain &other)
    : ToolChain(other.typeId())
    , m_compilerCommand(other.m_compilerCommand)
    , m_goRoot(other.m_goRoot)
    , m_targetAbi(other.m_targetAbi)
    , m_supportedAbis(other.m_supportedAbis)
    , m_originalTargetTriple(other.m_originalTargetTriple)
    , m_version(other.m_version)
{ }

Abi GoToolChain::targetAbi() const
{
    return m_targetAbi;
}

QString GoToolChain::originalTargetTriple() const
{
    if (m_originalTargetTriple.isEmpty())
        m_originalTargetTriple = detectSupportedAbis().originalTargetTriple;
    return m_originalTargetTriple;
}

QString GoToolChain::version() const
{
    if (m_version.isEmpty())
        m_version = detectVersion();
    return m_version;
}

QVector<Abi> GoToolChain::supportedAbis() const
{
    return m_supportedAbis;
}

void GoToolChain::setSupportedAbis(const QVector<Abi> &abis)
{
    m_supportedAbis = abis;
}

void GoToolChain::setTargetAbi(const Abi &abi)
{
    if (abi == m_targetAbi)
        return;

    m_targetAbi = abi;
    toolChainUpdated();
}

void GoToolChain::setOriginalTargetTriple(const QString &targetTriple)
{
    m_originalTargetTriple = targetTriple;
}

void GoToolChain::setCompilerCommand(const FilePath &path, const FilePath &goRoot)
{
    if (path == m_compilerCommand && goRoot == m_goRoot)
        return;

    bool resetDisplayName = displayName() == defaultDisplayName();

    m_compilerCommand = path;
    m_goRoot = goRoot;

    Abi currentAbi = m_targetAbi;
    m_supportedAbis = detectSupportedAbis().supportedAbis;

    m_targetAbi = Abi();
    if (!m_supportedAbis.isEmpty()) {
        if (m_supportedAbis.contains(currentAbi))
            m_targetAbi = currentAbi;
        else
            m_targetAbi = m_supportedAbis.at(0);
    }

    if (resetDisplayName)
        setDisplayName(defaultDisplayName()); // calls toolChainUpdated()!
    else
        toolChainUpdated();
}

QString GoToolChain::defaultDisplayName() const
{
    if (!m_targetAbi.isValid())
        return typeDisplayName();
    return QCoreApplication::translate("GoLang::GoToolChain",
                                       "%1 (%2 %3 in %4)").arg(typeDisplayName(),
                                                               Abi::toString(m_targetAbi.architecture()),
                                                               Abi::toString(m_targetAbi.wordWidth()),
                                                               compilerCommand().parentDir().toUserOutput());
}

FileName GoToolChain::goRoot() const
{ return m_goRoot; }

QString GoToolChain::typeDisplayName() const
{
    return QStringLiteral("Go Toolchain");
}

bool GoToolChain::isValid() const
{
    if (m_compilerCommand.isEmpty() || m_goRoot.isEmpty())
        return false;

    QFileInfo fi = compilerCommand().toFileInfo();
    QFileInfo dir = m_goRoot.toFileInfo();
    return fi.isExecutable() && dir.isDir();
}

ToolChain::MacroInspectionRunner GoToolChain::createMacroInspectionRunner() const
{ return ToolChain::MacroInspectionRunner(); }

Macros GoToolChain::predefinedMacros(const QStringList &) const
{ return Macros(); }

LanguageExtensions GoToolChain::languageExtensions(const QStringList &) const
{ return LanguageExtension::None; }

WarningFlags GoToolChain::warningFlags(const QStringList &) const
{ return WarningFlags::NoWarnings; }

ToolChain::BuiltInHeaderPathsRunner GoToolChain::createBuiltInHeaderPathsRunner() const
{ return ToolChain::BuiltInHeaderPathsRunner(); }

HeaderPaths GoToolChain::builtInHeaderPaths(const QStringList &, const FileName &) const
{ return {}; }

void GoToolChain::addToEnvironment(Environment &env) const
{
    addCommandPathToEnvironment(m_compilerCommand,env);
    env.set(QStringLiteral("GOROOT"), goRoot().toString());
    if (m_targetAbi.architecture() != Abi::UnknownArchitecture)
        env.set(QStringLiteral("GOARCH"), toString(m_targetAbi.architecture(), m_targetAbi.wordWidth()));
}

FilePath GoToolChain::makeCommand(const Environment &) const
{ return FilePath(); }

FilePath GoToolChain::compilerCommand() const
{ return m_compilerCommand; }

IOutputParser *GoToolChain::outputParser() const
{ return nullptr; }

std::unique_ptr<ProjectExplorer::ToolChainConfigWidget> GoToolChain::createConfigurationWidget()
{ return std::make_unique<GoToolChainConfigWidget>(this); }

QVariantMap GoToolChain::toMap() const
{
    QVariantMap map = ToolChain::toMap();
    map.insert(QLatin1String(compilerCommandKeyC), m_compilerCommand.toString());
    map.insert(QLatin1String(goRootKeyC), m_goRoot.toString());
    map.insert(QLatin1String(targetAbiKeyC), m_targetAbi.toString());
    QStringList abiList;
    foreach (const Abi &a, m_supportedAbis)
        abiList.append(a.toString());
    map.insert(QLatin1String(supportedAbisKeyC), abiList);
    return map;
}

bool GoToolChain::fromMap(const QVariantMap &data)
{
    if(!ToolChain::fromMap(data))
        return false;

    m_compilerCommand = FileName::fromString(data.value(QLatin1String(compilerCommandKeyC)).toString());
    m_goRoot = FileName::fromString(data.value(QLatin1String(goRootKeyC)).toString());
    m_targetAbi = Abi::fromString(data.value(QLatin1String(targetAbiKeyC)).toString());
    QStringList abiList = data.value(QLatin1String(supportedAbisKeyC)).toStringList();
    m_supportedAbis.clear();
    foreach (const QString &a, abiList) {
        Abi abi = Abi::fromString(a);
        if (!abi.isValid())
            continue;
        m_supportedAbis.append(abi);
    }
    return true;
}

QVector<Task> GoToolChain::validateKit(const Kit *k) const
{
    QVector<Task> result;

    ToolChain *tc = ToolChainKitAspect::toolChain(k, Constants::C_GOLANGUAGE_ID);
    if(tc) {
        Abi targetAbi = tc->targetAbi();
        bool compMatch = targetAbi.isCompatibleWith(m_targetAbi);
        bool fullMatch = (targetAbi == m_targetAbi);

        QString message;
        if (!fullMatch) {
            if (!compMatch)
                message = QCoreApplication::translate("GoLang::GoToolChain",
                                                      "The compiler '%1' (%2) cannot produce code for the Go version '%3' (%4).");
            else
                message = QCoreApplication::translate("GoLang::GoToolChain",
                                                      "The compiler '%1' (%2) may not produce code compatible with the Go version '%3' (%4).");
            message = message.arg(tc->displayName(), targetAbi.toString(),
                                  displayName(), m_targetAbi.toString());
            result << Task(compMatch ? Task::Warning : Task::Error, message, FileName(), -1,
                           ProjectExplorer::Constants::TASK_CATEGORY_BUILDSYSTEM);
        }
    }
    return result;
}

void GoToolChain::addCommandPathToEnvironment(const FileName &command, Environment &env)
{
    if (!command.isEmpty())
        env.prependOrSetPath(command.parentDir().toString());
}

QString GoToolChain::toString(Abi::Architecture arch, int width)
{
    switch (arch) {
        case Abi::ArmArchitecture:
            return QStringLiteral("arm");
        case Abi::X86Architecture:{
            if(width == 64)
                return QStringLiteral("amd64");
            return QStringLiteral("386");
        }
        case Abi::UnknownArchitecture: // fall through!
        default:
            return QLatin1String("unknown");
    }
}

QVector<Abi> GoToolChain::detectGoAbi(const QString &goRoot)
{
    return guessGoAbi(goRoot);
}

GoToolChain::DetectedAbisResult GoToolChain::detectSupportedAbis() const
{
    Environment env = Environment::systemEnvironment();
    addToEnvironment(env);
    return guessGoAbi(m_compilerCommand, env.toStringList());
}

QString GoToolChain::detectVersion() const
{
    Environment env = Environment::systemEnvironment();
    addToEnvironment(env);
    return goVersion(m_compilerCommand, env.toStringList());
}

QString GoToolChain::detectVersionForPath(const FileName &goPath)
{
    return goVersion(goPath, Environment::systemEnvironment().toStringList());
}

GoToolChainFactory::GoToolChainFactory()
{
    setDisplayName(QStringLiteral("Go Toolchain"));
}

static QList<ToolChain *> getGoToolChains(const FilePath &compilerPath,
                                          const Core::Id &language,
                                          ToolChain::Detection d)
{
    QList<ToolChain *> result;
    if (language == Constants::C_GOLANGUAGE_ID) {
        Environment systemEnvironment = Environment::systemEnvironment();
        QMap<QString, QString> goEnv = getGoEnv(compilerPath,systemEnvironment.toStringList());
        QString rootKey = QStringLiteral("GOROOT");
        if(!goEnv.contains(rootKey))
            return result;

        QVector<Abi> abiList = guessGoAbi(goEnv[rootKey]);
        foreach (const Abi &abi, abiList) {
            auto tc = new GoToolChain();
            tc->setCompilerCommand(compilerPath, FileName::fromString(goEnv[rootKey]));
            tc->setTargetAbi(abi);
            tc->setDetection(d);
            tc->setDisplayName(tc->defaultDisplayName()); // reset displayname
            result.append(tc);
        }
    }
    return result;
}

QList<ToolChain *> GoToolChainFactory::autoDetect(const QList<ToolChain *> &alreadyKnown)
{
    QList<ToolChain *> result;

    QString compiler = QStringLiteral("go");
    Environment systemEnvironment = Environment::systemEnvironment();
    const FileName compilerPath = systemEnvironment.searchInPath(compiler);
    if (compilerPath.isEmpty())
        return result;

    result = Utils::filtered(alreadyKnown, [compilerPath](ToolChain *tc) {
        return tc->typeId() == Constants::GO_TOOLCHAIN_ID
                && tc->compilerCommand() == compilerPath;
    });

    if (!result.empty())
        return result;

    GoToolChain::addCommandPathToEnvironment(compilerPath, systemEnvironment);

    return getGoToolChains(compilerPath, Constants::C_GOLANGUAGE_ID, ToolChain::AutoDetection);
}

QList<ToolChain *> GoToolChainFactory::autoDetect(const FilePath &compilerPath, const Core::Id &language)
{
    return getGoToolChains(compilerPath, language, ToolChain::ManualDetection);
}

}   // namespace GoLang

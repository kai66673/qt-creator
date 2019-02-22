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

#include "godocument.h"
#include "goeditorconstants.h"
#include "gohighlighter.h"
#include "goindenter.h"
#include "gocompletionassist.h"
#include "gopackage.h"
#include "gosettings.h"

#include <coreplugin/messagemanager.h>
#include <texteditor/fontsettings.h>
#include <texteditor/tabsettings.h>
#include <utils/mimetypes/mimedatabase.h>
#include <utils/qtcassert.h>
#include <utils/textfileformat.h>

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QThread>
#include <QTimer>

using namespace TextEditor;
using Utils::TextFileFormat;

namespace GoEditor {
namespace Internal {

enum { processDocumentIntervalInMs = 150 };

GoDocument::GoDocument()
    : TextDocument(Constants::GOEDITOR_ID)
    , m_fileIsBeingReloaded(false)
    , m_cachedContentsRevision(-1)
    , m_processorRevision(0)
    , m_completionAssistProvider(new GoCompletionAssistProvider)
{
    setSyntaxHighlighter(new GoHighlighter);
    setIndenter(new GoIndenter(document()));

    connect(this, &Core::IDocument::mimeTypeChanged,
            this, &GoDocument::onMimeTypeChanged);

    connect(this, &Core::IDocument::aboutToReload,
            this, &GoDocument::onAboutToReload);
    connect(this, &Core::IDocument::reloadFinished,
            this, &GoDocument::onReloadFinished);
    connect(this, &IDocument::filePathChanged,
            this, &GoDocument::onFilePathChanged);

    connect(this, &GoDocument::tabSettingsChanged,
            this, &GoDocument::fixTabSettings);
}

GoDocument::~GoDocument()
{ }

QByteArray GoDocument::contentsText() const
{
    QMutexLocker locker(&m_cachedContentsLock);

    const int currentRevision = document()->revision();
    if (m_cachedContentsRevision != currentRevision && !m_fileIsBeingReloaded) {
        m_cachedContentsRevision = currentRevision;
        m_cachedContents = plainText().toUtf8();
    }

    return m_cachedContents;
}

unsigned GoDocument::contentsRevision() const
{ return document()->revision(); }

CompletionAssistProvider *GoDocument::completionAssistProvider() const
{ return m_completionAssistProvider.data(); }

static const int COMPLETION_WAIT_TIME_MSEC = 3000;
static QLatin1String GOFMT_COMMAND("gofmt");
static QLatin1String GOIMPORTS_COMMAND("goimports");

static bool runGoTool(const QString &filePath, const QString &cmd, const QStringList &args)
{
    QProcess tool;
    tool.setWorkingDirectory(QFileInfo(filePath).dir().absolutePath());

    tool.start(cmd, args);
    if (!tool.waitForFinished(COMPLETION_WAIT_TIME_MSEC)) {
        switch (tool.error()) {
            case QProcess::FailedToStart:
                // Tool is not installed
                Core::MessageManager::write(QString("Starting process \"%1\" failed: tool not installed?").arg(cmd));
                return false;
            case QProcess::Crashed:
                Core::MessageManager::write(QString("Process \"%1\" execution crashed.").arg(cmd));
                return false;
            case QProcess::Timedout:
                Core::MessageManager::write(QString("Process \"%1\" execution failed: waiting timeout exceed.").arg(cmd));
                return false;
            default:
                Core::MessageManager::write(QString("Process \"%1\" execution failed: I/O error.").arg(cmd));
                return false;
        }
    }

    if (tool.exitCode() != QProcess::NormalExit) {
        Core::MessageManager::write(QString("Process \"%1\" exited with code '%2'.\nProcess stderr: %3")
                                    .arg(cmd).arg(tool.exitCode())
                                    .arg(QString::fromUtf8(tool.readAllStandardError())));
        return false;
    }

    tool.closeReadChannel(QProcess::StandardOutput);
    return true;
}

bool GoDocument::save(QString *errorString, const QString &fileName, bool autoSave)
{
    if (!TextDocument::save(errorString, fileName, autoSave))
        return false;

    // Don't format on autosave
    if (autoSave)
        return true;

    const GoLang::GoToolsSettings &toolsSettings = GoLang::GoSettings::toolsSettings();
    bool needReload = false;

    const QString path = filePath().toString();
    if (toolsSettings.goFmtEnabled()) {
        needReload = true;
        runGoTool(path, GOFMT_COMMAND, {QLatin1String("-w=true"), QFileInfo(path).fileName()});
    }

    if (toolsSettings.goImportsEnabled()) {
        needReload = true;
        runGoTool(path, GOIMPORTS_COMMAND, {QLatin1String("-w"), QFileInfo(path).fileName()});
    }

    if (needReload) {
        QString reloadError;
        if (!reloadKeepHistory(reloadError))
            Core::MessageManager::write(QLatin1String("GoEditor: reload formatted code failed. ") + reloadError);
    }

    return true;
}

void GoDocument::findLinkAt(const QTextCursor &tc, Utils::ProcessLinkCallback &&processLinkCallback)
{ return processor()->findLinkAt(tc, std::move(processLinkCallback)); }

QString GoDocument::evaluateIdentifierTypeDescription(int pos)
{ return processor()->evaluateIdentifierTypeDescription(pos); }

void GoDocument::findUsages(int pos)
{ processor()->findUsages(pos); }

void GoDocument::renameSymbolUnderCursor(int pos)
{ processor()->renameSymbolUnderCursor(pos); }

void GoDocument::applyFontSettings()
{
    if (TextEditor::SyntaxHighlighter *highlighter = syntaxHighlighter()) {
        // Clear all additional formats since they may have changed
        QTextBlock b = document()->firstBlock();
        while (b.isValid()) {
            QVector<QTextLayout::FormatRange> noFormats;
            highlighter->setExtraFormats(b, noFormats);
            b = b.next();
        }
    }
    TextDocument::applyFontSettings();
    if (m_processor)
        m_processor->semanticRehighlight();
}

void GoDocument::fixTabSettings()
{
    // Prevent recursive call from signal.
    if (m_isFixingTabSettings)
        return;
    m_isFixingTabSettings = true;
    TabSettings settings = tabSettings();
    settings.m_tabPolicy = TabSettings::TabsOnlyTabPolicy;
    setTabSettings(settings);
    m_isFixingTabSettings = false;
}

bool GoDocument::reloadKeepHistory(QString &errorString)
{
    emit aboutToReload();
    QString text;
    QByteArray errorSample;
    TextFileFormat textFormat;
    ReadResult result = TextFileFormat::readFile(filePath().toString(), codec(), &text, &textFormat, &errorString, &errorSample);
    bool success = false;
    if (result == TextFileFormat::ReadSuccess) {
        success = true;
        QTextCursor cursor(document());
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.insertText(text);
        cursor.endEditBlock();
        document()->setModified(false);
    }
    emit reloadFinished(success);
    return success;
}

void GoDocument::scheduleProcessDocument()
{
    m_processorRevision = document()->revision();
    m_processorTimer.start();
    processor()->editorDocumentTimerRestarted();
}

void GoDocument::scheduleProcessDocumentForceRehighlight()
{
    m_processorRevision = document()->revision();
    m_processorTimer.start();
    processor()->setForceSemanticRehighligting();
    processor()->editorDocumentTimerRestarted();
}

void GoDocument::onGoDocumentUpdated(const GoTools::GoSource::Ptr document)
{
    m_source = document;
    emit goDocumentUpdated(document);
}

void GoDocument::onAboutToReload()
{
    QTC_CHECK(!m_fileIsBeingReloaded);
    m_fileIsBeingReloaded = true;
}

void GoDocument::onReloadFinished()
{
    QTC_CHECK(m_fileIsBeingReloaded);
    m_fileIsBeingReloaded = false;
}

void GoDocument::onFilePathChanged(const Utils::FileName &oldPath, const Utils::FileName &newPath)
{
    Q_UNUSED(oldPath)

    if (!newPath.isEmpty()) {
        setMimeType(Utils::mimeTypeForFile(newPath.toFileInfo()).name());

        disconnect(this, &Core::IDocument::contentsChanged, this, &GoDocument::scheduleProcessDocument);
        connect(this, &Core::IDocument::contentsChanged, this, &GoDocument::scheduleProcessDocument);

        GoTools::GoPackageCache *cache = GoTools::GoPackageCache::instance();
        disconnect(cache, &GoTools::GoPackageCache::packageCacheCleaned,
                   this, &GoDocument::scheduleProcessDocumentForceRehighlight);
        connect(cache, &GoTools::GoPackageCache::packageCacheCleaned,
                this, &GoDocument::scheduleProcessDocumentForceRehighlight);

        // Un-Register/Register in ModelManager
        m_editorDocumentHandle.reset();
        m_editorDocumentHandle.reset(new GoTools::GoEditorDocumentHandle(this));

        resetProcessor();
        m_processorRevision = document()->revision();
        processDocument();
    }
}

void GoDocument::onMimeTypeChanged()
{ initializeTimer(); }

void GoDocument::processDocument()
{
    if (processor()->isParserRunning() || m_processorRevision != contentsRevision()) {
        m_processorTimer.start();
        processor()->editorDocumentTimerRestarted();
        return;
    }

    m_processorTimer.stop();
    if (m_fileIsBeingReloaded || filePath().isEmpty())
        return;

    processor()->run(contentsText(), contentsRevision());
}

GoTools::GoEditorDocumentProcessor *GoDocument::processor()
{
    if (!m_processor) {
        m_processor.reset(new GoTools::GoEditorDocumentProcessor(this));
        GoTools::GoEditorDocumentProcessor *processor = m_processor.data();

        connect(processor, &GoTools::GoEditorDocumentProcessor::codeWarningsUpdated,
                this, &GoDocument::codeWarningsUpdated);
        connect(processor, &GoTools::GoEditorDocumentProcessor::goDocumentUpdated,
                this, &GoDocument::onGoDocumentUpdated);

        GoTools::GoPackageCache *cache = GoTools::GoPackageCache::instance();
        connect(cache, &GoTools::GoPackageCache::packageCacheUpdated,
                processor, &GoTools::GoEditorDocumentProcessor::onPackageCacheUpdated);

        return processor;
    }

    return m_processor.data();
}

void GoDocument::resetProcessor()
{
    releaseResources();
    processor(); // creates a new processor
}

void GoDocument::releaseResources()
{
    if (m_processor)
        disconnect(m_processor.data(), 0, this, 0);
    m_processor.reset();
}

void GoDocument::initializeTimer()
{
    m_processorTimer.setSingleShot(true);
    m_processorTimer.setInterval(processDocumentIntervalInMs);

    connect(&m_processorTimer,
            &QTimer::timeout,
            this,
            &GoDocument::processDocument,
            Qt::UniqueConnection);
}

GoTools::GoSource::Ptr GoDocument::source() const
{
    return m_source;
}

GoTools::GoSource::Ptr GoDocument::actualSource(unsigned revision)
{ return processor()->actualSource(revision); }

} // namespace Internal
} // namespace GoEditor

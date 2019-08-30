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

#include "goeditordocumenthandle.h"
#include "goeditordocumentprocessor.h"

#include <texteditor/textdocument.h>
#include <texteditor/texteditor.h>
#include <utils/fileutils.h>

#include <QFutureWatcher>
#include <QSharedPointer>
#include <QTextFormat>
#include <QTimer>

namespace GoEditor {

namespace Internal {

class GoCompletionAssistProvider;

class GoDocument : public TextEditor::TextDocument
{
    Q_OBJECT

public:
    explicit GoDocument();
    ~GoDocument() override;

    QByteArray contentsText() const;
    unsigned contentsRevision() const;

    TextEditor::CompletionAssistProvider *completionAssistProvider() const override;
    bool save(QString *errorString, const QString &fileName, bool autoSave) override;

    void findLinkAt(const QTextCursor &tc, Utils::ProcessLinkCallback &&processLinkCallback);
    QString evaluateIdentifierTypeDescription(int pos);
    void findUsages(int pos);
    void renameSymbolUnderCursor(int pos);

    GoTools::GoSource::Ptr source() const;
    GoTools::GoSource::Ptr actualSource(unsigned revision);

protected:
    void applyFontSettings() override;


private:
    void fixTabSettings();
    bool reloadKeepHistory(QString &errorString);

    int m_indexRevision = 0;
    QTimer *m_semaHighlightsUpdater = nullptr;
    bool m_isFixingTabSettings = false;

    //////////////////////////////////////
    void scheduleProcessDocument();
    void scheduleProcessDocumentForceRehighlight();

    using HeaderErrorDiagnosticWidgetCreator = std::function<QWidget*()>;

signals:
    void codeWarningsUpdated(unsigned revision,
                             const QList<QTextEdit::ExtraSelection> selections,
                             const HeaderErrorDiagnosticWidgetCreator &creator,
                             const TextEditor::RefactorMarkers &refactorMarkers);
    void goDocumentUpdated(const GoTools::GoSource::Ptr document);

private:
    void onGoDocumentUpdated(const GoTools::GoSource::Ptr document);
    void onAboutToReload();
    void onReloadFinished();
    void onFilePathChanged(const Utils::FileName &oldPath, const Utils::FileName &newPath);
    void onMimeTypeChanged();

    void processDocument();

    GoTools::GoEditorDocumentProcessor *processor();
    void resetProcessor();
    void releaseResources();

    void initializeTimer();

private:
    bool m_fileIsBeingReloaded;

    // Caching contents
    mutable QMutex m_cachedContentsLock;
    mutable QByteArray m_cachedContents;
    mutable int m_cachedContentsRevision;

    unsigned m_processorRevision;
    QTimer m_processorTimer;
    QScopedPointer<GoTools::GoEditorDocumentProcessor> m_processor;

    // (Un)Registration in GoModelManager
    QScopedPointer<GoTools::GoEditorDocumentHandle> m_editorDocumentHandle;

    GoTools::GoSource::Ptr m_source;
    QScopedPointer<GoCompletionAssistProvider> m_completionAssistProvider;
};

} // namespace Internal
} // namespace GoEditor

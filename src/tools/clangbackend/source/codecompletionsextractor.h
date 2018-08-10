/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include <codecompletion.h>

#include <clang-c/Index.h>

#include <QVector>

#include <iosfwd>

namespace ClangBackEnd {

class CodeCompletionsExtractor
{
public:
    CodeCompletionsExtractor(CXTranslationUnit cxTranslationUnit,
                             CXCodeCompleteResults *cxCodeCompleteResults);

    CodeCompletionsExtractor(CodeCompletionsExtractor&) = delete;
    CodeCompletionsExtractor &operator=(CodeCompletionsExtractor&) = delete;

    CodeCompletionsExtractor(CodeCompletionsExtractor&&) = delete;
    CodeCompletionsExtractor &operator=(CodeCompletionsExtractor&&) = delete;

    bool next();
    bool peek(const Utf8String &name);

    CodeCompletions extractAll();

    const CodeCompletion &currentCodeCompletion() const;

private:
    void extractCompletionKind();
    void extractText();
    void extractMethodCompletionKind();
    void extractMacroCompletionKind();
    void extractPriority();
    void extractAvailability();
    void extractHasParameters();
    void extractBriefComment();
    void extractCompletionChunks();
    void extractRequiredFixIts();

    void adaptPriority();
    void decreasePriorityForNonAvailableCompletions();
    void decreasePriorityForDestructors();
    void decreasePriorityForSignals();
    void decreasePriorityForQObjectInternals();
    void decreasePriorityForOperators();

    bool hasText(const Utf8String &text, CXCompletionString cxCompletionString) const;

private:
    CodeCompletion currentCodeCompletion_;
    CXTranslationUnit cxTranslationUnit;
    CXCompletionResult currentCxCodeCompleteResult;
    CXCodeCompleteResults *cxCodeCompleteResults;
    uint cxCodeCompleteResultIndex = 0;
};

std::ostream &operator<<(std::ostream &os, const CodeCompletionsExtractor &extractor);
} // namespace ClangBackEnd

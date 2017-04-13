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

#include "gotoken.h"

#include <texteditor/textdocumentlayout.h>
#include <texteditor/tabsettings.h>

#include <QtCore/QStack>

namespace GoEditor {
namespace Internal {

class GoCodeFormatter
{
public:
    enum State {
        Invalid = 0,
        TopMost,

        PackageClause,      // after "package"

        ConstDecl,          // after "const"
        ConstOpen,          // after "const ("
        IdentDecl,          // after "identifier"
        IdentArgs,          // after "identifier("
        IdentFields,        // after "identifier{"
        ImportDecl,         // after "import"
        ImportOpen,         // after "import ("
        FuncDecl,           // after "func"
        FuncOpen,           // after "func {"
        FuncParams,         // after "func ... ("
        InterfaceDecl,      // after "interface"
        InterfaceOpen,      // after "interface {"
        StructDecl,         // after "struct"
        StructOpen,         // after "struct {"
        TypeDecl,           // after "type"
        TypeOpen,           // after "type ("
        VarDecl,            // after "var"
        VarOpen,            // after "var ("
        VarExpr,            // after "identifier ="

        ForStmt,            // after "for"
        ForOpen,            // after "for {"
        IfStmt,             // after "if"
        IfOpen,             // after "if {"
        SwitchStmt,         // after "switch" or "select"
        SwitchOpen,         // after "switch {" or "select {"
        CaseClause,         // after "case" or "default"
        CaseOpen,           // after "case ... :" or "default ... :"

        BlockOpen,          // after "{" outside of other statements
        GroupOpen,          // after "(" outside of other statements

        Label,              // "Label:"

        MultilineCommentStart,  // after "/*"
        MultilineCommentCont,

    };
    GoCodeFormatter(const TextEditor::TabSettings &tabSettings);

    void updateStateUntil(const QTextBlock &endBlock);
    void calcIndentation(const QTextBlock &block, int *indentation, int *padding, bool withoutLexer = false);

private:
    struct CodeState {
        CodeState(quint8 _state = 0,
                  quint16 _indentation = 0,
                  qint16 _padding = 0) :
            state(_state),
            indentation(_indentation),
            padding(_padding)
        {}
        bool operator==(const CodeState &other) const {
            return state == other.state
                && indentation == other.indentation
                && padding == other.padding;
        }

        quint8 state;
        quint16 indentation;
        quint16 padding;
    };

    struct BlockData
    {
    public:
        BlockData() :
            revision(-1)
        {}

        QStack<CodeState> beginState;
        QStack<CodeState> endState;
        int indentation;
        int padding;
        int revision;
    };

    class GoCodeFormatterData: public TextEditor::CodeFormatterData
    {
    public:
        const BlockData &blockData() const { return m_blockData; }
        void setBlockData(const BlockData &blockData) {m_blockData = blockData;}
    private:
        BlockData m_blockData;
    };

private:
    int column(int position) const;

    CodeState state(int belowTop = 0) const;
    QStack<CodeState> initialState();
    void restoreCurrentState(const QTextBlock &block);
    void saveCurrentState(const QTextBlock &block);
    void recalculateStateAfter(const QTextBlock &block);

    int tokenizeBlock(const QTextBlock &block);

    bool loadBlockData(const QTextBlock &block, BlockData *data) const;
    void saveBlockData(QTextBlock *block, const BlockData &data) const;

    bool tryDeclaration();
    bool tryStatement();

    void enter(int newState);
    void leave();
    void adjustIndentation(const QTextBlock &block, bool withoutLexer = false);

private:
    TextEditor::TabSettings m_tabSettings;

    QStack<CodeState> m_beginState;
    QStack<CodeState> m_currentState;
    QStack<CodeState> m_newStates;

    int m_indentation;
    int m_padding;

    int m_index;
    QString m_line;
    QList<GoToken> m_tokens;
    GoToken m_tk;
};

} // namespace Internal
} // namespace GoEditor

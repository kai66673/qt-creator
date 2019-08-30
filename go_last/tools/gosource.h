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

#include "gopackagetypes.h"
#include "control.h"
#include "translationunit.h"

#include <texteditor/codeassist/assistproposaliteminterface.h>
#include <coreplugin/find/searchresultitem.h>

#include <QString>
#include <QSharedPointer>

#include <set>

namespace GoTools {

class GoPackage;

class GoSource
{
    GoSource(const GoSource &other);
    void operator = (const GoSource &other);
    GoSource(const QString &fileName);

public:
    typedef QSharedPointer<GoSource> Ptr;
    static Ptr create(const QString &fileName);

public:
    virtual ~GoSource();

    class DiagnosticMessage
    {
    public:
        enum Level {
            Warning,
            Error,
            Fatal
        };

    public:
        DiagnosticMessage(int level, const QString &fileName,
                          unsigned line, unsigned column,
                          const QString &text,
                          unsigned length = 0)
            : _level(level),
              _line(line),
              _fileName(fileName),
              _column(column),
              _length(length),
              _text(text)
        { }

        int level() const
        { return _level; }

        bool isWarning() const
        { return _level == Warning; }

        bool isError() const
        { return _level == Error; }

        bool isFatal() const
        { return _level == Fatal; }

        QString fileName() const
        { return _fileName; }

        unsigned line() const
        { return _line; }

        unsigned column() const
        { return _column; }

        unsigned length() const
        { return _length; }

        QString text() const
        { return _text; }

        bool operator==(const DiagnosticMessage &other) const;
        bool operator!=(const DiagnosticMessage &other) const;

    private:
        int _level;
        unsigned _line;
        QString _fileName;
        unsigned _column;
        unsigned _length;
        QString _text;
    };

    struct Import {
        QString dir;
        QString resolvedDir;
        QString packageName;
        QString alias;
        bool aliasSpecified;
        unsigned tokenIndex;
    };

    void addDiagnosticMessage(const DiagnosticMessage &d)
    { _diagnosticMessages.append(d); }

    QList<DiagnosticMessage> diagnosticMessages() const
    { return _diagnosticMessages; }

    QString fileName() const;

    QByteArray source() const;
    void setSource(const QByteArray &source);

    void parse(TranslationUnit::ParseMode mode);
    bool parsePackageFile(const QString &packageName);
    void resolveImportsAndPackageName(const QString &goRoot, const QString &goPath);

    TranslationUnit *translationUnit() const;

    int revision() const;
    void setRevision(int revision);

    GoPackageKeySet importTasks() const;
    QString packageName() const;
    QList<Import> imports() const;
    QString location() const;

    bool isTooBig() const;
    Core::SearchResultItem searchResultItemForTokenIndex(unsigned tokenIndex, unsigned length);

    GoPackage *package() const;
    void setPackage(GoPackage *package);

private:
    QString _fileName;
    QString _packageName;
    QString _location;
    Control *_control;
    TranslationUnit *_translationUnit;
    QByteArray _source;
    int _revision;
    QList<DiagnosticMessage> _diagnosticMessages;
    QList<Import> _imports;
    std::set<QString> _resolvedImportDirs;
    GoPackageKeySet _importTasks;

    GoPackage *_package;
};

}   // namespace GoTools

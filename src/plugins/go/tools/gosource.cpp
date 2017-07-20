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
#include "gosource.h"
#include "literals.h"
#include "ast.h"
#include "astvisitor.h"

#include <texteditor/codeassist/assistproposalitem.h>

#include <QDir>

#include <functional>

namespace GoTools {

using ImportSpecCallback = std::function<void (ImportSpecAST *)>;

QString C_UNDERSCORE(QStringLiteral("_"));

class ImportDeclarationsVisitor: protected ASTVisitor
{
public:
    ImportDeclarationsVisitor(TranslationUnit *unit, ImportSpecCallback callback)
        : ASTVisitor(unit)
        , m_callback(callback)
    { }

    void operator () (DeclListAST *ast) {
        accept(ast);
    }

protected:
    bool visit(ImportSpecAST *ast) {
        m_callback(ast);
        return false;
    }

private:
    ImportSpecCallback m_callback;
};

class DocumentDiagnosticClient : public DiagnosticClient
{
    enum { MAX_MESSAGE_COUNT = 20 };

public:
    DocumentDiagnosticClient(GoSource *doc, QList<GoSource::DiagnosticMessage> *messages_)
        : doc(doc)
        , messages(messages_)
        , errorCount(0)
    { }

    virtual void report(int level,
                        const char * /*fileId*/,
                        unsigned line, unsigned column, unsigned length,
                        const char *format, va_list ap)
    {
        if (level == Error) {
            ++errorCount;

            if (errorCount >= MAX_MESSAGE_COUNT)
                return; // ignore the error
        }

        QString message;
        message.vsprintf(format, ap);

        GoSource::DiagnosticMessage m(convertLevel(level), doc->fileName(),
                                      line, column, message, length);
        messages->append(m);
    }

    virtual void reportString(int level,
                              const char * /*fileId*/,
                              unsigned line, unsigned column, unsigned length,
                              const QString &message)
    {
        if (level == Error) {
            ++errorCount;

            if (errorCount >= MAX_MESSAGE_COUNT)
                return; // ignore the error
        }

        GoSource::DiagnosticMessage m(convertLevel(level), doc->fileName(),
                                      line, column, message, length);
        messages->append(m);
    }

    static int convertLevel(int level) {
        switch (level) {
            case Warning: return GoSource::DiagnosticMessage::Warning;
            case Error:   return GoSource::DiagnosticMessage::Error;
            case Fatal:   return GoSource::DiagnosticMessage::Fatal;
            default:      return GoSource::DiagnosticMessage::Error;
        }
    }

private:
    GoSource *doc;
    QList<GoSource::DiagnosticMessage> *messages;
    int errorCount;
};

GoSource::GoSource(const QString &fileName)
    : _fileName(fileName)
    , _revision(0)
    , _package(0)
{
    _control = new Control();
    _control->setDiagnosticClient(new DocumentDiagnosticClient(this, &_diagnosticMessages));

    _translationUnit = new TranslationUnit(_control, fileName);
    (void) _control->switchTranslationUnit(_translationUnit);
}

GoSource::Ptr GoSource::create(const QString &fileName)
{
    GoSource::Ptr doc(new GoSource(fileName));
    return doc;
}

GoSource::~GoSource()
{
    delete _translationUnit;
    delete _control;
}

QString GoSource::fileName() const
{ return _fileName; }

QByteArray GoSource::source() const
{ return _source; }

void GoSource::setSource(const QByteArray &source)
{
    _source = source;
    _translationUnit->setSource(_source.constBegin(), _source.size());
}

void GoSource::parse(TranslationUnit::ParseMode mode)
{ _translationUnit->parse(this, mode); }

bool GoSource::parsePackageFile(const QString &packageName)
{ return _translationUnit->parsePackageFile(this, packageName); }

void GoSource::resolveImportsAndPackageName(const QString &goRoot, const QString &goPath)
{
    _imports.clear();
    _resolvedImportDirs.clear();
    _importTasks.clear();
    _packageName.clear();

    QFileInfo fi(_fileName);
    _location = fi.absolutePath();

    QStringList paths = { goPath, goRoot };

    if (FileAST *ast = _translationUnit->fileAst()) {
        if (ast->packageName)
            _packageName = ast->packageName->ident->toString();
        ImportDeclarationsVisitor visitor(_translationUnit, [&](ImportSpecAST *importSpec) {
            GoSource::Import import;
            if (importSpec->t_path) {
                import.tokenIndex = importSpec->t_path;
                import.dir = _translationUnit->tokenAt(importSpec->t_path).string->unquoted();
                if (!import.dir.isEmpty()) {
                    int pos = import.dir.lastIndexOf('/') + 1;
                    import.packageName = import.dir.right(import.dir.length() - pos);
                    import.aliasSpecified = false;
                    if (importSpec->name) {
                        import.alias = importSpec->name->ident->toString();
                        if (import.alias != C_UNDERSCORE)
                            import.aliasSpecified = true;
                    } else {
                        import.alias = import.packageName;
                    }
                    _imports.append(import);
                }
            }
        });
        visitor(ast->importDecls);
    }

    std::set<QString> aliases;
    for (Import &import: _imports) {
        const Token &tk = _translationUnit->tokenAt(import.tokenIndex);

        bool resolved = false;
        for (const QString &path: paths) {
            QString potentialDir = path + QStringLiteral("/src/") + import.dir;
            QDir dir(potentialDir);
            if (dir.exists()) {
                import.resolvedDir = potentialDir;
                std::pair<std::set<QString>::iterator, bool> insertResult = _resolvedImportDirs.insert(potentialDir);
                if (!insertResult.second)
                    addDiagnosticMessage(DiagnosticMessage(DiagnosticMessage::Warning, QString(),
                                                           tk.kindAndPos.line, tk.kindAndPos.column,
                                                           QStringLiteral("Import directory duplicated"),
                                                           tk.length()));
                else
                    _importTasks.insert(qMakePair(potentialDir, import.packageName));
                resolved = true;
                break;
            }
        }

        if (!resolved)
            addDiagnosticMessage(DiagnosticMessage(DiagnosticMessage::Warning, QString(),
                                                   tk.kindAndPos.line, tk.kindAndPos.column,
                                                   QStringLiteral("Unresolved import directory"),
                                                   tk.length()));

        // Check aliases duplication
        if (import.aliasSpecified) {
            std::pair<std::set<QString>::iterator, bool> insertAliasResult = aliases.insert(import.alias);
            if (!insertAliasResult.second) {
                const Token &aliasTk = _translationUnit->tokenAt(import.tokenIndex - 1);
                addDiagnosticMessage(DiagnosticMessage(DiagnosticMessage::Warning, QString(),
                                                       aliasTk.kindAndPos.line, aliasTk.kindAndPos.column,
                                                       QStringLiteral("Import alias duplicated"),
                                                       aliasTk.length()));
            }
        }
    }
}

TranslationUnit *GoSource::translationUnit() const
{ return _translationUnit; }

int GoSource::revision() const
{ return _revision; }

void GoSource::setRevision(int revision)
{ _revision = revision; }

GoPackageKeySet GoSource::importTasks() const
{ return _importTasks; }

QString GoSource::packageName() const
{ return _packageName; }

QList<GoSource::Import> GoSource::imports() const
{ return _imports; }

QString GoSource::location() const
{ return _location; }

bool GoSource::isTooBig() const
{ return _source.size() >= 1024 * 200; }

Core::SearchResultItem GoSource::searchResultItemForTokenIndex(unsigned tokenIndex, unsigned length)
{
    Core::SearchResultItem item;
    item.path = QStringList() << QDir::toNativeSeparators(_fileName);
    const Token &tk = _translationUnit->tokenAt(tokenIndex);
    unsigned position = tk.begin();
    int lineBegin = _source.lastIndexOf('\n', position) + 1;
    int lineEnd = _source.indexOf('\n', position);
    if (lineEnd == -1)
        lineEnd = _source.length();
    const QByteArray matchingLine = _source.mid(lineBegin, lineEnd - lineBegin);
    item.text = QString::fromUtf8(matchingLine, matchingLine.size());
    item.mainRange.begin.line = tk.line();
    item.mainRange.begin.column = tk.column() - 1;
    item.mainRange.end.line = tk.line();
    item.mainRange.end.column = tk.column() + length - 1;
    item.useTextEditorFont = true;
    return item;
}

GoPackage *GoSource::package() const
{ return _package; }

void GoSource::setPackage(GoPackage *package)
{ _package = package; }

}   // namespace GoTools

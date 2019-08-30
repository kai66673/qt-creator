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
#include "gooverviewmodel.h"
#include "goiconprovider.h"
#include "ast.h"
#include "astvisitor.h"

namespace GoTools {

class OverviewVisitor: protected ASTVisitor
{
public:
    OverviewVisitor(TranslationUnit *unit)
        : ASTVisitor(unit)
    { }

    void operator()(FileAST *ast, GoOverviewTreeItem *parentItem) {
        m_parentItem = parentItem;
        accept(ast->decls);
    }

protected:
    bool visit(BadDeclAST *) { return false; }

    bool visit(VarSpecWithTypeAST *ast) {
        if (ast->names) {
            int startTokenIndex = ast->names->firstToken();
            const Token &tk = translationUnit()->tokenAt(startTokenIndex);
            QString descr = "";
            for(DeclIdentListAST *it = ast->names; it; it = it->next) {
                if (DeclIdentAST *ident = it->value) {
                    descr += ident->ident->toString() + ",";
                }
            }
            descr.chop(1);
            type(ast->type, descr, ast->comment ? ast->comment : ast->doc, tk.begin(), GoOverviewTreeItem::VariableDeclaration);
        }

        return false;
    }

    bool visit(VarSpecWithValuesAST *ast) {
        if (ast->names) {
            int startTokenIndex = ast->names->firstToken();
            const Token &tk = translationUnit()->tokenAt(startTokenIndex);
            QString descr = "";
            for(DeclIdentListAST *it = ast->names; it; it = it->next) {
                if (DeclIdentAST *ident = it->value) {
                    descr += ident->ident->toString() + ",";
                }
            }
            descr.chop(1);
            m_parentItem->appandChild(new GoOverviewTreeItem(descr, extractDoc(ast->comment? ast->comment : ast->doc),
                                                             tk.begin(), GoOverviewTreeItem::VariableDeclaration, m_parentItem));
        }

        return false;
    }

    bool visit(ConstSpecAST *ast) {
        if (ast->names) {
            int startTokenIndex = ast->names->firstToken();
            const Token &tk = translationUnit()->tokenAt(startTokenIndex);
            QString descr = "";
            for(DeclIdentListAST *it = ast->names; it; it = it->next) {
                if (DeclIdentAST *ident = it->value) {
                    descr += ident->ident->toString() + ",";
                }
            }
            descr.chop(1);
            type(ast->type, descr, ast->comment ? ast->comment : ast->doc, tk.begin(), GoOverviewTreeItem::ConstantDeclaration);
        }

        return false;
    }

    bool visit(TypeSpecAST *ast) {
        if (ast->name) {
            int startTokenIndex = ast->name->t_identifier;
            const Token &tk = translationUnit()->tokenAt(startTokenIndex);
            QString descr = ast->name->ident->toString();
            type(ast->type, descr, ast->comment ? ast->comment : ast->doc, tk.begin(), GoOverviewTreeItem::TypeDeclaration);
        }

        return false;
    }

    bool visit(FieldAST *ast) {
        if (ast->names) {
            int startTokenIndex = ast->names->firstToken();
            const Token &tk = translationUnit()->tokenAt(startTokenIndex);
            QString descr = "";
            for(DeclIdentListAST *it = ast->names; it; it = it->next) {
                if (DeclIdentAST *ident = it->value) {
                    descr += ident->ident->toString() + ",";
                }
            }
            descr.chop(1);
            type(ast->type, descr, ast->comment ? ast->comment : ast->doc, tk.begin(), GoOverviewTreeItem::VariableDeclaration);
        }
        return false;
    }

    bool visit(FuncDeclAST *ast) {
        if (ast->name) {
            int startTokenIndex = ast->name->t_identifier;
            QString descr = "";
            if (ast->recv) {
                descr += "(";
                if (ast->recv->fields)
                    descr += methodRecv(ast->recv->fields->value->type);
                descr += ")";
            }
            descr += ast->name->ident->toString();
            const Token &tk = translationUnit()->tokenAt(startTokenIndex);
            if (ast->type) {
                descr += ast->type->params ? ast->type->params->describe() : QStringLiteral("()");
                if (ast->type->results)
                    descr += ":" + ast->type->results->describe();
            }
            m_parentItem->appandChild(new GoOverviewTreeItem(descr, extractDoc(ast->doc), tk.begin(), GoOverviewTreeItem::FunctionDeclaration, m_parentItem));
        }
        return false;
    }

    bool visit(SelectorExprAST *) { return false; }
    bool visit(StarTypeAST *) { return false; }
    bool visit(ParenExprAST *) { return false; }

    bool visit(ArrayTypeAST *) { return false; }
    bool visit(ChanTypeAST *) { return false; }
    bool visit(FuncTypeAST *) { return false; }
    bool visit(MapTypeAST *) { return false; }
    bool visit(IdentAST *) { return false; }

private:
    QString extractDoc(CommentGroupAST *doc) {
        if (!doc)
            return QString();
        QStringList result;
        for (CommentGroupAST *it = doc; it; it = it->next) {
            if (CommentAST *comment = it->value)
                result << translationUnit()->tokenAt(comment->t_comment).comment->body();
        }
        return result.join(QChar('\n'));
    }

    void type(TypeAST *typ, const QString &descr, CommentGroupAST *doc, unsigned pos, GoOverviewTreeItem::Kind kind) {
        QString typeDescr;
        if (typ)
            typeDescr = QStringLiteral(":") + typ->describe();
        GoOverviewTreeItem *newItem = new GoOverviewTreeItem(descr + typeDescr, extractDoc(doc), pos, kind, m_parentItem);
        m_parentItem->appandChild(newItem);
        if (typ) {
            if (StructTypeAST *structType = typ->asStructType()) {
                GoOverviewTreeItem *oldItem = m_parentItem;
                m_parentItem = newItem;
                accept(structType->fields);
                m_parentItem = oldItem;
            }
        }
    }

    QString methodRecv(ExprAST *type) {
        if (type) {
            if (StarTypeAST *s = type->asStarType())
                return QStringLiteral("*") + methodRecv(s->typ);
            if (IdentAST *ident = type->asIdent())
                return ident->ident->toString();
        }

        return QString();
    }

    GoOverviewTreeItem *m_parentItem;
};

GoOverviewTreeItem::GoOverviewTreeItem(const QString &name, int position, GoOverviewTreeItem::Kind kind, GoOverviewTreeItem *parent)
    : m_parent(parent)
    , m_name(name)
    , m_position(position)
    , m_kind(kind)
{ }

GoOverviewTreeItem::GoOverviewTreeItem(const QString &name, const QString &toolTip, int position, GoOverviewTreeItem::Kind kind, GoOverviewTreeItem *parent)
    : m_parent(parent)
    , m_name(name)
    , m_toolTip(toolTip)
    , m_position(position)
    , m_kind(kind)
{ }

GoOverviewTreeItem::~GoOverviewTreeItem()
{ qDeleteAll(m_childrens); }

GoOverviewTreeItem *GoOverviewTreeItem::parent() const
{ return m_parent; }

QString GoOverviewTreeItem::name() const
{ return m_name; }

QString GoOverviewTreeItem::toolTip() const
{ return m_toolTip; }

GoOverviewTreeItem::Kind GoOverviewTreeItem::kind() const
{ return m_kind; }

int GoOverviewTreeItem::position() const
{ return m_position; }

int GoOverviewTreeItem::childCount() const
{ return m_childrens.size(); }

GoOverviewTreeItem *GoOverviewTreeItem::childAt(int index)
{ return m_childrens[index]; }

void GoOverviewTreeItem::appandChild(GoOverviewTreeItem *child)
{ m_childrens.append(child); }

int GoOverviewTreeItem::row() const
{
    if (m_parent)
        return m_parent->m_childrens.indexOf(const_cast<GoOverviewTreeItem *>(this));
    return 0;
}

GoOverviewModel::GoOverviewModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_revision(0)
{
    m_rootItem = new GoOverviewTreeItem(QStringLiteral("<ROOT>"), 0, GoOverviewTreeItem::Root, 0);
    m_rootItem->appandChild(new GoOverviewTreeItem(QStringLiteral(" <Select Item>"), 0, GoOverviewTreeItem::SelectHint, m_rootItem));
}

GoOverviewModel::~GoOverviewModel()
{ delete m_rootItem; }

QVariant GoOverviewModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
{ return QVariant(); }

QModelIndex GoOverviewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, m_rootItem->childAt(row));

    GoOverviewTreeItem *parentItem = static_cast<GoOverviewTreeItem *>(parent.internalPointer());
    return createIndex(row, column, parentItem->childAt(row));
}

QModelIndex GoOverviewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    GoOverviewTreeItem *item = static_cast<GoOverviewTreeItem *>(index.internalPointer());
    GoOverviewTreeItem *parentItem = item->parent();

    if (parentItem == m_rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int GoOverviewModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_rootItem->childCount();

    GoOverviewTreeItem *parentItem = static_cast<GoOverviewTreeItem *>(parent.internalPointer());
    return parentItem->childCount();
}

int GoOverviewModel::columnCount(const QModelIndex & /*parent*/) const
{ return 1; }

QVariant GoOverviewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    GoOverviewTreeItem *item = static_cast<GoOverviewTreeItem *>(index.internalPointer());

    switch (role) {
        case Qt::DisplayRole:
            return item->name();

        case Qt::DecorationRole: {
            switch (item->kind()) {
                case GoOverviewTreeItem::ConstantDeclaration:
                    return Go::GoIconProvider::instance()->icon(Go::GoIconProvider::Const);
                case GoOverviewTreeItem::TypeDeclaration:
                    return Go::GoIconProvider::instance()->icon(Go::GoIconProvider::Type);
                case GoOverviewTreeItem::VariableDeclaration:
                    return Go::GoIconProvider::instance()->icon(Go::GoIconProvider::Variable);
                case GoOverviewTreeItem::FunctionDeclaration:
                    return Go::GoIconProvider::instance()->icon(Go::GoIconProvider::Func);
                default:
                    return QVariant();
            }
        }

        case Qt::ToolTipRole:
            return item->toolTip();
    }

    return QVariant();
}

void GoOverviewModel::rebuild(GoSource::Ptr doc)
{
    m_revision = doc->revision();

    beginResetModel();

    delete m_rootItem;
    m_rootItem = new GoOverviewTreeItem(QStringLiteral("<ROOT>"), 0, GoOverviewTreeItem::Root, 0);
    m_rootItem->appandChild(new GoOverviewTreeItem(QStringLiteral(" <Select Item>"), 0, GoOverviewTreeItem::SelectHint, m_rootItem));

    if (doc) {
        if (TranslationUnit *unit = doc->translationUnit()) {
            if (FileAST *ast = unit->fileAst()) {
                OverviewVisitor visitor(unit);
                visitor(ast, m_rootItem);
            }
        }
    }

    endResetModel();
}

GoOverviewTreeItem *GoOverviewModel::itemFromIndex(const QModelIndex &index)
{
    if (!index.isValid())
        return 0;

    return static_cast<GoOverviewTreeItem *>(index.internalPointer());
}

unsigned GoOverviewModel::revision() const
{ return m_revision; }

}   // namespace GoTools

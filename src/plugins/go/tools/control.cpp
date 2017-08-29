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
#include "control.h"
#include "translationunit.h"
#include "literaltable.h"
#include "scope.h"
#include "types.h"
#include "ast.h"

#include <QList>

namespace GoTools {

static Identifier s_underscoreIdentifier("_", 1);
static Identifier s_dotIdentifier(".", 1);

// Builting types
static Identifier s_intIdentifier("!int", 4);
static Identifier s_boolIdentifier("!bool", 5);
static Identifier s_byteIdentifier("!byte", 5);
static Identifier s_int8Identifier("!int8", 5);
static Identifier s_runeIdentifier("!rune", 5);
static Identifier s_uintIdentifier("!uint", 5);
static Identifier s_errorIdentifier("!error", 6);
static Identifier s_int16Identifier("!int16", 6);
static Identifier s_int32Identifier("!int32", 6);
static Identifier s_int64Identifier("!int64", 6);
static Identifier s_uint8Identifier("!uint8", 6);
static Identifier s_uint16Identifier("!uint16", 7);
static Identifier s_uint32Identifier("!uint32", 7);
static Identifier s_uint64Identifier("!uint64", 7);
static Identifier s_stringIdentifier("!string", 7);
static Identifier s_float32Identifier("!float32", 8);
static Identifier s_float64Identifier("!float64", 8);
static Identifier s_uintptrIdentifier("!uintptr", 8);
static Identifier s_complex64Identifier("!complex64", 10);
static Identifier s_complex128Identifier("!complex128", 11);

static Identifier s_newIdentifier("!0", 2);
static Identifier s_makeIdentifier("!1", 2);

static ErrorType s_errorBuiltinType;
static VoidType s_voidType;
static UnresolvedType s_unresolvedType;
static UnresolvedTupleType s_unresolvedTupleType;
static IntBuiltinType s_intBuiltinType;
static IntegralBuiltinType s_integralBuiltinType;
static StringBuiltinType s_stringBuiltinType;

static ErrorErrorMethod s_errorErrorMethod;

class Control::Data
{
public:
    Data(Control *control)
        : control(control)
        , translationUnit(0)
        , diagnosticClient(0)
    { }

    ~Data() {
        qDeleteAll(comments);
        qDeleteAll(strings);
        qDeleteAll(identifiers);
        qDeleteAll(scopes);
        qDeleteAll(methods);
        qDeleteAll(functions);
        qDeleteAll(variables);
        qDeleteAll(fields);
        qDeleteAll(types);
        qDeleteAll(constants);
        qDeleteAll(shortVarDecls);
        qDeleteAll(rangeKeys);
        qDeleteAll(rangeValues);
    }

    Control *control;
    TranslationUnit *translationUnit;
    DiagnosticClient *diagnosticClient;

    QList<Comment *> comments;
    QList<StringLiteral *> strings;
    QList<Identifier *> identifiers;
    QList<Scope *> scopes;
    QList<MethodDecl *> methods;
    QList<FuncDecl *> functions;
    QList<VarDecl *> variables;
    QList<FieldDecl *> fields;
    QList<TypeDecl *> types;
    QList<ConstDecl *> constants;
    QList<ShortVarDecl *> shortVarDecls;
    QList<RangeKeyDecl *> rangeKeys;
    QList<RangeValueDecl *> rangeValues;
};

DiagnosticClient *Control::diagnosticClient() const
{ return d->diagnosticClient; }

void Control::setDiagnosticClient(DiagnosticClient *diagnosticClient)
{ d->diagnosticClient = diagnosticClient; }

TranslationUnit *Control::translationUnit() const
{ return d->translationUnit; }

TranslationUnit *Control::switchTranslationUnit(TranslationUnit *unit)
{
    TranslationUnit *previousTranslationUnit = d->translationUnit;
    d->translationUnit = unit;
    return previousTranslationUnit;
}

const Comment *Control::comment(const char *chars, unsigned size)
{
    Comment *comment = new Comment(chars, size);
    d->comments.push_back(comment);
    return comment;
}

const StringLiteral *Control::string(const char *chars, unsigned size)
{
    StringLiteral *str = new StringLiteral(chars, size);
    d->strings.push_back(str);
    return str;
}

const Identifier *Control::identifier(const char *chars, unsigned size)
{
    if (size == 1 && *chars == '_')
        return &s_underscoreIdentifier;

    Identifier *ident = new Identifier(chars, size);
    d->identifiers.append(ident);
    return ident;
}

Scope *Control::newScope(Scope *outer)
{
    Scope *scope = new Scope(outer);
    d->scopes.append(scope);
    return scope;
}

Scope *Control::newStructScope(Scope *outer)
{
    Scope *scope = new StructScope(outer);
    d->scopes.append(scope);
    return scope;
}

FileScope *Control::newFileScope(GoSource *source)
{
    FileScope *scope = new FileScope(source);
    d->scopes.append(scope);
    return scope;
}

Symbol *Control::newMethod(TypeIdentAST *recv, FuncDeclAST *funcDecl, Scope *owner)
{
    MethodDecl *symbol = new MethodDecl(funcDecl->name->firstToken(), funcDecl->name->ident,
                                        recv, funcDecl->type, owner);
    d->methods.append(symbol);
    return symbol;
}

Symbol *Control::newFuncDecl(unsigned tokenIndex, const Identifier *identifier, FuncTypeAST *decl, Scope *owner)
{
    FuncDecl *symbol = new FuncDecl(tokenIndex, identifier, decl, owner);
    d->functions.append(symbol);
    return symbol;
}

Symbol *Control::newVarDecl(unsigned tokenIndex, const Identifier *identifier, TypeAST *decl, Scope *owner)
{
    VarDecl *symbol  = new VarDecl(tokenIndex, identifier, decl, owner);
    d->variables.append(symbol);
    return symbol;
}

Symbol *Control::newFieldDecl(unsigned tokenIndex, const Identifier *identifier, TypeAST *decl, Scope *owner)
{
    FieldDecl *symbol = new FieldDecl(tokenIndex, identifier, decl, owner);
    d->fields.append(symbol);
    return symbol;
}

Symbol *Control::newTypeDecl(unsigned tokenIndex, const Identifier *identifier, TypeSpecAST *decl, Scope *owner)
{
    TypeDecl *symbol = new TypeDecl(tokenIndex, identifier, decl, owner);
    d->types.append(symbol);
    return symbol;
}

Symbol *Control::newConstDecl(unsigned tokenIndex, const Identifier *identifier, ExprAST *value , Scope *owner)
{
    ConstDecl *symbol = new ConstDecl(tokenIndex, identifier, value, owner);
    d->constants.append(symbol);
    return symbol;
}

Symbol *Control::newShortVarDecl(unsigned tokenIndex, const Identifier *identifier, RhsExprListAST *decl, unsigned index, Scope *owner)
{
    ShortVarDecl *symbol = new ShortVarDecl(tokenIndex, identifier, decl, index, owner);
    d->shortVarDecls.append(symbol);
    return symbol;
}

Symbol *Control::newRangeKeyDecl(unsigned tokenIndex, const Identifier *identifier, RangeExpAST *decl, Scope *owner)
{
    RangeKeyDecl *symbol = new RangeKeyDecl(tokenIndex, identifier, decl, owner);
    d->rangeKeys.append(symbol);
    return symbol;
}

Symbol *Control::newRangeValueDecl(unsigned tokenIndex, const Identifier *identifier, RangeExpAST *decl, Scope *owner)
{
    RangeValueDecl *symbol = new RangeValueDecl(tokenIndex, identifier, decl, owner);
    d->rangeValues.append(symbol);
    return symbol;
}

const Identifier *Control::dotIdentifier()
{ return &s_dotIdentifier; }

const Identifier *Control::underscoreIdentifier()
{ return &s_underscoreIdentifier; }

const Identifier *Control::builtinErrorIdentifier()
{ return &s_errorIdentifier; }

const Identifier *Control::builtinIntIdentifier()
{ return &s_intIdentifier; }

const Identifier *Control::builtinByteIdentifier()
{ return &s_byteIdentifier; }

const Identifier *Control::builtinInt8Identifier()
{ return &s_int8Identifier; }

const Identifier *Control::builtinRuneIdentifier()
{ return &s_runeIdentifier; }

const Identifier *Control::builtinUintIdentifier()
{ return &s_uintIdentifier; }

const Identifier *Control::builtinInt16Identifier()
{ return &s_int16Identifier; }

const Identifier *Control::builtinInt32Identifier()
{ return &s_int32Identifier; }

const Identifier *Control::builtinInt64Identifier()
{ return &s_int64Identifier; }

const Identifier *Control::builtinUint8Identifier()
{ return &s_uint8Identifier; }

const Identifier *Control::builtinStringIdentifier()
{ return &s_stringIdentifier; }

const Identifier *Control::builtinUint16Identifier()
{ return &s_uint16Identifier; }

const Identifier *Control::builtinUint32Identifier()
{ return &s_uint32Identifier; }

const Identifier *Control::builtinUint64Identifier()
{ return &s_uint64Identifier; }

const Identifier *Control::builtinFloat32Identifier()
{ return &s_float32Identifier; }

const Identifier *Control::builtinFloat64Identifier()
{ return &s_float64Identifier; }

const Identifier *Control::builtinUintptrIdentifier()
{ return &s_uintptrIdentifier; }

const Identifier *Control::builtinComplex64Identifier()
{ return &s_complex64Identifier; }

const Identifier *Control::builtinComplex128Identifier()
{ return &s_complex128Identifier; }

Type *Control::errorBuiltinType()
{ return &s_errorBuiltinType; }

Type *Control::voidType()
{ return &s_voidType; }

Type *Control::unresolvedType()
{ return &s_unresolvedType; }

Type *Control::unresolvedTupleType()
{ return &s_unresolvedTupleType; }

Type *Control::intBuiltinType()
{ return &s_intBuiltinType; }

Type *Control::integralBuiltinType()
{ return &s_integralBuiltinType; }

Type *Control::stringBuiltingType()
{ return &s_stringBuiltinType; }

const Identifier *Control::newIdentifier()
{ return &s_newIdentifier; }

const Identifier *Control::makeIdentifier()
{ return &s_makeIdentifier; }

Symbol *Control::errorErrorMethod()
{ return &s_errorErrorMethod; }

Control::Control()
    : d(new Data(this))
{ }

Control::~Control()
{
    delete d;
}

}   // namespace GoTools

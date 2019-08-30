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
#include "gofunctionhintassistvisitor.h"
#include "ast.h"

namespace GoTools {

GoFunctionHintAssistVisitor::GoFunctionHintAssistVisitor(GoSource::Ptr doc)
    : ScopePositionVisitor(doc.data())
    , m_doc(doc)
{ }

QStringList GoFunctionHintAssistVisitor::functionArguments(unsigned pos)
{
    m_funcExpr = 0;
    m_pos = pos;
    m_functionArgs.clear();
    m_lparenPosition = -1;

    if (m_doc && isValidResolveContext()) {
        acceptForPosition(m_initialFileAst->decls);

        if (m_funcExpr) {
            if (const FuncTypeAST *type = dynamic_cast<const FuncTypeAST *>(m_funcExpr->resolve(this).type())) {
                if (type->params) {
                    for (FieldListAST *field_it = type->params->fields; field_it; field_it = field_it->next) {
                        if (FieldAST *field = field_it->value) {
                            QString typeDescr = field->type ? field->type->describe() : "";
                            if (DeclIdentListAST *name_it = field->names) {
                                for (; name_it; name_it = name_it->next) {
                                    QString argName;
                                    if (DeclIdentAST *name = name_it->value)
                                        argName = name->ident->toString();
                                    QString argDescr(argName + QLatin1String(" ") + typeDescr);
                                    m_functionArgs << argDescr.trimmed();
                                }
                            } else if (!typeDescr.isEmpty()) {
                                m_functionArgs << typeDescr;
                            }
                        }
                    }
                }
            }
        }

    }

    return m_functionArgs;
}

bool GoFunctionHintAssistVisitor::visit(CallExprAST *ast)
{
    unsigned lparen = ast->t_lparen;
    unsigned rparen = ast->t_rparen;
    if (lparen && rparen) {
        const Token &ltk = _tokens->at(lparen);
        const Token &rtk = _tokens->at(rparen);
        unsigned lparenPosition = ltk.begin();
        if (m_pos >= lparenPosition && m_pos < rtk.begin()) {
            m_funcExpr = ast->fun;
            m_lparenPosition = lparenPosition;
            if (m_pos == lparenPosition) {
                _traverseFinished = true;
                return false;
            }
            accept(ast->args);
        }
    }

    return false;
}



}   // namespace GoTools

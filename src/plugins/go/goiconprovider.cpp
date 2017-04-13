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

#include "goiconprovider.h"

namespace Go {

GoIconProvider *GoIconProvider::m_instance = nullptr;

GoIconProvider *GoIconProvider::instance()
{ return m_instance; }

QIcon GoIconProvider::icon(GoIconProvider::IconKind kind) const
{
    switch (kind) {
        case Keyword: return m_keywordIcon;
        case Func: return m_funcIcon;
        case Package: return m_packageIcon;
        case PackageImport: return m_importPackageIcon;
        case Variable: return m_varIcon;
        case Type: return m_typeIcon;
        case Const: return m_constIcon;
        case Other: return m_otherIcon;
    }

    return m_otherIcon;
}

GoIconProvider::GoIconProvider()
    : m_keywordIcon(QLatin1String(":/go/images/keyword.png"))
    , m_varIcon(QLatin1String(":/go/images/var.png"))
    , m_funcIcon(QLatin1String(":/go/images/func.png"))
    , m_typeIcon(QLatin1String(":/go/images/type.png"))
    , m_constIcon(QLatin1String(":/go/images/const.png"))
    , m_packageIcon(QLatin1String(":/go/images/package.png"))
    , m_importPackageIcon(QLatin1String(":/go/images/pkg_import.png"))
    , m_otherIcon(QLatin1String(":/go/images/other.png"))
{
    m_instance = this;
}

}   // namespace Go

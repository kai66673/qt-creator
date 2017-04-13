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

#include "goplugin.h"

#include <QIcon>

namespace Go {

class GoIconProvider
{
public:
    enum IconKind {
        Keyword,
        Func,
        Package,
        PackageImport,
        Variable,
        Type,
        Const,
        Other
    };

    static GoIconProvider *instance();

    QIcon icon(IconKind kind) const;

private:
    GoIconProvider();
    static GoIconProvider *m_instance;

    QIcon m_keywordIcon;
    QIcon m_varIcon;
    QIcon m_funcIcon;
    QIcon m_typeIcon;
    QIcon m_constIcon;
    QIcon m_packageIcon;
    QIcon m_importPackageIcon;
    QIcon m_otherIcon;

    friend class Internal::GoPlugin;
};

}   // namespace Go

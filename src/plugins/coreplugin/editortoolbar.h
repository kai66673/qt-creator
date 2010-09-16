/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef FAKETOOLBAR_H
#define FAKETOOLBAR_H

#include "core_global.h"

#include <QtCore/QScopedPointer>

#include <utils/styledbar.h>

namespace Core {
    class IEditor;

struct EditorToolBarPrivate;

/**
  * Fakes an IEditor-like toolbar for design mode widgets such as Qt Designer and Bauhaus.
  * Creates a combobox for open files and lock and close buttons on the right.
  */
class CORE_EXPORT EditorToolBar : public Utils::StyledBar
{
    Q_OBJECT
    Q_DISABLE_COPY(EditorToolBar)
public:
    explicit EditorToolBar(QWidget *parent = 0);
    virtual ~EditorToolBar();

    enum ToolbarCreationFlags { FlagsNone = 0, FlagsStandalone = 1 };

    /**
      * Adds an editor whose state is listened to, so that the toolbar can be kept up to date
      * with regards to locked status and tooltips.
      */
    void addEditor(IEditor *editor);

    /**
      * Sets the editor and adds its custom toolbar to the widget.
      */
    void setCurrentEditor(IEditor *editor);

    void setToolbarCreationFlags(ToolbarCreationFlags flags);

    /**
      * Adds a toolbar to the widget and sets invisible by default.
      */
    void addCenterToolBar(QWidget *toolBar);

    void setNavigationVisible(bool isVisible);
    void setCanGoBack(bool canGoBack);
    void setCanGoForward(bool canGoForward);
    void removeToolbarForEditor(IEditor *editor);

public slots:
    void updateEditorStatus(IEditor *editor);

signals:
    void closeClicked();
    void goBackClicked();
    void goForwardClicked();
    void listSelectionActivated(int row);

private slots:
    void updateEditorListSelection(Core::IEditor *newSelection);
    void changeActiveEditor(int row);
    void listContextMenu(QPoint);
    void makeEditorWritable();

    void checkEditorStatus();
    void closeView();
    void updateActionShortcuts();

private:
    void updateToolBar(QWidget *toolBar);
    IEditor *currentEditor() const;

    QScopedPointer<EditorToolBarPrivate> d;
};

} // namespace Core

#endif // FAKETOOLBAR_H

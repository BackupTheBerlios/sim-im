/***************************************************************************
                          mainwin.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MAINWIN_H
#define _MAINWIN_H

#include <list>
#include "simapi.h"
#include "event.h"
#include "simgui/toolbar.h"

#include <QMainWindow>

using namespace std;

class QToolBat;
class CorePlugin;
class QCloseEvent;
class QEvent;
class QFocusEvent;
class QHBoxLayout;
class QResizeEvent;
class QSizeGrip;
class QVBoxLayout;

class CorePlugin;
class UserView;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(CorePlugin* core);
    ~MainWindow();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void resizeEvent(QResizeEvent *e);

//    void focusInEvent(QFocusEvent*);
//    void quit();
//    void addStatus(QWidget *w, bool);
//    list<QWidget*> statusWidgets;
//    QString	m_icon;

//    virtual void closeEvent(QCloseEvent *e);

private:
    void updateTitle();
    void addWidget(QWidget* widget);

//    void populateMainToolbar();
//    void loadDefaultCommandList();

    UserView* m_view;
    SIM::ToolBar* m_bar;
    QVBoxLayout* m_layout;
    QWidget* m_centralWidget;
    CorePlugin* m_core;
    bool m_noresize;

private slots:
    void eventInit();
};

#endif


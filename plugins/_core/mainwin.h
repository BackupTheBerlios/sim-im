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
    //bool m_bNoResize;
protected:
    bool eventFilter(QObject *o, QEvent *e);
//    void focusInEvent(QFocusEvent*);
//    void resizeEvent(QResizeEvent *e);
//    void quit();
//    void addWidget(QWidget*, bool bDown);
//    void addStatus(QWidget *w, bool);
//    list<QWidget*> statusWidgets;
//    QString	m_icon;

//    virtual void closeEvent(QCloseEvent *e);

//    friend class CorePlugin;

private:
    void updateTitle();
//    void populateMainToolbar();
//    void loadDefaultCommandList();
//    UserView* m_view;

    SIM::ToolBar* m_bar;
    QVBoxLayout* m_layout;
    QWidget* m_centralWidget;
    CorePlugin* m_core;

private slots:
    void eventInit();
};

#endif


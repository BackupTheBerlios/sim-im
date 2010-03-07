/***************************************************************************
                          dock.h  -  description
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

#ifndef _DOCK_H
#define _DOCK_H

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

#include <QObject>
#include <QDateTime>

class DockWnd;
class QMenu;

class DockPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    DockPlugin(unsigned, Buffer*);
    virtual ~DockPlugin();

    QMenu *createMenu();

protected slots:
    void showPopup(QPoint);
    void toggleWin();
    void doubleClicked();
    void timer();
protected:
    virtual bool processEvent(SIM::Event *e);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QByteArray getConfig();
    virtual bool eventFilter(QObject*, QEvent*);
    QWidget *getMainWindow();
    bool isMainShow();
    void init();

    DockWnd *m_dock;
    QWidget* m_main;
    QMenu *m_popup;
    unsigned long DockMenu;
    unsigned long CmdTitle;
    unsigned long CmdToggle;
    unsigned long CmdCustomize;
    bool m_bQuit;
    QDateTime m_inactiveTime;
    friend class DockCfg;
    friend class DockWnd;

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


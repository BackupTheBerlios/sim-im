/***************************************************************************
                          monitor.h  -  description
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

#ifndef _MONITOR_H
#define _MONITOR_H

#include <QMainWindow>
#include <QMutex>

#include "event.h"

class QAction;
class QMenu;
class QTextEdit;
class NetmonitorPlugin;

const unsigned short L_PACKETS = 0x08;

class MonitorWindow : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MonitorWindow(NetmonitorPlugin*);
signals:
    void finished();
protected slots:
    void save();
    void exit();
    void copy();
    void erase();
    void pause();
    void toggleType(QAction*);
    void toggleAutoscroll();
    void adjustFile();
    void adjustEdit();
    void adjustLog();
protected:
    virtual bool processEvent(SIM::Event *e);
    void closeEvent(QCloseEvent*);
    bool bPause;
    bool bAutoscroll;
    QTextEdit  *edit;
    QAction *m_saveAction;
    QAction *m_autoscrollAction;
    QAction *m_pauseAction;
    QAction *m_copyAction;
    QAction *m_eraseAction;
    QMenu *m_menuLog;
    NetmonitorPlugin *m_plugin;
    QMutex m_mutex;
    QStringList m_logStrings;
protected slots:
    void outputLog();
};

#endif


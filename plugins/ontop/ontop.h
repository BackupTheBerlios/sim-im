/***************************************************************************
                          ontop.h  -  description
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

#ifndef _ONTOP_H
#define _ONTOP_H

#ifdef WIN32
#include <windows.h>
#endif

#include <QObject>
#include <QEvent>
#include <QByteArray>

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

class EventInTaskManager : public SIM::Event
{
public:
    EventInTaskManager(bool bShowInTask)
        : Event(SIM::eEventInTaskManager), m_bShowInTask(bShowInTask) {}

    bool showInTaskmanager() const { return m_bShowInTask; }
protected:
    bool m_bShowInTask;
};

class EventOnTop : public SIM::Event
{
public:
    // bShowOnTop is maybe wrong, rename if someone knows what it means
    EventOnTop(bool bShowOnTop)
        : Event(SIM::eEventOnTop), m_bShowOnTop(bShowOnTop) {}

    bool showOnTop() const { return m_bShowOnTop; }
protected:
    bool m_bShowOnTop;
};

struct OnTopData
{
//    SIM::Data	OnTop;
//    SIM::Data	InTask;
//    SIM::Data	ContainerOnTop;
};

class OnTopPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    OnTopPlugin(unsigned, Buffer*);
    virtual ~OnTopPlugin();

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual bool processEvent(SIM::Event *e);
#if defined(USE_KDE) || defined(WIN32)
    virtual QWidget *createConfigWindow(QWidget *parent);
#endif
    virtual QByteArray getConfig();
    void getState();
    void setState();
    QWidget *getMainWindow();
    unsigned CmdOnTop;
//    PROP_BOOL(OnTop);
//    PROP_BOOL(InTask);
//    PROP_BOOL(ContainerOnTop);
//    OnTopData data;
#ifdef WIN32
    HWND m_state;
#endif
    friend class OnTopCfg;

private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


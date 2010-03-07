/***************************************************************************
                          transparent.h  -  description
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

#ifndef _TRANSPARENT_H
#define _TRANSPARENT_H

#include <QObject>
#include <QTime>

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

class QWidget;
class QTimer;

class TransparentPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    TransparentPlugin(unsigned, Buffer*);
    virtual ~TransparentPlugin();
    void	setState();
    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
public Q_SLOTS:
    void tick();
    void tickMouse();
protected:
    virtual QByteArray getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool eventFilter(QObject*, QEvent*);
    QWidget *getMainWindow();
    bool    m_bState;
    virtual bool processEvent(SIM::Event *e);

    QTime startTime;
    QTimer   *timer;
    bool	 m_bHaveMouse;
    bool	 m_bActive;
private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


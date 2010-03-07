/***************************************************************************
                          forward.h  -  description
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

#ifndef _FORWARD_H
#define _FORWARD_H

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

struct ForwardUserData
{
    SIM::Data	Phone;
    SIM::Data	Send1st;
    SIM::Data	Translit;
};

class ForwardPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
public:
    ForwardPlugin(unsigned);
    virtual ~ForwardPlugin();
    unsigned long user_data_id;

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool processEvent(SIM::Event *e);
private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


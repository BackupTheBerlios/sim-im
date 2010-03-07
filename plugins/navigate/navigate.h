/***************************************************************************
                          navigate.h  -  description
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

#ifndef _NAVIGATE_H
#define _NAVIGATE_H

#include "simapi.h"

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"
#include <QByteArray>


class NavigatePlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
public:
    NavigatePlugin(unsigned, Buffer *name);
    virtual ~NavigatePlugin();
    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
protected:
    virtual bool processEvent(SIM::Event *e);
    QString parseUrl(const QString &text);
    unsigned long CmdMail;
    unsigned long CmdMailList;
    unsigned long CmdCopyLocation;
    unsigned long MenuMail;
    virtual QByteArray getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    friend class NavCfg;
private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


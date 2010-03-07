/***************************************************************************
                          icon.h  -  description
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

#ifndef _ICON_H
#define _ICON_H

#include "cfg.h"
#include "plugins.h"
#include "propertyhub.h"


class IconsPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
public:
    IconsPlugin(unsigned, Buffer*);
    virtual ~IconsPlugin();
    void setIcons(bool bForce);

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
protected:
    bool processEvent(SIM::Event *e);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QByteArray getConfig();
private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


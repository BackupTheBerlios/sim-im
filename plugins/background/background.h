/***************************************************************************
                          background.h  -  description
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

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

#include <QImage>
#include <QPixmap>
#include <QByteArray>

const unsigned ContactLeft  = 0;
const unsigned ContactScale = 1;
const unsigned WindowTop    = 2;
const unsigned WindowBottom = 3;
const unsigned WindowCenter = 4;
const unsigned WindowScale  = 5;

class BackgroundPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
public:
    BackgroundPlugin(unsigned, Buffer *name);
    virtual ~BackgroundPlugin();
    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

protected:
    virtual bool processEvent(SIM::Event *e);
    virtual QByteArray getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
	
    QImage	bgImage;
    QPixmap	bgScale;
    QPixmap &makeBackground(int w, int h);
    void redraw();
    friend class BkgndCfg;

private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


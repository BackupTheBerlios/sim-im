/***************************************************************************
                          background.cpp  -  description
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

#include "misc.h"

#include "background.h"
#include "bkgndcfg.h"
#include "log.h"

#include "profile.h"
#include "profilemanager.h"

#include <QFile>
#include <QPixmap>
#include <QByteArray>
#include <QPainter>

using namespace SIM;

Plugin *createBackgroundPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new BackgroundPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Background"),
        I18N_NOOP("Plugin provides background pictures for user list"),
        VERSION,
        createBackgroundPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

BackgroundPlugin::BackgroundPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
{
    m_propertyHub = SIM::PropertyHub::create("background");
    redraw();
}

BackgroundPlugin::~BackgroundPlugin()
{
    
}

QByteArray BackgroundPlugin::getConfig()
{
    return QByteArray(); //Fixme
}

QWidget *BackgroundPlugin::createConfigWindow(QWidget *parent)
{
    return new BkgndCfg(parent, this);
}

bool BackgroundPlugin::processEvent(Event *e)
{
    if (e->type() == eEventPaintView){
        EventPaintView *ev = static_cast<EventPaintView*>(e);
        EventPaintView::PaintView *pv = ev->paintView();;
        if (!bgImage.isNull()){
            unsigned w = bgImage.width();
            unsigned h = bgImage.height();
            int x = pv->pos.x();
            int y = pv->pos.y();
            bool bTiled = false;
            unsigned pos = value("Position").toUInt();
            switch(pos){
            case ContactLeft:
                h = pv->height;
                bTiled = true;
                break;
            case ContactScale:
                h = pv->height;
                w = pv->win->width();
                bTiled = true;
                break;
            case WindowTop:
                break;
            case WindowBottom:
                y += (bgImage.height() - pv->win->height());
                break;
            case WindowCenter:
                y += (bgImage.height() - pv->win->height()) / 2;
                break;
            case WindowScale:
                w = pv->win->width();
                h = pv->win->height();
                break;
            }
            const QPixmap &bg = makeBackground(w, h);
            if (bTiled){
                for (int py = 0; py < pv->size.height(); py += bg.height()){
                    pv->p->drawPixmap(QPoint(0, py), bgScale, QRect(x, 0, w, h));
                }
            }else{
                pv->p->drawPixmap(QPoint(0, 0), bgScale, QRect(x, y, pv->size.width(), pv->size.height()));
                pv->isStatic = true;
            }
        }
        pv->margin = pv->isGroup ? value("MarginGroup").toUInt() : value("MarginContact").toUInt();
    }
    else if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("_core");
        if(!hub.isNull())
            setPropertyHub(hub);
        redraw();
    }
    return false;
}

void BackgroundPlugin::redraw()
{
    bgImage = QImage();
    bgScale = QPixmap();
    if (value("Background").toString().isEmpty())
        return;
    bgImage = QImage(value("Background").toString());
    EventRepaintView e;
    e.process();
}

QPixmap &BackgroundPlugin::makeBackground(int w, int h)
{
    if (bgImage.isNull())
        return bgScale;
    if ((bgScale.width() != w) || (bgScale.height() != h)){
        if ((bgImage.width() == w) && (bgImage.height() == h)){
            bgScale = QPixmap::fromImage(bgImage);
        }else{
            QImage img = bgImage.scaled( w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
            bgScale = QPixmap::fromImage( img );
        }
    }
    return bgScale;
}

void BackgroundPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr BackgroundPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant BackgroundPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void BackgroundPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}
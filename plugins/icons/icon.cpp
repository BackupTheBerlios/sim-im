/***************************************************************************
                          icon.cpp  -  description
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

#include "icons.h"
#include "misc.h"
#include "log.h"

#include "profile.h"
#include "profilemanager.h"

#include "icon.h"
#include "iconcfg.h"

#include <QByteArray>

using namespace std;
using namespace SIM;

Plugin *createIconsPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new IconsPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Icons"),
        I18N_NOOP("Plugin provides JISP icons themes and emoticons"),
        VERSION,
        createIconsPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

IconsPlugin::IconsPlugin(unsigned base, Buffer *config)
        : QObject(), Plugin(base), EventReceiver(HighestPriority - 0x100)
{
    m_propertyHub = SIM::PropertyHub::create("icon");
    setIcons(false);
}

IconsPlugin::~IconsPlugin()
{

}

void IconsPlugin::setIcons(bool bForce)
{
    if (!bForce && value("Default").toBool())
        return;
    getIcons()->removeIconSet(NULL);
    if (value("Default").toBool()){
        getIcons()->addIconSet("icons/smile.jisp", false);
    }else{
        const QStringList l = value("Icons").toStringList();
        for (unsigned i = 0; i < value("NIcons").toUInt(); i++)
        {
            if(i >= (unsigned)l.size())
                break;
            getIcons()->addIconSet(l[i], false);
        }
    }
    EventIconChanged().process();
}

QByteArray IconsPlugin::getConfig()
{
    return QByteArray();
}

bool IconsPlugin::processEvent(Event *e)
{
    switch (e->type())
    {
        case eEventPluginLoadConfig:
        {
            PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("icon");
		    if(!hub.isNull())
			    setPropertyHub(hub);
			setIcons(false);
            break;
        }
        default:
            break;
    }
    return false;
}
QWidget *IconsPlugin::createConfigWindow(QWidget *parent)
{
    return new IconCfg(parent, this);
}

void IconsPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr IconsPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant IconsPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void IconsPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

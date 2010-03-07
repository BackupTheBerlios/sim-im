/***************************************************************************
                          styles.cpp  -  description
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

#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>
#include <QMenu>

#include "simgui/fontedit.h"

#include "misc.h"

#include "profile.h"
#include "profilemanager.h"

#include "styles.h"
#include "stylescfg.h"

using namespace SIM;

Plugin *createStylesPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new StylesPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Styles"),
        I18N_NOOP("Plugin provides styles"),
        VERSION,
        createStylesPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

StylesPlugin::StylesPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
    , m_saveBaseFont(NULL)
    , m_saveMenuFont(NULL)
{
    m_propertyHub = SIM::PropertyHub::create("styles");
    m_savePalette = new QPalette(QApplication::palette());
}

StylesPlugin::~StylesPlugin()
{
    delete m_saveBaseFont;
    delete m_saveMenuFont;
    delete m_savePalette;
}

QByteArray StylesPlugin::getConfig()
{
    return QByteArray();//Fixme
}

QWidget *StylesPlugin::createConfigWindow(QWidget *parent)
{
    return new StylesConfig(parent, this);
}

void StylesPlugin::setFonts()
{
    if (value("SystemFonts").toBool()){
        if (m_saveBaseFont)
            QApplication::setFont(*m_saveBaseFont);
        if (m_saveMenuFont)
            QApplication::setFont(*m_saveMenuFont, "QMenu");
    }else{
        setupDefaultFonts();
        QApplication::setFont(FontEdit::str2font(value("BaseFont").toString(), *m_saveBaseFont));
        QApplication::setFont(FontEdit::str2font(value("MenuFont").toString(), *m_saveMenuFont), "QMenu");
    }
}

void StylesPlugin::setupDefaultFonts()
{
    if (m_saveBaseFont == NULL)
        m_saveBaseFont = new QFont(QApplication::font());
    if (m_saveMenuFont == NULL){
        QMenu menu;
        m_saveMenuFont = new QFont(QApplication::font(&menu));
    }
}

void StylesPlugin::setColors()
{
    if (value("SystemColors").toBool()){
        QApplication::setPalette(*m_savePalette);
    }else{
        QApplication::setPalette(QPalette(QColor(value("BtnColor").toUInt()), QColor(value("BgColor").toUInt())));
    }
}

void StylesPlugin::setStyles()
{
    QString sStyle = value("Style").toString();
    QStyle *style = QStyleFactory::create(sStyle);
    if (style){
        QApplication::setStyle(style);
        if (!value("SystemColors").toBool())
            setColors();
    }else{
        setValue("Style", QString());
    }
}

bool StylesPlugin::processEvent(SIM::Event *e)
{
    if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("style");
        if(!hub.isNull())
            setPropertyHub(hub);
        setFonts();
        if (value("SystemColors").toBool()){
            setValue("BtnColor", m_savePalette->color(QPalette::Active, QPalette::Button).rgb());
            setValue("BgColor", m_savePalette->color(QPalette::Active, QPalette::Background).rgb());
        }
        setColors();
        setStyles();
    }
    return false;
}

void StylesPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr StylesPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant StylesPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void StylesPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

/***************************************************************************
                          stylescfg.cpp  -  description
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

#include <QTabWidget>
#include <QStyle>
#include <QDir>
#include <QStyleFactory>

#include "fontconfig.h"
#include "misc.h"

#include "stylescfg.h"
#include "styles.h"

StylesConfig::StylesConfig(QWidget *parent, StylesPlugin *plugin)
        : QWidget(parent)
        , m_plugin(plugin)
{
    setupUi(this);
    for (QObject *p = parent; p != NULL; p = p->parent()){
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if (!tab)
            continue;
        font_cfg = new FontConfig(tab, m_plugin);
        tab->addTab(font_cfg, i18n("Fonts and colors"));
        break;
    }
    lstStyle->addItems(QStyleFactory::keys());
    if (!m_plugin->value("Style").toString().isEmpty()){
        QList<QListWidgetItem *> items;
        items = lstStyle->findItems(m_plugin->value("Style").toString(), Qt::MatchExactly);
        if (items.count())
            lstStyle->setCurrentItem(items[0]);
    }
}

StylesConfig::~StylesConfig()
{
}

void StylesConfig::apply()
{
    font_cfg->apply();
    QListWidgetItem *item = lstStyle->currentItem();
    if (item) {
        m_plugin->setValue("Style", item->text());
        m_plugin->setStyles();
    }
}


/***************************************************************************
                          plugincfg.cpp  -  description
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
#include "profilemanager.h"
#include "log.h"

#include "plugincfg.h"
#include <QVBoxLayout>

using namespace SIM;

PluginCfg::PluginCfg(QWidget *parent, const QString& pluginname) : QWidget(parent)
{
	setupUi(this);
    m_pluginName = pluginname;
	if(ProfileManager::instance()->currentProfile()->enabledPlugins().contains(m_pluginName) ||
			getPluginManager()->isPluginAlwaysEnabled(m_pluginName))
	{
		PluginPtr plugin = getPluginManager()->plugin(pluginname);
        QWidget *w = plugin->createConfigWindow(addWnd);
        if (w){
            QVBoxLayout *lay = new QVBoxLayout(addWnd);
            lay->addWidget(w);
            QObject::connect(parent->topLevelWidget(), SIGNAL(applyChanges()), w, SLOT(apply()));
            // adjust plugin widget
            w->setMinimumSize(w->sizeHint());
            w->adjustSize();
            // adjust addWnd widget
            addWnd->setMinimumSize(addWnd->sizeHint());
            addWnd->adjustSize();
        }
    }
    // adjust description
    if(!getPluginManager()->pluginDescription(pluginname).isNull())
	{
        lblDescription->setText(i18n(getPluginManager()->pluginDescription(pluginname)));
    }
	else
	{
        lblDescription->setText("");
    }
    // adjust tab
    tabWnd->setCurrentIndex(0);
    tabWnd->setTabText(0, i18n(getPluginManager()->pluginTitle(pluginname)));
    tabWnd->setMinimumSize(tabWnd->sizeHint());
    tabWnd->adjustSize();
    // adjust complete widget
    setMinimumSize(sizeHint());
    adjustSize();
    if (getPluginManager()->isPluginAlwaysEnabled(pluginname)) {
        chkEnable->hide();
	} else {
		chkEnable->setEnabled(true);
		chkEnable->setChecked(ProfileManager::instance()->currentProfile()->enabledPlugins().contains(pluginname));
	}
}

void PluginCfg::apply()
{
}


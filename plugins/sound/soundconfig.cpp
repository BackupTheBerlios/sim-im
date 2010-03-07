/***************************************************************************
                          soundconfig.cpp  -  description
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

#include "simapi.h"

#include <QCheckBox>

#include <QTabWidget>
#include <QSound>

#include "simgui/editfile.h"
#include "contacts.h"
#include "misc.h"

#include "soundconfig.h"
#include "sounduser.h"
#include "sound.h"

using SIM::getContacts;

SoundConfig::SoundConfig(QWidget *parent, SoundPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
  , user_cfg(NULL)
{
    setupUi(this);

    edtStartup->setText(plugin->value("StartUp").toString());
    edtFileDone->setText(plugin->value("FileDone").toString());
    edtSent->setText(plugin->value("MessageSent").toString());

    for (QObject *p = parent; p != NULL; p = p->parent())
    {
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if (!tab)
            continue;

        user_cfg = new SoundUserConfig(tab, getContacts()->userdata(), plugin);
        tab->addTab(user_cfg, i18n("Events"));
        tab->adjustSize();
        break;
    }
}

SoundConfig::~SoundConfig()
{
}

void SoundConfig::apply()
{
    if(user_cfg)
	{
		SIM::PropertyHubPtr data = getContacts()->userdata();
		user_cfg->apply(data, true);
    }
	m_plugin->setValue("StartUp", edtStartup->text());
    m_plugin->setValue("FileDone", edtFileDone->text());
    m_plugin->setValue("MessageSent", edtSent->text());
}

void SoundConfig::artsToggled(bool)
{
}


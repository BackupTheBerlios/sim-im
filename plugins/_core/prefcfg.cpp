/***************************************************************************
                          prefcfg.cpp  -  description
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

#include "log.h"
#include "prefcfg.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#include <QVBoxLayout>

using namespace SIM;

PrefConfig::PrefConfig(QWidget *parent, CommandDef *cmd, Contact *contact, Group *group)
  : QWidget(parent)
  , m_cmd(cmd)
  , m_contact(contact)
  , m_group(group)
{
    setupUi(this);
	SIM::PropertyHubPtr data;
	SIM::PropertyHubPtr mapdata;
    if (m_contact)
    {
        data = m_contact->getUserData(m_cmd->accel); // HACK !
        if (m_contact->getUserData()->getUserData(m_cmd->accel))
            chkOverride->setChecked(true);
        mapdata = m_contact->getUserData()->root();
    }
    else if (m_group) {
        mapdata = m_group->getUserData()->root();
    }
    QWidget *w = NULL;
	if(cmd->flags & COMMAND_CONTACT)
	{
        w = ((getPreferencesWindowContact)(cmd->param))(addWnd, mapdata);
        chkOverride->setChecked(w->property("override").toBool());
	}
	else if(!data.isNull())
	{
        w = ((getPreferencesWindowContact)(cmd->param))(addWnd, data);
	}
    if(w) {
        QVBoxLayout *lay = new QVBoxLayout(addWnd);
        lay->addWidget(w);

        if(cmd->flags & COMMAND_CONTACT)
        {
            connect(this, SIGNAL(apply(SIM::PropertyHubPtr, bool)), w, SLOT(apply(SIM::PropertyHubPtr, bool)));
        }
        else
        {
            connect(this, SIGNAL(apply(SIM::PropertyHubPtr)), w, SLOT(apply(SIM::PropertyHubPtr)));
        }
        if(addWnd)
            addWnd->setMinimumSize(w->minimumSizeHint());
        setMinimumSize(sizeHint());
    }
    tabWnd->setCurrentIndex(0);
    tabWnd->setTabText(tabWnd->currentIndex(), i18n(m_cmd->text));
    tabWnd->adjustSize();
    connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideToggled(bool)));
    overrideToggled(chkOverride->isChecked());
}

void PrefConfig::apply()
{
    if(m_cmd->flags & COMMAND_CONTACT)
    {
		SIM::PropertyHubPtr data;
        if (m_contact)
        {
            log(L_DEBUG, "Contact");
            data = m_contact->getUserData()->root();
        }
        else if (m_group)
        {
            log(L_DEBUG, "Group");
            data = m_group->getUserData()->root();
        }
        log(L_DEBUG, "NULL Contact");
        if (data)
            emit apply(data, chkOverride->isChecked());
    }
    else
    {
        if (chkOverride->isChecked())
        {
			SIM::PropertyHubPtr data;
            if (m_contact)
            {
                data = m_contact->getUserData()->getUserData(m_cmd->accel);
				if(data.isNull())
					data = m_contact->getUserData()->createUserData(m_cmd->accel);
            }
            else if (m_group)
            {
                data = m_group->getUserData()->getUserData(m_cmd->accel);
				if(data.isNull())
					data = m_group->getUserData()->createUserData(m_cmd->accel);
            }
            if (data)
                emit apply(data);
        }
        else
        {
            if (m_contact)
            {
                m_contact->getUserData()->destroyUserData(m_cmd->accel);
            }
            else if(m_group)
            {
                m_group->getUserData()->destroyUserData(m_cmd->accel);
            }
        }
    }
}

void PrefConfig::overrideToggled(bool bState)
{
    for(int i = 0; i < tabWnd->count(); ++i)
        tabWnd->widget(i)->setEnabled(bState);
}

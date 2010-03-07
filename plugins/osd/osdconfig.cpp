/***************************************************************************
                          osdconfig.cpp  -  description
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

#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QTabWidget>
#include <QLabel>

#include "simgui/fontedit.h"
#include "misc.h"
#include "simgui/qcolorbutton.h"

#include "osdconfig.h"
#include "osdiface.h"
#include "osd.h"

using SIM::getContacts;

OSDConfig::OSDConfig(QWidget *parent, SIM::PropertyHubPtr data, OSDPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);
    chkMessage->setChecked(data->value("EnableMessage").toBool());
    chkMessageContent->setChecked(data->value("EnableMessageShowContent").toBool());
    chkCapsLockFlash->setChecked(data->value("EnableCapsLockFlash").toBool());
    chkStatus->setChecked(data->value("EnableAlert").toBool());
    chkStatusOnline->setChecked(data->value("EnableAlertOnline").toBool());
    chkStatusAway->setChecked(data->value("EnableAlertAway").toBool());
    chkStatusNA->setChecked(data->value("EnableAlertNA").toBool());
    chkStatusDND->setChecked(data->value("EnableAlertDND").toBool());
    chkStatusOccupied->setChecked(data->value("EnableAlertOccupied").toBool());
    chkStatusFFC->setChecked(data->value("EnableAlertFFC").toBool());
    chkStatusOffline->setChecked(data->value("EnableAlertOffline").toBool());
    chkTyping->setChecked(data->value("EnableTyping").toBool());
    for (QObject *p = parent; p != NULL; p = p->parent()){
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if (!tab)
            continue;
		SIM::PropertyHubPtr data = getContacts()->getUserData("OSD");
        m_iface = new OSDIface(tab, data, plugin);
        tab->addTab(m_iface, i18n("&Interface"));
        break;
    }
    edtLines->setValue(data->value("ContentLines").toUInt());
    connect(chkStatus, SIGNAL(toggled(bool)), this, SLOT(statusToggled(bool)));
    connect(chkMessage, SIGNAL(toggled(bool)), this, SLOT(showMessageToggled(bool)));
    connect(chkMessageContent, SIGNAL(toggled(bool)), this, SLOT(contentToggled(bool)));
    showMessageToggled(chkMessage->isChecked());
    contentToggled(chkMessageContent->isChecked());
    statusToggled(data->value("EnableAlert").toBool());
}

void OSDConfig::apply()
{
    apply(getContacts()->getUserData("OSD"));
}

void OSDConfig::apply(SIM::PropertyHubPtr data)
{
    data->setValue("EnableMessage", chkMessage->isChecked());
    data->setValue("EnableMessageShowContent", chkMessageContent->isChecked());
    data->setValue("EnableCapsLockFlash", chkCapsLockFlash->isChecked());
    data->setValue("EnableAlert", chkStatus->isChecked());
    data->setValue("EnableAlertOnline", chkStatusOnline->isChecked());
    data->setValue("EnableAlertAway", chkStatusAway->isChecked());
    data->setValue("EnableAlertNA", chkStatusNA->isChecked());
    data->setValue("EnableAlertDND", chkStatusDND->isChecked());
    data->setValue("EnableAlertOccupied", chkStatusOccupied->isChecked());
    data->setValue("EnableAlertFFC", chkStatusFFC->isChecked());
    data->setValue("EnableAlertOffline", chkStatusOffline->isChecked());
    data->setValue("EnableTyping", chkTyping->isChecked());
    data->setValue("ContentLines", (uint)edtLines->text().toULong());
    m_iface->apply(data);
}

void OSDConfig::statusToggled(bool bState)
{
    chkStatusOnline->setEnabled(bState);
    chkStatusAway->setEnabled(bState);
    chkStatusNA->setEnabled(bState);
    chkStatusDND->setEnabled(bState);
    chkStatusOccupied->setEnabled(bState);
    chkStatusFFC->setEnabled(bState);
    chkStatusOffline->setEnabled(bState);
    chkCapsLockFlash->setEnabled(bState);
}

void OSDConfig::showMessageToggled(bool bState)
{
    chkMessageContent->setEnabled(bState);
    edtLines->setEnabled(bState && chkMessageContent->isChecked());
    lblLines->setEnabled(bState && chkMessageContent->isChecked());
}

void OSDConfig::contentToggled(bool bState)
{
    edtLines->setEnabled(bState && chkMessage->isChecked());
    lblLines->setEnabled(bState && chkMessage->isChecked());
}


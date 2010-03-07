/***************************************************************************
                          forwardcfg.cpp  -  description
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

#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

#include "contacts/contact.h"
#include "misc.h"

#include "forwardcfg.h"
#include "forward.h"

using namespace SIM;

ForwardConfig::ForwardConfig(QWidget *parent, PropertyHubPtr data, ForwardPlugin *plugin) : QWidget(parent)
{
	setupUi(this);
    m_plugin = plugin;
    //PropertyHubPtr data = (ForwardUserData*)_data;
    chkFirst->   setChecked(data->value("Send1st")  .toBool());
    chkTranslit->setChecked(data->value("Translit") .toBool());
    cmbPhone->setEditable(true);
    QString phones = getContacts()->owner()->getPhones();
    while (!phones.isEmpty())
    {
        QString item = getToken(phones, ';', false);
        QString number = getToken(item, ',');
        getToken(item, ',');
        if (item.toULong() == CELLULAR)
            cmbPhone->insertItem(INT_MAX,number);
    }
    cmbPhone->lineEdit()->setText(data->value("Phone").toString());
}

void ForwardConfig::apply(PropertyHubPtr data)
{
    //ForwardUserData *data = (ForwardUserData*)_data;
    data->setValue("Send1st",   chkFirst->isChecked()       );
    data->setValue("Translit",  chkTranslit->isChecked()    );
    data->setValue("Phone",     cmbPhone->lineEdit()->text());
}

void ForwardConfig::apply()
{
    apply(getContacts()->getUserData("forward"));
}


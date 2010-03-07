/***************************************************************************
                          msgcfg.cpp  -  description
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

#include "msgcfg.h"
#include "filecfg.h"
#include "smscfg.h"
#include "core.h"

#include <QCheckBox>
#include <QRadioButton>
#include <QTabWidget>

MessageConfig::MessageConfig(QWidget *parent, SIM::PropertyHubPtr _data)
  : QWidget(parent)
{
    setupUi(this);
    m_file = NULL;
    for (QObject *p = parent; p != NULL; p = p->parent()){
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if(!tab)
            continue;
        m_file = new FileConfig(tab, _data);
        tab->addTab(m_file, i18n("File"));
        tab->adjustSize();
        break;
    }

	SIM::PropertyHubPtr data = _data;
    chkOnline->setChecked(data->value("OpenOnOnline").toBool());
    chkStatus->setChecked(data->value("LogStatus").toBool());
    switch (data->value("OpenNewMessage").toUInt()){
    case NEW_MSG_NOOPEN:
        btnNoOpen->setChecked(true);
        break;
    case NEW_MSG_MINIMIZE:
        btnMinimize->setChecked(true);
        break;
    case NEW_MSG_RAISE:
        btnRaise->setChecked(true);
        break;
    }
}

void MessageConfig::apply(SIM::PropertyHubPtr _data)
{
    if (m_file)
        m_file->apply(_data);

	SIM::PropertyHubPtr data = _data;
    data->setValue("OpenOnOnline", chkOnline->isChecked());
    data->setValue("LogStatus", chkStatus->isChecked());
    data->setValue("OpenNewMessage", NEW_MSG_NOOPEN);
    if (btnMinimize->isChecked())
        data->setValue("OpenNewMessage", NEW_MSG_MINIMIZE);
    if (btnRaise->isChecked())
        data->setValue("OpenNewMessage", NEW_MSG_RAISE);
}

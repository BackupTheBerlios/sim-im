/***************************************************************************
                          connectionsetting.cpp  -  description
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
#include "connectionsettings.h"
#include "contacts/client.h"

#include <QVBoxLayout>

using namespace SIM;

ConnectionSettings::ConnectionSettings(Client *client) : QDialog(NULL)
        //: ConnectionSettingsBase(NULL, NULL, true)
{
	setupUi(this);
    SET_WNDPROC("client")
    setButtonsPict(this);
    m_client = client;
    Protocol *protocol = client->protocol();
    const CommandDef *cmd = protocol->description();
    setWindowIcon(Icon(cmd->icon));
    setWindowTitle(i18n("Configure %1 client") .arg(i18n(cmd->text)));
    QVBoxLayout *lay = new QVBoxLayout(addWnd);
    QWidget *setupWnd = client->setupWnd();
    setupWnd->setParent(addWnd);
    setupWnd->move(QPoint());
    lay->addWidget(setupWnd);
    setupWnd->show();
}

void ConnectionSettings::apply()
{
}

/*
#ifndef NO_MOC_INCLUDES
#include "connectionsettings.moc"
#endif
*/


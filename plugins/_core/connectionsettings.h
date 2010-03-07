/***************************************************************************
                          connectionsetting.h  -  description
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

#ifndef _CONNECTIONSETTINGS_H
#define _CONNECTIONSETTINGS_H

#include "ui_connectionsettingsbase.h"
#include "event.h"

class ConnectionSettings : public QDialog, public Ui::ConnectionSettingsBase
{
    Q_OBJECT
public:
    ConnectionSettings(SIM::Client *client);
protected slots:
    void apply();
protected:
    SIM::Client *m_client;
};

#endif


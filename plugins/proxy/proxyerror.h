/***************************************************************************
                          proxyerror.h  -  description
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

#ifndef _PROXYERROR_H
#define _PROXYERROR_H

#include "socket/socket.h"
#include "ui_proxyerrorbase.h"

class ProxyConfig;
class ProxyPlugin;

class ProxyError : public QDialog, public Ui::ProxyErrorBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ProxyError(ProxyPlugin *plugin, SIM::TCPClient *client, const QString& msg);
    ~ProxyError();
signals:
    void apply();
protected:
    virtual bool processEvent(SIM::Event *e);
    virtual void accept();
    ProxyPlugin *m_plugin;
    SIM::TCPClient	*m_client;

    
};

#endif


/***************************************************************************
                          yahoo.h  -  description
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

#ifndef _YAHOO_H
#define _YAHOO_H

#include "contacts.h"
#include "contacts/client.h"

class YahooProtocol : public SIM::Protocol
{
public:
    YahooProtocol(SIM::Plugin *plugin);
    ~YahooProtocol();
    SIM::ClientPtr	createClient(Buffer *cfg);
    const SIM::CommandDef *description();
    const SIM::CommandDef *statusList();
    virtual const SIM::DataDef *userDataDef();
};

class YahooPlugin : public SIM::Plugin
{
public:
    YahooPlugin(unsigned);
    virtual ~YahooPlugin();
    static unsigned YahooPacket;
protected:
    void registerMessages();
    void unregisterMessages();
    SIM::Protocol *m_protocol;
};

#endif


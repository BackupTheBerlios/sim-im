/***************************************************************************
                          icq.h  -  description
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

#ifndef _ICQ_H
#define _ICQ_H

#include "contacts.h"
#include "contacts/client.h"
#include "contacts/imstatus.h"
#include "icqstatus.h"

//class CorePlugin;

//const unsigned IcqCmdBase	= 0x00040000;

//#include "icq_events.h"

//const unsigned long CmdVisibleList		= IcqCmdBase;
//const unsigned long CmdInvisibleList    = IcqCmdBase + 1;
//const unsigned long CmdGroups			= IcqCmdBase + 2;
//const unsigned long CmdIcqSendMessage	= IcqCmdBase + 5;
//const unsigned long CmdShowWarning		= IcqCmdBase + 6;
//const unsigned long CmdPasswordFail		= IcqCmdBase + 7;
//const unsigned long CmdPasswordSuccess	= IcqCmdBase + 8;

//const unsigned long MenuSearchResult	= IcqCmdBase;
//const unsigned long MenuIcqGroups		= IcqCmdBase + 2;

class ICQProtocol : public SIM::Protocol
{
public:
    ICQProtocol(SIM::Plugin *plugin);
    ~ICQProtocol();
    virtual QString name();
    virtual QString iconId();
    virtual SIM::ClientPtr createClient(const QString& name);

private:
	void initStatuses();
	void addStatus(ICQStatusPtr status);
    QList<ICQStatusPtr> m_states;
};

//class AIMProtocol : public SIM::Protocol
//{
//public:
//    AIMProtocol(SIM::Plugin *plugin);
//    ~AIMProtocol();
//    virtual SIM::ClientPtr createClient(Buffer *cfg);
//    virtual SIM::ClientPtr createClient(const QString& name);
//    virtual SIM::IMContact* createIMContact(const QSharedPointer<SIM::Client>& client);
//    virtual QStringList states();
//    virtual SIM::IMStatusPtr status(const QString& id);
//};

class ICQPlugin : public SIM::Plugin
{
public:
    ICQPlugin();
    virtual ~ICQPlugin();

    unsigned OscarPacket;
    unsigned ICQDirectPacket;
    unsigned AIMDirectPacket;
    unsigned RetrySendDND;
    unsigned RetrySendOccupied;
    SIM::ProtocolPtr m_icq;
    SIM::ProtocolPtr m_aim;
    static ICQPlugin  *icq_plugin;

    void registerMessages();
    void unregisterMessages();
};

#endif


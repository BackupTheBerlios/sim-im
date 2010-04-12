/***************************************************************************
                          livejournal.h  -  description
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

#ifndef _LIVEJOURNAL_H
#define _LIVEJOURNAL_H

#include "buffer.h"
#include "socket/socket.h"
#include "socket/tcpclient.h"
#include "fetch.h"
#include "contacts/client.h"

#include <QByteArray>

const unsigned long JournalCmdBase			= 0x00070000;
const unsigned long MessageJournal			= JournalCmdBase;
const unsigned long MessageUpdated			= JournalCmdBase + 1;
const unsigned long CmdDeleteJournalMessage	= JournalCmdBase + 2;
const unsigned long CmdMenuWeb				= JournalCmdBase + 3;
const unsigned long MenuWeb					= JournalCmdBase + 0x10;

const unsigned LIVEJOURNAL_SIGN	= 5;

const unsigned COMMENT_ENABLE	= 0;
const unsigned COMMENT_NO_MAIL	= 1;
const unsigned COMMENT_DISABLE	= 2;

/*
struct LiveJournalUserData : public SIM::clientData //Fixme: clientdata not a member of SIM
{
    SIM::Data	User;
    SIM::Data	Shared;
    SIM::Data	bChecked;
};
*/
struct JournalMessageData
{
    SIM::Data	Subject;
    SIM::Data	Private;
    SIM::Data	Time;
    SIM::Data	ID;
    SIM::Data	OldID;
    SIM::Data	Mood;
    SIM::Data	Comments;
};

class JournalMessage : public SIM::Message
{
public:
    JournalMessage(Buffer *cfg = NULL);
    ~JournalMessage();
    virtual QByteArray save();
    PROP_UTF8(Subject);
    PROP_ULONG(Private);
    PROP_ULONG(Time);
    PROP_ULONG(ID);
    PROP_ULONG(OldID);
    PROP_ULONG(Mood);
    PROP_ULONG(Comments);
protected:
    QString presentation();
    JournalMessageData data;
};

class LiveJournalPlugin : public SIM::Plugin
{
public:
    LiveJournalPlugin(unsigned);
    virtual ~LiveJournalPlugin();
    static unsigned MenuCount;
protected:
    SIM::Protocol *m_protocol;
};

class LiveJournalProtocol : public SIM::Protocol
{
public:
    LiveJournalProtocol(SIM::Plugin *plugin);
    ~LiveJournalProtocol();
    SIM::ClientPtr	createClient(Buffer *cfg);
    const SIM::CommandDef *description();
    const SIM::CommandDef *statusList();
    const SIM::DataDef *userDataDef();
};

struct LiveJournalClientData
{
    SIM::Data	Server;
    SIM::Data	URL;
    SIM::Data	Port;
    SIM::Data	Interval;
    SIM::Data	Mood;
    SIM::Data	Moods;
    SIM::Data	Menu;
    SIM::Data	MenuUrl;
    SIM::Data	FastServer;
    SIM::Data	UseFormatting;
    SIM::Data	UseSignature;
    SIM::Data	Signature;
    SIM::Data	LastUpdate;
    LiveJournalUserData	owner;
};

class LiveJournalClient;

class LiveJournalRequest
{
public:
    LiveJournalRequest(LiveJournalClient *client, const char *mode);
    virtual ~LiveJournalRequest();
    void addParam(const QString &key, const QString &value);
    void result(Buffer*);
    virtual void result(const QString &key, const QString &value) = 0;
protected:
    LiveJournalClient *m_client;
    Buffer *m_buffer;
    bool getLine(Buffer *b, QByteArray &line);
    friend class LiveJournalClient;
};

class QTimer;

class LiveJournalClient : public SIM::TCPClient, public FetchClient
{
    Q_OBJECT
public:
    LiveJournalClient(SIM::Protocol*, Buffer *cfg);
    ~LiveJournalClient();
    PROP_STR(Server);
    PROP_STR(URL);
    PROP_USHORT(Port);
    PROP_ULONG(Interval);
    PROP_STRLIST(Mood);
    PROP_ULONG(Moods);
    PROP_STRLIST(Menu);
    PROP_STRLIST(MenuUrl);
    PROP_BOOL(FastServer);
    PROP_BOOL(UseFormatting);
    PROP_BOOL(UseSignature);
    PROP_UTF8(Signature);
    PROP_STR(LastUpdate);
    QString getSignatureText();
    void auth_fail(const QString &err);
    void auth_ok();
    LiveJournalUserData	*findContact(const QString &user, SIM::Contact *&contact, bool bCreate=true, bool bJoin=true);
    QTimer  *m_timer;
    virtual bool error_state(const QString &err, unsigned code);
    bool add(const QString &name);
    LiveJournalUserData* toLiveJournalUserData(SIM::clientData * data); // More safely type conversion from generic SIM::clientData into LiveJournalUserData
public slots:
    void timeout();
    void send();
    void messageUpdated();
protected:
    virtual bool done(unsigned code, Buffer &data, const QString &headers);
    virtual QByteArray getConfig();
    virtual QString name();
    virtual QString dataName(void*);
    virtual QWidget	*setupWnd();
    virtual bool isMyData(SIM::clientData*&, SIM::Contact*&);
    virtual bool createData(SIM::clientData*&, SIM::Contact*);
    virtual void setupContact(SIM::Contact*, void *data);
    virtual bool send(SIM::Message*, void *data);
    virtual bool canSend(unsigned type, void *data);
    virtual void setStatus(unsigned status);
    virtual void socketConnect();
    virtual void disconnected();
    virtual void packet_ready();
    virtual bool processEvent(SIM::Event *e);
    virtual void contactInfo(void*, unsigned long &curStatus, unsigned&, QString &statusIcon, QSet<QString> *icons);
    QWidget *searchWindow(QWidget *parent);
    SIM::CommandDef *configWindows();
    QWidget *configWindow(QWidget *parent, unsigned id);
    void statusChanged();
    std::list<LiveJournalRequest*> m_requests;
    LiveJournalRequest		  *m_request;
    LiveJournalClientData	data;
    friend class LiveJournalCfg;
    friend class LiveJournalRequest;
};

#endif


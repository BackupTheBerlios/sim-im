/***************************************************************************
                          sms.h  -  description
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

#ifndef _SMS_H
#define _SMS_H

#include "contacts/client.h"
#include "log.h"
#include "socket/socket.h"
#include "socket/tcpclient.h"

class SMSProtocol;
class GsmTA;
class MainInfo;

const unsigned long SMSCmdBase			= 0x00080000;
const unsigned long MessagePhoneCall		= SMSCmdBase;

class SMSPlugin : public QObject, public SIM::Plugin
{
    Q_OBJECT
public:
    SMSPlugin(unsigned);
    virtual ~SMSPlugin();
    static unsigned SerialPacket;
    void setPhoneCol();
    void setPhoneCol(MainInfo *w);
    void removePhoneCol();
    void removePhoneCol(MainInfo *w);
protected:
    bool eventFilter(QObject *obj, QEvent *e);
    SMSProtocol *m_protocol;
};

class SMSProtocol : public SIM::Protocol
{
public:
    SMSProtocol(SIM::Plugin *plugin);
    ~SMSProtocol();
    SIM::ClientPtr	createClient(Buffer *cfg);
    const SIM::CommandDef *description();
    const SIM::CommandDef *statusList();
    const SIM::DataDef *userDataDef();
};

struct SMSClientData
{
    SIM::Data	Device;
    SIM::Data	BaudRate;
    SIM::Data	XonXoff;
    SIM::Data	Charge;
    SIM::Data	Charging;
    SIM::Data	Quality;
};

const unsigned SMS_SIGN	= 6;

struct smsUserData : public SIM::clientData
{
    SIM::Data	Name;
    SIM::Data	Phone;
    SIM::Data	Index;
    SIM::Data	Type;
};

class SMSClient : public SIM::TCPClient
{
    Q_OBJECT
public:
    SMSClient(SIM::Protocol *protocol, Buffer *cfg);
    ~SMSClient();
    PROP_STR(Device);
    PROP_ULONG(BaudRate);
    PROP_BOOL(XonXoff);
    PROP_ULONG(Charge);
    PROP_BOOL(Charging);
    PROP_ULONG(Quality);
    QByteArray model() const;
    QByteArray oper() const;
    smsUserData* tosmsUserData(SIM::clientData * data);
protected slots:
    void error();
    void init();
    void ta_error();
    void charge(bool, unsigned);
    void quality(unsigned);
    void phoneCall(const QString&);
    void phonebookEntry(int, int, const QString&, const QString&);
    void callTimeout();
protected:
    virtual QString         getServer() const;
    virtual unsigned short  getPort() const;
    virtual void	setStatus(unsigned status);
    virtual void	disconnected();
    virtual QByteArray getConfig();
    virtual QString name();
    virtual QString dataName(void*);
    virtual bool	isMyData(SIM::clientData*&, SIM::Contact*&);
    virtual bool	createData(SIM::clientData*&, SIM::Contact*);
    virtual void	setupContact(SIM::Contact*, void *data);
    virtual bool	send(SIM::Message*, void *data);
    virtual bool	canSend(unsigned type, void *data);
    virtual void	packet_ready();
    virtual void	socketConnect();
    virtual SIM::CommandDef *configWindows();
    virtual QWidget *configWindow(QWidget *parent, unsigned id);
    virtual QWidget	*setupWnd();
    virtual QWidget *searchWindow(QWidget*);
    virtual void contactInfo(void *,unsigned long &,unsigned int &,QString &,QSet<QString> *) {}
    QString			m_callNumber;
    QTimer			*m_callTimer;
    SIM::Message	*m_call;
    bool			m_bCall;
    GsmTA			*m_ta;
    SMSClientData	data;
};

#endif


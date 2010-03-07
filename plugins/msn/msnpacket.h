/***************************************************************************
                          msnpacket.h  -  description
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

#ifndef _MSNPACKET_H
#define _MSNPACKET_H

#include "msnclient.h"

#include <QByteArray>

class MSNPacket
{
public:
    MSNPacket(MSNClient *client, const QString &cmd);
    virtual ~MSNPacket(){}
    const QString &cmd() const { return m_cmd; }
    unsigned id()        const { return m_id; }
    virtual void        answer(const QStringList&) {};
    virtual void        error(unsigned code);
    void                addArg(const QString &str);
    void                addArg(const char *str);
    virtual void        send();
protected:
    QString     m_line;
    QString     m_cmd;
    MSNClient  *m_client;
    unsigned    m_id;
};

class VerPacket : public MSNPacket
{
public:
    VerPacket(MSNClient *client);
    void answer(const QStringList &args);
};

class CvrPacket : public MSNPacket
{
public:
    CvrPacket(MSNClient *client);
    void answer(const QStringList &args);
};

class UsrPacket : public MSNPacket
{
public:
    UsrPacket(MSNClient *client, const QString &hash = QString::null);
    void answer(const QStringList &args);
};

class OutPacket : public MSNPacket
{
public:
    OutPacket(MSNClient *client);
};

class ChgPacket : public MSNPacket
{
public:
    ChgPacket(MSNClient *client);
};

class SynPacket : public MSNPacket
{
public:
    SynPacket(MSNClient *client);
    void answer(const QStringList &args);
};

class QryPacket : public MSNPacket
{
public:
    QryPacket(MSNClient *client, const QString &qry);
    virtual void send();
};

class AdgPacket : public MSNPacket
{
public:
    AdgPacket(MSNClient *client, unsigned grp_id, const QString &name);
    void answer(const QStringList &args);
protected:
    unsigned m_id;
};

class RegPacket : public MSNPacket
{
public:
    RegPacket(MSNClient *client, unsigned id, const QString &name);
};

class RmgPacket : public MSNPacket
{
public:
    RmgPacket(MSNClient *client, unsigned long id);
};

class AddPacket : public MSNPacket
{
public:
    AddPacket(MSNClient *client, const QString &listType, const QString &mail, const QString &name, unsigned grp=0);
    void answer(const QStringList &args);
    virtual void	error(unsigned code);
protected:
    QString m_mail;
};

class RemPacket : public MSNPacket
{
public:
    RemPacket(MSNClient *client, const QString &listType, const QString &mail, unsigned group=NO_GROUP);
};

class ReaPacket : public MSNPacket
{
public:
    ReaPacket(MSNClient *client, const QString &mail, const QString &name);
    virtual void	error(unsigned code);
};

class BlpPacket : public MSNPacket
{
public:
    BlpPacket(MSNClient *client);
};

class XfrPacket : public MSNPacket
{
public:
    XfrPacket(MSNClient *client, SBSocket *socket);
    void clear();
    void answer(const QStringList &args);
protected:
    SBSocket *m_socket;
};

class MSNServerMessage
{
public:
    MSNServerMessage(MSNClient *client, unsigned size);
    ~MSNServerMessage();
    bool packet();
protected:
    QByteArray  m_msg;
    MSNClient  *m_client;
    unsigned    m_size;
};

#endif


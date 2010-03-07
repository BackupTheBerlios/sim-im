/***************************************************************************
                          jabbermessage.h  -  description
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

#ifndef _JABBERMESSAGE_H
#define _JABBERMESSAGE_H

#include "cfg.h"
#include "message.h"

#include <QByteArray>

const unsigned long MessageJabber		= 0x201;
const unsigned long MessageJabberOnline	= 0x202;
const unsigned long MessageJabberOffline	= 0x203;
const unsigned long MessageJabberError	= 0x204;

struct JabberMessageData
{
    SIM::Data	Subject;
};

class JabberMessage : public SIM::Message
{
public:
    JabberMessage(Buffer *cfg = NULL);
    ~JabberMessage();
    PROP_UTF8(Subject);
    virtual QByteArray save();
    virtual QString presentation();
    virtual unsigned baseType() { return SIM::MessageGeneric; }
protected:
    JabberMessageData	data;
};

struct JabberMessageErrorData
{
    SIM::Data	Error;
    SIM::Data	Code;
};

class JabberMessageError : public SIM::Message
{
public:
    JabberMessageError(Buffer *cfg = NULL);
    ~JabberMessageError();
    PROP_UTF8(Error);
    PROP_ULONG(Code);
    virtual QByteArray save();
    virtual QString presentation();
protected:
    JabberMessageErrorData	data;
};

struct JabberMessageFileData
{
    SIM::Data	ID;
    SIM::Data	From;
    SIM::Data	Host;
    SIM::Data	Port;
};

class JabberFileMessage : public SIM::FileMessage
{
public:
    JabberFileMessage(Buffer *cfg = NULL);
    ~JabberFileMessage();
    PROP_STR(ID);
    PROP_STR(From);
    PROP_STR(Host);
    PROP_USHORT(Port);
    virtual unsigned baseType() { return SIM::MessageFile; }
protected:
    JabberMessageFileData	data;
};

#endif


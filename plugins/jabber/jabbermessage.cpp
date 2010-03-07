/***************************************************************************
                          jabbermessage.cpp  -  description
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

#include "jabbermessage.h"
#include "jabber.h"
#include "core.h"

#include <QByteArray>

using namespace SIM;

static DataDef jabberMessageData[] =
    {
        { "Subject", DATA_UTF, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

JabberMessage::JabberMessage(Buffer *cfg)
        : Message(MessageJabber, cfg)
{
    load_data(jabberMessageData, &data, cfg);
}

JabberMessage::~JabberMessage()
{
    free_data(jabberMessageData, &data);
}

QByteArray JabberMessage::save()
{
    QByteArray res = Message::save();
    QByteArray s = save_data(jabberMessageData, &data);
    if (!s.isEmpty()){
        if (!res.isEmpty())
            res += '\n';
        res += s;
    }
    return res;
}

QString JabberMessage::presentation()
{
    QString res = i18n("<p>Subject: %1</p>")
                  .arg(getSubject());
    res += Message::presentation();
    return res;
}

static Message *createJabberMessage(Buffer *cfg)
{
    return new JabberMessage(cfg);
}

static MessageDef defJabber =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        NULL,
        NULL,
        createJabberMessage,
        NULL,
        NULL
    };

static DataDef jabberMessageErrorData[] =
    {
        { "Error", DATA_UTF, 1, 0 },
        { "Code", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

JabberMessageError::JabberMessageError(Buffer *cfg)
        : Message(MessageJabberError, cfg)
{
    load_data(jabberMessageErrorData, &data, cfg);
}

JabberMessageError::~JabberMessageError()
{
    free_data(jabberMessageErrorData, &data);
}

QByteArray JabberMessageError::save()
{
    QByteArray res = Message::save();
    QByteArray s = save_data(jabberMessageErrorData, &data);
    if (!s.isEmpty()){
        if (!res.isEmpty())
            res += '\n';
        res += s;
    }
    return res;
}

QString JabberMessageError::presentation()
{
    QString res = "<p>";
    res += i18n("Error");
    if (getCode()){
        res += ' ';
        res += QString::number(getCode());
    }
    QString err = getError();
    if (!err.isEmpty()){
        res += ": <b>";
        res += err;
        res += "</b>";
    }
    res += "<br/>";
    res += i18n("Original message:");
    res += "</p>";
    res += Message::presentation();
    return res;
}

static Message *createJabberMessageError(Buffer *cfg)
{
    return new JabberMessageError(cfg);
}

#if 0
i18n("Error", "%n errors", 1);
#endif

static MessageDef defJabberError =
    {
        NULL,
        NULL,
        MESSAGE_DEFAULT,
        "Error",
        "%n errors",
        createJabberMessageError,
        NULL,
        NULL
    };

static Message *createJabberOnlineMessage(Buffer *cfg)
{
    return new AuthMessage(MessageJabberOnline, cfg);
}

static MessageDef defJabberOnline =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_SENDONLY,
        I18N_NOOP("Log On"),
        NULL,
        createJabberOnlineMessage,
        NULL,
        NULL
    };

static Message *createJabberOfflineMessage(Buffer *cfg)
{
    return new AuthMessage(MessageJabberOffline, cfg);
}

static MessageDef defJabberOffline =
    {
        NULL,
        NULL,
        MESSAGE_SILENT | MESSAGE_SENDONLY,
        I18N_NOOP("Log Off"),
        NULL,
        createJabberOfflineMessage,
        NULL,
        NULL
    };

static DataDef jabberMessageFileData[] =
    {
        { "", DATA_STRING, 1, 0 },		// ID
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },		// Host
        { "", DATA_ULONG, 1, 0 },		// Port
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

JabberFileMessage::JabberFileMessage(Buffer *cfg)
        : FileMessage(MessageFile, cfg)
{
    load_data(jabberMessageFileData, &data, cfg);
}

JabberFileMessage::~JabberFileMessage()
{
    free_data(jabberMessageFileData, &data);
}

void JabberPlugin::registerMessages()
{
    Command cmd;
    cmd->id			 = MessageJabber;
    cmd->text		 = "Jabber";
    cmd->icon		 = "message";
    cmd->flags		 = COMMAND_DEFAULT;
    cmd->param		 = &defJabber;
    EventCreateMessageType(cmd).process();

    cmd->id			 = MessageJabberOnline;
    cmd->text		 = I18N_NOOP("Log On");
    cmd->icon		 = "Jabber_online";
    cmd->menu_grp	 = 0x3020;
    cmd->param		 = &defJabberOnline;
    EventCreateMessageType(cmd).process();

    cmd->id			 = MessageJabberOffline;
    cmd->text		 = I18N_NOOP("Log Off");
    cmd->icon		 = "Jabber_offline";
    cmd->param		 = &defJabberOffline;
    EventCreateMessageType(cmd).process();

    cmd->id			 = MessageJabberError;
    cmd->text		 = I18N_NOOP("Error");
    cmd->icon		 = "error";
    cmd->param		 = &defJabberError;
    EventCreateMessageType(cmd).process();
}

void JabberPlugin::unregisterMessages()
{
    EventRemoveMessageType(MessageJabber).process();
    EventRemoveMessageType(MessageJabberOnline).process();
    EventRemoveMessageType(MessageJabberOffline).process();
    EventRemoveMessageType(MessageJabberError).process();
}


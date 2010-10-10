/***************************************************************************
                          contacts.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _CONTACTS_H
#define _CONTACTS_H

#include "cfg.h"
#include "plugins.h"
#include "contacts/userdata.h"
#include "contacts/clientuserdata.h"
#include "contacts/packettype.h"
#include "contacts/protocol.h"
#include "misc.h"
#include "propertyhub.h"

#include <map>
#include <QImage>
#include <QString>
#include <QSet>
#include <QDomDocument>

namespace SIM {

class Client;
class ClientDataIterator;


const unsigned PHONE    = 0;
const unsigned FAX      = 1;
const unsigned CELLULAR = 2;
const unsigned PAGER    = 3;

const unsigned STATUS_UNKNOWN   = 0;
const unsigned STATUS_OFFLINE   = 1;
const unsigned STATUS_INVISIBLE	= 2;
const unsigned STATUS_DND       = 10;
const unsigned STATUS_OCCUPIED  = 20;
const unsigned STATUS_NA        = 30;
const unsigned STATUS_AWAY      = 40;
const unsigned STATUS_ONLINE    = 50;
const unsigned STATUS_FFC       = 60;

const unsigned CONTACT_UNDERLINE    = 0x0001;
const unsigned CONTACT_ITALIC       = 0x0002;
const unsigned CONTACT_STRIKEOUT    = 0x0004;

const unsigned PROTOCOL_INFO            = 0x00010000;
const unsigned PROTOCOL_SEARCH          = 0x00020000;
const unsigned PROTOCOL_AR_OFFLINE      = 0x00040000;
const unsigned PROTOCOL_INVISIBLE       = 0x00080000;
const unsigned PROTOCOL_AR              = 0x00100000;
const unsigned PROTOCOL_AR_USER         = 0x00200000;
const unsigned PROTOCOL_ANY_PORT        = 0x00800000;
const unsigned PROTOCOL_NOSMS           = 0x01000000;
const unsigned PROTOCOL_NOPROXY         = 0x02000000;
const unsigned PROTOCOL_TEMP_DATA       = 0x04000000;
const unsigned PROTOCOL_NODATA          = 0x08000000;
const unsigned PROTOCOL_NO_AUTH         = 0x10000000;

struct ENCODING
{
    const char *language;
    const char *codec;
    int         mib;
    int         rtf_code;
    int         cp_code;
    bool        bMain;
};

QString addString(const QString &oldValue, const QString &newValue, const QString &client);
QString addStrings(const QString &old_value, const QString &values, const QString &client);

typedef std::map<unsigned, PacketType*>	PACKET_MAP;

} // namespace SIM

// vim: set expandtab:

#endif

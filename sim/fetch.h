/***************************************************************************
                          fetch.h  -  description
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

#ifndef _FETCH_H
#define _FETCH_H

#include <QString>

#include "simapi.h"     // COPY_RESTRICTED

class Buffer;

const unsigned NO_POSTSIZE	= (unsigned)(-1);

class FetchClientPrivate;

class EXPORT FetchClient
{
public:
    FetchClient();
    virtual ~FetchClient();
    void fetch(const QString &url, const QString &headers = QString::null, Buffer *postData = NULL, bool bRedirect = true);
    virtual const char *read_data(char *buf, unsigned &size);
    virtual bool write_data(const char *buf, unsigned size);
    virtual unsigned post_size();
    virtual bool done(unsigned code, Buffer &data, const QString &headers) = 0;
    bool isDone();
    void stop();
    void set_speed(unsigned speed);
    static bool	crackUrl(const QString &url, QString &proto, QString &host, unsigned short &port, QString &user, QString &pass, QString &uri, QString &extra);
    void clearCookies();
private:
    FetchClientPrivate *p;
    friend class FetchClientPrivate;

    COPY_RESTRICTED(FetchClient)
};

EXPORT QByteArray basic_auth(const QString &user, const QString &pass);
EXPORT bool get_connection_state(bool &state);
EXPORT QString get_user_agent();

#endif


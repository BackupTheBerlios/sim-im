/***************************************************************************
                          socket.cpp  -  description
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

#include <QMutex>
#include <QTimer>
#include <QSet>
#include <QTimerEvent>

#ifdef WIN32
	#include <winsock.h>
#else
#ifndef Q_OS_MAC
	#include <net/if.h>
#endif
	#include <sys/ioctl.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
#endif


#include "socket.h"
#include "socketfactory.h"
#include "misc.h"
#include "log.h"

namespace SIM
{

using namespace std;

#ifndef INADDR_NONE
#define INADDR_NONE	0xFFFFFFFF
#endif


void Socket::error(const QString &err_text, unsigned code)
{
    if (notify)
        notify->error_state(err_text, code);
}

}


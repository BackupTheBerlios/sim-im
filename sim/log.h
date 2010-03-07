/***************************************************************************
                          log.h  -  description
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

#ifndef _LOG_H
#define _LOG_H

#include "simapi.h"

class QString;

namespace SIM {

const unsigned short L_ERROR        = 0x01;
const unsigned short L_WARN         = 0x02;
const unsigned short L_DEBUG        = 0x04;

const unsigned short L_PACKET_IN    = 0x10;
const unsigned short L_PACKET_OUT   = 0x20;

#if defined __GNUC__
EXPORT void log(unsigned short level, const char *fmt, ...)
__attribute__ ((__format__ (printf, 2, 3)));
#else
EXPORT void log(unsigned short level, const char *fmt, ...);
#endif      

/* QString counterpart of logging facility */
EXPORT void log(unsigned short level, const QString &msg);

EXPORT const char *level_name(unsigned short n);

} // namespace SIM

#endif


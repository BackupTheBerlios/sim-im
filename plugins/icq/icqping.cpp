/***************************************************************************
                          icqping.cpp  -  description
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

#include "log.h"

#include "icqclient.h"

using namespace SIM;

const unsigned short ICQ_SNACxPING_ERROR            = 0x0001;
const unsigned short ICQ_SNACxPING_REPORTxINTERVALL = 0x0002;
const unsigned short ICQ_SNACxPING_STATS            = 0x0003;   // not implemented
const unsigned short ICQ_SNACxPING_STATSxACK        = 0x0004;   // not implemented

void ICQClient::snac_ping(unsigned short type, unsigned short)
{
    switch (type){
    case ICQ_SNACxPING_ERROR:
        break;
    case ICQ_SNACxPING_REPORTxINTERVALL:
        break;
    default:
        log(L_WARN, "Unknown ping foodgroup type %04X", type);
    }
}


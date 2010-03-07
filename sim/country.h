/***************************************************************************
                          country.h  -  description
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

#ifndef _COUNTRY_H
#define _COUNTRY_H

#include "simapi.h"

namespace SIM {

// _____________________________________________________________________________________
// Data

struct ext_info
{
    const char *szName;
    unsigned short nCode;
};

EXPORT const ext_info *getCountries();
EXPORT const ext_info *getCountryCodes();

struct pager_provider
{
    const char *szName;
    const char *szGate;
};

EXPORT const pager_provider *getProviders();

} // namespace SIM

#endif

/***************************************************************************
                          unquot.h  -  description
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

#ifndef _UNQUOT_H
#define _UNQUOT_H

#include "simapi.h"

class QString;
namespace SIM {

enum quoteMode
{
    quoteHTML,
    quoteXML,
    quoteXMLattr,
    quoteNOBR
};

EXPORT QString  unquoteText(const QString &text);
EXPORT QString  quoteString(const QString &str, quoteMode mode = quoteHTML, bool bQuoteSpaces = true);
EXPORT QString  unquoteString(const QString &str);
EXPORT QString  quote_nbsp(const QString &str);

} // namespace SIM

#endif


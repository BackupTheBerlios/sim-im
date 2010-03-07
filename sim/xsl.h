/***************************************************************************
                          xsl.h  -  description
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

#ifndef _XSL_H
#define _XSL_H

#include "simapi.h"

class QString;
class EXPORT XSL
{
public:
    XSL(const QString &name);
    ~XSL();
    void setXSL(const QString &xsl);
    QString process(const QString &xml);
    static void cleanup();
protected:
    class XSLPrivate *d;

    COPY_RESTRICTED(XSL)
};

#endif


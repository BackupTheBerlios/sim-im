/***************************************************************************
                          html.h  -  description
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

#ifndef _HTML_H
#define _HTML_H	1

#include "simapi.h"
#include <list>

class Buffer;
class QString;

namespace SIM
{

class EXPORT HTMLParser
{
public:
    HTMLParser();
    virtual ~HTMLParser();
    void parse(const QString &str);
    void parse(Buffer &buf);
    static std::list<QString> parseStyle(const QString &str);
    static QString makeStyle(const std::list<QString> &opt);
protected:
    void parse();
    unsigned start_pos;
    unsigned end_pos;
    virtual void text(const QString &text) = 0;
    virtual void tag_start(const QString &tag, const std::list<QString> &options) = 0;
    virtual void tag_end(const QString &tag) = 0;
    class HTMLParserPrivate *p;
    friend class HTMLParserPrivate;

    COPY_RESTRICTED(HTMLParser)
};

}

#endif

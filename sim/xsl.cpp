/***************************************************************************
                          xsl.cpp  -  description
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

/* before xsl.h because auf STL - problem !! */
//#include <libxslt/xsltInternals.h>
#include <libxslt/xslt.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

#include <QFile>
#include <QTextStream>

#include "log.h"
#include "misc.h"
#include "unquot.h"
#include "xsl.h"

using namespace SIM;

class XSLPrivate
{
public:
    XSLPrivate(const QString &my_xsl);
    ~XSLPrivate();
    xsltStylesheetPtr styleSheet;
    xmlDocPtr doc;
};

XSLPrivate::XSLPrivate(const QString &my_xsl)
{
    styleSheet = NULL;
    xmlSubstituteEntitiesDefault(1);
    xmlLoadExtDtdDefaultValue = 1;
    const QByteArray xslUtf8 = my_xsl.toUtf8();
    doc = xmlParseMemory(xslUtf8.data(), xslUtf8.length());
    if (doc == NULL){
        xmlErrorPtr ptr = xmlGetLastError();
        log(L_ERROR, "Can't parse XSLT (%s)", ptr ? ptr->message : "");
        return;
    }
    styleSheet = xsltParseStylesheetDoc(doc);
}

XSLPrivate::~XSLPrivate()
{
    if (styleSheet)
        xsltFreeStylesheet(styleSheet);
}

static char STYLES[] = "styles/";
static char EXT[]    = ".xsl";

XSL::XSL(const QString &name)
{
    QString fname = QString(STYLES).append(name).append(EXT);
    QFile f(user_file(fname));
    bool bOK = true;
    if (f.size() == 0 || !f.open(QIODevice::ReadOnly)){
        f.setFileName(app_file(fname));
        if (f.size() == 0 || !f.open(QIODevice::ReadOnly)){
            log(L_WARN, "Can't open / empty file %s", qPrintable(f.fileName()));
            bOK = false;
        }
    }
    QString xsl;
    if(bOK){
        QTextStream ts(&f);
        xsl = ts.readAll();
    }
    d = new XSLPrivate(xsl);
}

XSL::~XSL()
{
    delete d;
}

void XSL::setXSL(const QString &my_xsl)
{
    delete d;
    d = new XSLPrivate(my_xsl);
}

QString XSL::process(const QString &my_xml)
{
    QString my_xsl;
    /* Petr Cimprich, Sablot developer:
       &nbsp; is predefined in HTML but not in XML
       ... use Unicode numerical entity instead: &#160;*/
    my_xsl = quote_nbsp(my_xml);

    const QByteArray xslUtf8 = my_xsl.toUtf8();
    xmlDocPtr doc = xmlParseMemory(xslUtf8.data(), xslUtf8.length());
    if (doc == NULL){
        xmlErrorPtr ptr = xmlGetLastError();
        log(L_WARN, "Parse XML error (%s): %s", ptr ? ptr->message : "", qPrintable(my_xsl));
        return QString(ptr ? ptr->message : "Parse XML error!");
    }
    const char *params[1];
    params[0] = NULL;
    xmlDocPtr res = xsltApplyStylesheet(d->styleSheet, doc, params);
    xmlFreeDoc(doc);
    if (res == NULL){
        xmlErrorPtr ptr = xmlGetLastError();
        log(L_WARN, "Apply stylesheet error (%s)", ptr ? ptr->message : "");
        return QString(ptr ? ptr->message : "Apply stylesheet error!");
    }

    xmlOutputBufferPtr buf = xmlAllocOutputBuffer(NULL);
    xsltSaveResultTo(buf, res, d->styleSheet);
// Strange crash on MacOS X in libxsl !
//    xmlFreeDoc(res);

    QString result = QString::fromUtf8((char*)(buf->buffer->content));
    xmlOutputBufferClose(buf);

    return result;
}

void XSL::cleanup()
{
    xsltCleanupGlobals();
}

/***************************************************************************
                          sax.cpp  -  description
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

#include "sax.h"
#include "log.h"

class SAXParserPrivate: public QXmlDefaultHandler
{
public:
    SAXParserPrivate(SAXParser *parser);
    bool parse(const QByteArray& data);
    QXmlSimpleReader m_reader;
    QXmlInputSource m_source;
    SAXParser *m_parser;
    bool startElement(const QString&, 
                      const QString&, 
                      const QString &qName,
                      const QXmlAttributes &attribs);
    bool endElement(const QString&,
                    const QString&,
                    const QString &qName);
    bool characters(const QString &str);
    bool fatalError(const QXmlParseException & exception);
private:
	bool m_started;
};

SAXParserPrivate::SAXParserPrivate(SAXParser *parser)
{
    m_parser = parser;
    m_reader.setFeature("http://xml.org/sax/features/namespace-prefixes", TRUE);
    m_reader.setContentHandler(this);
    m_reader.setErrorHandler(this);
	m_started = false;
    //m_reader.parse(&m_source, true);
}

bool SAXParserPrivate::parse(const QByteArray& data)
{
	if(data.count() > 0)
	{
		m_source.setData(data);
		if(m_started)
		{
			return m_reader.parseContinue();
		}
		else
		{
			m_started = true;
			return m_reader.parse(&m_source, true);
		}
	}
	return true;
}

bool SAXParserPrivate::startElement(const QString&,
                                    const QString&,
                                    const QString &qName,
                                    const QXmlAttributes &attribs)
{
    m_parser->element_start(qName, attribs);
    return true;
}

bool SAXParserPrivate::endElement(const QString&,
                                  const QString&,
                                  const QString &qName)
{
    m_parser->element_end(qName);
    return true;
}

bool SAXParserPrivate::characters(const QString &str)
{
    m_parser->char_data(str);
    return true;
}
    
bool SAXParserPrivate::fatalError(const QXmlParseException & exception)
{
    SIM::log(SIM::L_DEBUG, "XML parse error: %s", qPrintable(exception.message()));
    return false;
}

SAXParser::SAXParser()
{
    p = NULL;
}

SAXParser::~SAXParser()
{
    reset();
}

void SAXParser::reset()
{
    if (p)
	{
        delete p;
        p = NULL;
    }
}

bool SAXParser::parse(const QByteArray& data, bool bChunk)
{
    if (!bChunk)
        reset();
    if (p == NULL)
        p = new SAXParserPrivate(this);
    if (!p->parse(data))
	{
        reset();
        return false;
    }
    if (!bChunk)
        reset();
    return true;
}


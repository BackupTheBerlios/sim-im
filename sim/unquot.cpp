/***************************************************************************
                          unquote.cpp  -  description
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

#include <QRegExp>
#include <QDomDocument>

#include "html.h"
#include "icons.h"
#include "unquot.h"

using namespace std;
using namespace SIM;

class UnquoteParser : public SIM::HTMLParser
{
public:
    UnquoteParser();
    QString parse(const QString &str);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    QString res;
    bool m_bPar;
    bool m_bTD;
    bool m_bTR;
    bool m_bPre;
};

UnquoteParser::UnquoteParser()
{
}

QString GetTextFromElement( QDomElement el ) {
    QString text;
    QDomNode node = el.firstChild();
    while( !node.isNull() ) {
        switch( node.nodeType() ) {
            case QDomNode::TextNode : {
                QDomCharacterData data = node.toCharacterData();
                text += data.data();
                break;
            }
            case QDomNode::ElementNode : {
                text += GetTextFromElement( node.toElement() );
                break;
            }
			default:
				break;
        }
        node = node.nextSibling();
    }
    QString sName = el.nodeName();
    if( sName == "p" || sName == "br" ) {
        text += "\n";
    }
    else if( sName == "img" ) {
        QString src = el.attribute( "src" );
        if (src.startsWith("sim:icons/")){
            QStringList smiles = getIcons()->getSmile(src.mid(10));
            if (!smiles.empty()){
                text += smiles.front();
            }
        }
    }
    return text;
}

QString UnquoteParser::parse(const QString &str)
{
    QDomDocument doc;
    doc.setContent( str );
    QDomElement el = doc.firstChildElement( "html" );
    if( el.isNull() )
        return str;
    el = el.firstChildElement( "body" );
    if( el.isNull() )
        return str;
/*
    res = QString::null;
    m_bPar = false;
    m_bTD  = false;
    m_bTR  = false;
    m_bPre = true;
    HTMLParser::parse(str);
*/
    QString sText = GetTextFromElement( el );
    if( sText.right(1) == "\n" ) {
        sText = sText.left( sText.length() - 1 );
    }
    return sText;
}

void UnquoteParser::text(const QString &text)
{
    int len = text.length();
    if (len)
        m_bPre = false;
    for (int i = 0; i < len; i++){
        QChar c = text[i];
        if (c.unicode() == 160){
            res += ' ';
        }else{
            res += c;
        }
    }
}

void UnquoteParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "pre"){
        if (!m_bPre)
            res += '\n';
    }else if (tag == "br"){
        res += '\n';
    }else if (tag == "hr"){
        if (!res.isEmpty() && (res[(int)(res.length() - 1)] != '\n'))
            res += '\n';
        res += "---------------------------------------------------\n";
    }else if (tag == "td"){
        if (m_bTD){
            res += '\t';
            m_bTD = false;
        }
    }else if (tag == "tr"){
        if (m_bTR){
            res += '\n';
            m_bTR = false;
        }
    }else if (tag == "p"){
        if (m_bPar){
            res += '\n';
            m_bPar = false;
        }
    }else if (tag == "img"){
        QString src;
        QString alt;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString opt   = *it;
            ++it;
            QString value = *it;
            if (opt == "src")
                src = value;
            if (opt == "alt")
                alt = value;
        }
        if (!alt.isEmpty()){
            res += unquoteString(alt);
            return;
        }
        if (src.startsWith("sim:icons/")){
            QStringList smiles = getIcons()->getSmile(src.mid(10));
            if (!smiles.empty()){
                res += smiles.front();
                return;
            }
        }
        text(alt);
    }
}

void UnquoteParser::tag_end(const QString &tag)
{
    if (tag == "pre"){
        res += '\n';
        m_bPre = true;
    }
    if (tag == "p")
        m_bPar = true;
    if (tag == "td"){
        m_bPar = false;
        m_bTD  = true;
    }
    if (tag == "tr"){
        m_bPar = false;
        m_bTD  = false;
        m_bTR  = true;
    }
    if (tag == "table"){
        m_bPar = true;
        m_bTD  = false;
        m_bTR  = false;
    }
}

QString SIM::unquoteText(const QString &text)
{
    UnquoteParser p;
    return p.parse(text);
}

QString SIM::unquoteString(const QString &text)
{
    QString res = text;
    res = res.replace("&gt;",   ">");
    res = res.replace("&lt;",   "<");
    res = res.replace("&quot;", "\"");
    res = res.replace("&amp;",  "&");
    res = res.replace("&nbsp;", " ");
    res = res.replace("<br/?>", "\n");
    return res;
}

EXPORT QString SIM::quoteString(const QString &_str, quoteMode mode, bool bQuoteSpaces)
{
    QString str = _str;
    str.replace("&", "&amp;");
    str.replace("<", "&lt;");
    str.replace(">", "&gt;");
    str.replace("\"", "&quot;");
    str.replace("\r", QString::null);
    str.replace("\t", "&nbsp;&nbsp;");
    switch (mode){
    case quoteHTML:
        str.replace("\n", "<br>\n");
        break;
    case quoteXML:
        str.replace("\n", "<br/>\n");
        break;
    case quoteXMLattr:
        str.replace("'", "&apos;");
        break;
    default:
        break;
    }
    if(!bQuoteSpaces)
        return str;
    QRegExp re("  +");
    int pos = 0;
    while ((pos = re.indexIn(str, pos)) != -1) {
        int len = re.matchedLength();
        if (len == 1)
            continue;
        QString s = " ";
        for (int i = 1; i < len; i++)
            s += "&nbsp;";
        str.replace(pos, len, s);
    }
    return str;
}

EXPORT QString SIM::quote_nbsp(const QString &str)
{
    QString s = str;
    return s.replace("&nbsp;", QString("&#160;"));
}   

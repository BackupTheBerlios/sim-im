/***************************************************************************
                          rtfgen.cpp  -  description
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

#ifdef WIN32
#include <windows.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stack>
#include <cctype>

#include <QTextCodec>
#include <QRegExp>

#include "html.h"
#include "icons.h"
#include "unquot.h"
#include "log.h"

#include "icqclient.h"
#include "polling.h"


#include "icqclient.h"

using namespace std;
using namespace SIM;

// Represents a character (as opposed to paragraph) style
struct CharStyle
{
    CharStyle() :
            colorIdx(-1),
            sizePt(-1),
            faceIdx(-1),
            bold(false),
            italic(false),
            underline(false),
            bgColorIdx(-1)
    {}

    // Returns the diff from 'old' CharStyle to this CharStyle as RTF commands.
    QString getDiffRTF(const CharStyle& old) const;

    bool operator==(const CharStyle& that)
    {
        return (colorIdx == that.colorIdx &&
                sizePt == that.sizePt &&
                faceIdx == that.faceIdx &&
                bold == that.bold &&
                italic == that.italic &&
                underline == that.underline &&
                bgColorIdx == that.bgColorIdx);
    }

    bool operator!=(const CharStyle& that)
    {
        return !(*this == that);
    }

    signed colorIdx;
    signed sizePt; // size in points
    signed faceIdx;
    bool bold;
    bool italic;
    bool underline;
    signed bgColorIdx;
};

QString CharStyle::getDiffRTF(const CharStyle& old) const
{
    QString rtf;

    if (old.colorIdx != colorIdx)
    {
        rtf += QString("\\cf%1").arg(colorIdx);
    }
    if (old.sizePt != sizePt)
    {
        rtf += QString("\\fs%1").arg(sizePt * 2);
    }
    if (old.faceIdx != faceIdx)
    {
        rtf += QString("\\f%1").arg(faceIdx);
    }
    if (old.bold != bold)
    {
        rtf += QString("\\b%1").arg(bold ? 1 : 0);
    }
    if (old.italic != italic)
    {
        rtf += QString("\\i%1").arg(italic ? 1 : 0);
    }
    if (old.underline != underline)
    {
        rtf += QString("\\ul%1").arg(underline ? 1 : 0);
    }
    if (old.bgColorIdx != bgColorIdx)
    {
        rtf += QString("\\highlight%1").arg(bgColorIdx);
    }

    return rtf;
}

class Tag
{
public:
    Tag()
: pCharStyle(NULL)
    {
    }

    ~Tag()
    {
        if (pCharStyle != NULL)
            delete pCharStyle;
    }

    void setCharStyle(const CharStyle& charStyle)
    {
        if (pCharStyle == NULL)
            pCharStyle = new CharStyle();
        *pCharStyle = charStyle;
    }

    bool hasCharStyle()
    {
        return pCharStyle != NULL;
    }

public:
    QString name;
    CharStyle* pCharStyle;
};

class TagStack : private list<Tag>
{
public:
    TagStack()
    {
    }

    Tag* getTopTagWithCharStyle()
    {
        iterator it = end(), it_begin = begin();
        while(it != it_begin)
        {
            it--;
            if (it->hasCharStyle())
                return &(*it);
        }
        return NULL;
    }

    Tag* pushNew()
    {
        push_back(Tag());
        return &(back());
    }

    Tag* peek()
    {
        if (!empty())
            return &(back());
        else
            return NULL;
    }

    void pop()
    {
        pop_back();
    }
};

int htmlFontSizeToPt(int fontSize, int baseSize = 12)
{
    // Based on Qt's code (so we'd be compatible with QTextEdit)
    int pt;

    switch ( fontSize ) {
    case 1:
        pt =  7*baseSize/10;
        break;
    case 2:
        pt = (8 * baseSize) / 10;
        break;
    case 4:
        pt =  (12 * baseSize) / 10;
        break;
    case 5:
        pt = (15 * baseSize) / 10;
        break;
    case 6:
        pt = 2 * baseSize;
        break;
    case 7:
        pt = (24 * baseSize) / 10;
        break;
    default:
        pt = baseSize;
    }

    return pt;
}

class RTFGenParser : public HTMLParser
{
public:
    RTFGenParser(ICQClient *client, const QColor& foreColor, Contact *contact, unsigned max_size);
    QByteArray parse(const QString &text);
    // Returns the color's index in the colors table, adding the color if necessary.
    int getColorIdx(const QColor &color);
    // Returns the font face's index in the fonts table, adding the font face if necessary.
    int getFontFaceIdx(const QString &fontFace);
    unsigned textPos;
    stack<QString> tags;
    stack<QString> options;
    unsigned	m_res_size;
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
    QByteArray res;
    ICQClient  *m_client;
    Contact    *m_contact;
    QTextCodec *m_codec;
    bool        m_bSpace;
    unsigned	m_max_size;

    TagStack m_tags;

    // Used to compose RTF tables of unique font names and colors.
    list<QString> m_fontFaces;
    list<QColor>  m_colors;

    QColor m_foreColor;
    // The character position in 'res' string of the last paragraph start
    // (right after \par). Useful for inserting paragraph formatting
    // after you wrote past the paragraph mark.
    unsigned m_lastParagraphPos;
    enum {
        DirUnknown, // Initial BiDi dir; if not explicitly specified,
        // its determined from the first "strong character".
        DirLTR,
        DirRTL
    } m_paragraphDir;
};

RTFGenParser::RTFGenParser(ICQClient *client, const QColor& foreColor, Contact *contact, unsigned max_size)
{
    m_client    = client;
    m_contact   = contact;
    m_foreColor = foreColor;
    m_max_size	= max_size;
    m_lastParagraphPos = 0;
    m_paragraphDir = DirUnknown;
}

#ifdef WIN32

struct rtf_cp
{
    unsigned cp;
    unsigned charset;
};

rtf_cp rtf_cps[] =
    {
        { 737, 161 },
        { 855, 204 },
        { 857, 162 },
        { 862, 177 },
        { 864, 180 },
        { 866, 204 },
        { 869, 161 },
        { 875, 161 },
        { 932, 128 },
        { 1026, 162 },
        { 1250, 238 },
        { 1251, 204 },
        { 1253, 161 },
        { 1254, 162 },
        { 1255, 177 },
        { 0, 0 }
    };

#endif

int RTFGenParser::getColorIdx(const QColor& color)
{
    int i = 0;
    for (list<QColor>::iterator it = m_colors.begin(); it != m_colors.end(); ++it, i++){
        if ((*it) == color)
            return i + 1;
    }
    m_colors.push_back(color);
    return m_colors.size(); // the colors table is 1-based
}

int RTFGenParser::getFontFaceIdx(const QString& fontFace)
{
    int i = 0;
    for (list<QString>::iterator it = m_fontFaces.begin(); it != m_fontFaces.end(); ++it, i++){
        if ((*it) == fontFace)
            return i;
    }
    m_fontFaces.push_back(fontFace);
    return m_fontFaces.size() - 1;
}

QByteArray RTFGenParser::parse(const QString &text)
{
    res.clear();
    m_res_size = 0;
    m_codec = getContacts()->getCodec(m_contact);
    int charset = 0;
    for (const ENCODING *c = getContacts()->getEncodings(); c->language; c++){
        if (!qstricmp(c->codec, m_codec->name())){
            charset = c->rtf_code;
            break;
        }
    }
#ifdef WIN32
    if ((charset == 0) && !qstricmp(m_codec->name(), "system")){
        char buff[256];
        int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, (char*)&buff, sizeof(buff));
        if (res){
            unsigned codepage = atol(buff);
            if (codepage){
                for (const rtf_cp *c = rtf_cps; c->cp; c++){
                    if (c->cp == codepage)
                        charset = c->charset;
                }
            }
        }
    }
#endif
    unsigned ansicpg = 0;
    const char *send_encoding = 0;
    m_codec = NULL;
    if (charset){
        for (const ENCODING *c = getContacts()->getEncodings(); c->language; c++){
            if ((c->rtf_code == charset) && c->bMain){
                send_encoding = c->codec;
                m_codec = getContacts()->getCodecByName(send_encoding);
                ansicpg = c->cp_code;
                break;
            }
        }
    }

    // Add defaults to the tables
    m_fontFaces.push_back("MS Sans Serif");
    m_colors.push_back(m_foreColor);
    // Create a "fake" tag which'll serve as the default style
    CharStyle style;
    style.faceIdx = 0;
    style.colorIdx = 1; // colors are 1-based (0 = default)
    style.sizePt = 12; // default according to Microsoft
    Tag& tag = *(m_tags.pushNew());
    tag.setCharStyle(style);

    // Assume we go immediately after a tag.
    m_bSpace = true;
    HTMLParser::parse(text);

    QByteArray s;
    s = "{\\rtf1\\ansi";
    if (ansicpg){
        s += "\\ansicpg";
        s += QByteArray::number(ansicpg);
    }
    s += "\\deff0\r\n";
    s += "{\\fonttbl";
    unsigned n = 0;
    for (list<QString>::iterator it_face = m_fontFaces.begin(); it_face != m_fontFaces.end(); it_face++, n++){
        s += "{\\f";
        s += QByteArray::number(n);
        QString face = (*it_face);
        if (face.indexOf("Times") >= 0){
            s += "\\froman";
        }else if (face.indexOf("Courier") >= 0){
            s += "\\fmodern";
        }else{
            s += "\\fswiss";
        }
        if (charset){
            s += "\\fcharset";
            s += QByteArray::number(charset);
        }
        s += ' ';
        int pos = face.indexOf(QRegExp(" +["));
        if (pos > 0)
            face = face.left(pos);
        s += face.toLatin1();
        s += ";}";
    }
    s += "}\r\n";
    s += "{\\colortbl ;";
    for (list<QColor>::iterator it_colors = m_colors.begin(); it_colors != m_colors.end(); ++it_colors){
        QColor c = *it_colors;
        s += "\\red";
        s += QByteArray::number(c.red());
        s += "\\green";
        s += QByteArray::number(c.green());
        s += "\\blue";
        s += QByteArray::number(c.blue());
        s += ';';
    }
    s += "}\r\n";
    s += "\\viewkind4\\pard";
    s += style.getDiffRTF(CharStyle()).toUtf8();
    s += res;
    s += "\r\n}\r\n";

    log(L_DEBUG, "Resulting RTF: %s", s.data());

    return s;
}

void RTFGenParser::text(const QString &text)
{
    if (m_res_size)
        return;
    unsigned size = res.length();
    if (size > m_max_size){
        textPos = start_pos;
        m_res_size = size;
        return;
    }
    for (int i = 0; i < (int)(text.length()); i++){
        QChar c = text[i];
        if (c.isSpace()){
            unsigned size = res.length();
            if (size > m_max_size){
                textPos = start_pos + i;
                m_res_size = size;
                return;
            }
        }
        // In Qt, unless you force the paragraph direction with (Left/Right)
        // Ctrl-Shift (also known as Key_Direction_L and Key_Direction_R),
        // the P tag won't have a DIR attribute at all. In such cases, unlike
        // HTML, Qt will render the paragraph LTR or RTL according to the
        // first strong character (as Unicode TR#9 defines). Thus, if the
        // direction isn't known yet, we check each character till we find
        // a strong one.
        if ((m_lastParagraphPos != 0) && (m_paragraphDir == DirUnknown))
        {
            switch(c.direction())
            {
            case QChar::DirL:
                res.insert(m_lastParagraphPos, "\\ltrpar");
                m_paragraphDir = DirLTR;
                break;
            case QChar::DirR:
                res.insert(m_lastParagraphPos, "\\rtlpar");
                m_paragraphDir = DirRTL;
                break;
            default: // to avoid warnings
                break;
            }
        }

        unsigned short u = c.unicode();
        if (c == '\r' || c == '\n')
            continue;
        if ((c == '{') || (c == '}') || (c == '\\')){
            char b[5];
            snprintf(b, sizeof(b), "\\\'%02x", u & 0xFF);
            res += b;
            m_bSpace = false;
            continue;
        }
        if (u < 0x80){
            if (m_bSpace)
                res += ' ';
            res += (char)u;
            m_bSpace = false;
            continue;
        }
        QString s;
        s += c;
        if (m_codec){
            QByteArray plain = m_codec->fromUnicode(s);
            if ((plain.length() == 1) && (m_codec->toUnicode(plain) == s)){
                char b[5];
                snprintf(b, sizeof(b), "\\\'%02x", plain[0] & 0xFF);
                res += b;
                m_bSpace = false;
                continue;
            }
        }
        res += "\\u";
        res += QByteArray::number(s[0].unicode());
        res += '?';
        m_bSpace = false;
    }
}

static const char *def_smiles[] =
    {
        ":-)",
        ":-0",
        ":-|",
        ":-/",
        ":-(",
        ":-{}",
        ":*)",
        ":'-(",
        ";-)",
        ":-@",
        ":-\")",
        ":-X",
        ":-P",
        "8-)",
        "O:-)",
        ":-D",
        "*ANNOYED*",
        "*DISGUSTED*",
        "*DROOLING*",
        "*GIGGLING*",
        "*JOKINGLY*",
        "*SHOCKED*",
        "*WHINING*",
        "*SURPRISED*",
        "*SURPRISED*",
        "*IN LOVE*"
    };

void RTFGenParser::tag_start(const QString &tagName, const list<QString> &attrs)
{
	if (m_res_size)
		return;
	CharStyle parentStyle, style;
	{
		Tag* pParentTag = m_tags.getTopTagWithCharStyle();
		if (pParentTag != NULL)
		{
			parentStyle = *(pParentTag->pCharStyle);
		}
	}
	style = parentStyle;
	if ((tagName == "b") || (tagName == "i") || (tagName == "u") ||
			(tagName == "font") || (tagName == "p") || (tagName == "span")){
		QString tag = tagName;
		QString option;
		for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
			QString key = *it;
			++it;
			QString value = *it;
			option += ' ';
			option += key;
			if (!value.isEmpty()){
				option += "=\"";
				option += value;
				option += "\"";
			}
		}
		tags.push(tag);
		options.push(option);
	}

	if (tagName == "b"){
		style.bold = true;
	}
	else if (tagName == "i"){
		style.italic = true;
	}
	else if (tagName == "u"){
		style.underline = true;
	}
	else if (tagName == "font"){
		for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); it++){
			QString name = (*it);
			++it;
			QString value = (*it);
			if (name == "color")
			{
				style.colorIdx = getColorIdx(value);
			}
			else if (name == "face")
			{
				style.faceIdx = getFontFaceIdx(value);
			}
			else if (name == "size")
			{
				int logicalSize = value.toInt();
				if (value[0] == '+' || value[0] == '-')
					logicalSize += 3;
				if (logicalSize < 1)
					logicalSize = 1;
				else if (logicalSize > 7)
					logicalSize = 7;
				style.sizePt = htmlFontSizeToPt(logicalSize);
			}
		}
	}
	else if (tagName == "p"){
		m_paragraphDir = DirUnknown;
		m_lastParagraphPos = res.length();
		m_bSpace = true;
		for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = it->toLower();
			++it;
			QString value = (*it);
			if (name == "dir")
			{
                QString dir = value.toLower();
				if (dir == "ltr")
				{
					res += "\\ltrpar";
					m_paragraphDir = DirLTR;
				}
				if (dir == "rtl")
				{
					res += "\\rtlpar";
					m_paragraphDir = DirRTL;
				}
			}
		}

	}
	else if (tagName == "br"){
		res += "\\line";
		m_bSpace = true;
	}
	else if (tagName == "img"){
		QString src;
		QString alt;
		for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
			QString name = (*it);
			++it;
			QString value = (*it);
			if (name == "src"){
				src = value;
				break;
			}
			if (name == "alt"){
				alt = value;
				break;
			}
		}
        if (src.startsWith("sim:icons/")){
            QStringList smiles = getIcons()->getSmile(src.mid(10));
			for (QStringList::iterator its = smiles.begin(); its != smiles.end(); ++its){
				QString s = *its;
				for (unsigned nSmile = 0; nSmile < 26; nSmile++){
					if (s != def_smiles[nSmile])
						continue;
					res += "<##icqimage00";
					char buf[4];
					sprintf(buf, "%02X", nSmile);
					res += buf;
					res += '>';
					return;
				}
			}
			if (!smiles.empty()){
				text(smiles.front());
				return;
			}
		}
		text(alt);
		return;
	}

	// Process attributes which all tags share.

	for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = it->toLower();
		++it;
		QString value = (*it);

		// Any tag might have a STYLE.
		if (name == "style"){
			// A really crude CSS parser goes here:
			QRegExp cssReNum("[0-9]+");
			list<QString> cssProp = parseStyle(value);
			for (list<QString>::iterator it = cssProp.begin(); it != cssProp.end(); ++it)
			{
				QString cssPropName = *it;
				++it;
				if (it == cssProp.end())
					break;
				QString cssPropValue = *it;
				if (cssPropName == "font-family")
				{
					style.faceIdx = getFontFaceIdx(cssPropValue);
				}
				else if (cssPropName == "font-size")
				{
                    cssPropValue = cssPropValue.toLower();
					int length;
					if(cssReNum.indexIn(cssPropValue) != -1)
					{
						length = cssReNum.matchedLength();
						float number = cssPropValue.left(length).toFloat();
						QString type = cssPropValue.mid(length);
						if (type == "pt")
						{
							style.sizePt = static_cast<int>(number);
						}
						else if (type == "px")
						{
							// for now, handle like 'pt', though it's wrong
							style.sizePt = static_cast<int>(number);
						}
						else if (type == "%")
						{
							style.sizePt = static_cast<int>(parentStyle.sizePt * (number/100));
						}
						// We don't handle 'cm', 'em' etc.
					}
					else if (cssPropValue == "smaller")
					{
						// FONT SIZE=3 is 'normal', 2 is 'smaller'
						style.sizePt = htmlFontSizeToPt(2, parentStyle.sizePt);
					}
					else if (cssPropValue == "larger")
					{
						// FONT SIZE=3 is 'normal', 4 is 'larger'
						style.sizePt = htmlFontSizeToPt(4, parentStyle.sizePt);
					}

					// We don't handle 'small', 'medium' etc. It goes too far
					// beyond our basic implementation.
					// Also, empty 'type' would be invalid CSS, thus ignored.
				}
				else if (cssPropName == "font-style")
				{
                    style.italic = (cssPropValue.toLower() == "italic");
				}
				else if (cssPropName == "font-weight")
				{
					style.bold = (cssPropValue.toInt() >= 600);
				}
				else if (cssPropName == "text-decoration")
				{
                    style.underline = (cssPropValue.toLower() == "underline");
				}
				else if (cssPropName == "color")
				{
					style.colorIdx = getColorIdx(cssPropValue);
				}
				else if (cssPropName == "background-color")
				{
					style.bgColorIdx = getColorIdx(cssPropValue);
				}
			}
		}
	}

	Tag& tag = *(m_tags.pushNew());
	tag.name = tagName;
	// Check if anything changed in the style.
	// Only then the tag deserves getting a charStyle.
	if (parentStyle != style)
	{
		QString rtf = style.getDiffRTF(parentStyle);
		if (!rtf.isEmpty())
		{
            res += rtf.toUtf8();
			m_bSpace = true;
		}
		tag.setCharStyle(style);
	}
}

void RTFGenParser::tag_end(const QString &tagName)
{
    if (m_res_size)
        return;
    if ((tagName == "b") || (tagName == "i") || (tagName == "u") ||
            (tagName == "font") || (tagName == "p") || (tagName == "span")){
        while (!tags.empty()){
            QString tag = tags.top();
            tags.pop();
            options.pop();
            if (tag == tagName)
                break;
        }
    }

    // Roll back until we find our tag.
    bool found = false;
    for(Tag* pTag = m_tags.peek(); pTag != NULL && !found; pTag = m_tags.peek())
    {
        if (pTag->name == tagName)
        {
            found = true;
        }

        if (pTag->hasCharStyle())
        {
            CharStyle style = *(pTag->pCharStyle);

            // We must pop here, so that getTopTagWithCharStyle will find a parent tag.
            m_tags.pop();
            pTag = NULL; // to avoid confusion

            Tag* pParentTag = m_tags.getTopTagWithCharStyle();
            if (pParentTag != NULL)
            {
                if (pParentTag->hasCharStyle())
                {
                    CharStyle* pParentStyle = pParentTag->pCharStyle;

                    // Roll back the character style. This is regardless of whether
                    // we found the closed tag; we just collapse all styles on our way.
                    QString rtf = pParentStyle->getDiffRTF(style);
                    if (!rtf.isEmpty())
                    {
                        res += rtf.toUtf8();
                        m_bSpace = true;
                    }
                }
            }
        }
        else // if this tag has no char style attached
        {
            m_tags.pop(); // just pop the tag out
            pTag = NULL; // to avoid confusion
        }

        if (found)
        {
            if (tagName.toLower() == "p")
            {
                res += "\\par";
                m_bSpace = true;
            }
        }
    }
}

QByteArray ICQClient::createRTF(QString &text, QString &part, unsigned long foreColor, Contact *contact, unsigned max_size)
{
    RTFGenParser p(this, foreColor, contact, max_size);
    QByteArray res = p.parse(text);
    if (p.m_res_size == 0){
        part = text;
        text.clear();
        return res;
    }
    QString endTags;
    QString startTags;
    while (!p.tags.empty()){
        QString tag = p.tags.top();
        QString option = p.options.top();
        p.tags.pop();
        p.options.pop();
        endTags   += "</" + tag + '>';
        startTags = '<' + tag + option + '>' + startTags;
    }
    part = text.left(p.textPos) + endTags;
    text = startTags + text.mid(p.textPos);
    return res;
}

class ImageParser : public HTMLParser
{
public:
    ImageParser(bool bIcq);
    QString parse(const QString &text);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
    void startBody();
    void endBody();
    QByteArray res;
    bool	 m_bBody;
    bool	 m_bIcq;
};

ImageParser::ImageParser(bool bIcq)
{
    m_bIcq = bIcq;
}

QString ImageParser::parse(const QString &text)
{
    res = 0; //QString::null;
    startBody();
    HTMLParser::parse(text);
    endBody();
    return res;
}

void ImageParser::startBody()
{
    m_bBody = true;
    res = 0;//QString::null;
}

void ImageParser::endBody()
{
    if (m_bBody)
        m_bBody = false;
}

void ImageParser::text(const QString &text)
{
    if (!m_bBody)
        return;
    res += quoteString(text).toUtf8();
}

void ImageParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    QString oTag = tag;

    if (tag == "html")
	{
        res = 0; //QString::null;
        m_bBody = false;
        return;
    }
    if (tag == "body"){
        startBody();
        // We still want BODY's styles
        oTag = "span";
    }
    if (!m_bBody)
        return;
    if (tag == "img"){
        QString src;
        QString alt;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name == "src"){
                src = value;
                break;
            }
            if (name == "alt"){
                alt = value;
                break;
            }
        }
        if (src.left(10) != "sim:icons/"){
            text(alt);
            return;
        }
        QStringList smiles = getIcons()->getSmile(src.mid(5));
        if (smiles.empty()){
            text(alt);
            return;
        }
        if (m_bIcq){
            QStringList::ConstIterator its;
            for (its = smiles.constBegin(); its != smiles.constEnd(); ++its){
                for (unsigned nSmile = 0; nSmile < 26; nSmile++){
                    if ((*its) != def_smiles[nSmile])
                        continue;
                    res += "<img src=\"sim:icons/smile";
                    char b[4];
                    sprintf(b, "%X", nSmile);
                    res += b;
                    res += "\">";
                    return;
                }
            }
        }
        text(smiles.front());
        return;
    }
    res += '<';
    res += oTag.toUtf8();
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        res += ' ';
        res += name.toUpper().toUtf8();
        if (!value.isEmpty()){
            res += "=\"";
            res += quoteString(value).toUtf8();
            res += "\"";
        }
    }
    res += '>';
}

void ImageParser::tag_end(const QString &tag)
{
    QString oTag = tag;
    if (!m_bBody)
        return;
    if (tag == "body"){
        endBody();
        oTag = "span";
    }
    res += "</";
    res += oTag.toUtf8();
    res += '>';
}

QString ICQClient::removeImages(const QString &text, bool bIcq)
{
    ImageParser p(bIcq);
    return p.parse(text);
}

class BgParser : public HTMLParser
{
public:
    BgParser();
    QString parse(const QString &text);
    unsigned bgColor;
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
    QString res;
    bool	 m_bBody;
};

BgParser::BgParser()
{
    bgColor = 0xFFFFFF;
    m_bBody = true;
}

QString BgParser::parse(const QString &text)
{
    res = QString::null;
    HTMLParser::parse(text);
    return res;
}

void BgParser::text(const QString &text)
{
    if (!m_bBody)
        return;
    res += quoteString(text);
}

void BgParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    if (tag == "body"){
        m_bBody = true;
        res = QString::null;
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name.toLower() == "bgcolor"){
                QColor c(value);
                bgColor = c.rgb();
            }
        }
        return;
    }
    if (!m_bBody)
        return;
    res += '<';
    res += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        res += ' ';
        res += name;
        if (!value.isEmpty()){
            res += "=\"";
            res += quoteString(value);
            res += "\"";
        }
    }
    res += '>';
}

void BgParser::tag_end(const QString &tag)
{
    if (tag == "body"){
        m_bBody = false;
        return;
    }
    if (!m_bBody)
        return;
    res += "</";
    res += tag;
    res += '>';
}

unsigned ICQClient::clearTags(QString &text)
{
    BgParser p;
    text = p.parse(text);
    return p.bgColor;
}



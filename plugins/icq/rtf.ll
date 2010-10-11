%{
    /***************************************************************************
                              rtf.ll  -  description
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

#include <stdio.h>

#include <vector>
#include <stack>
#include <stdarg.h>

#include <QTextCodec>

#include "log.h"

#include "icqclient.h"

#define UP				1	
#define DOWN			2
#define CMD				3
#define TXT				4
#define HEX				5
#define IMG				6
#define UNICODE_CHAR	7
#define SKIP			8
#define SLASH			9
#define S_TXT			10

#define YY_NEVER_INTERACTIVE	1
#define YY_ALWAYS_INTERACTIVE	0
#define YY_MAIN			0	

%}

%option nounput
%option nostack
%option prefix="rtf"

%%

"{"				{ return UP; }
"}"				{ return DOWN; }
"\\"[\\\{\}]			{ return SLASH; }
"\\u"[0-9]{3,7}[ ]?"?"		{ return UNICODE_CHAR; }
"\\"[A-Za-z]+[0-9]*[ ]? 	{ return CMD; }
"\\'"[0-9A-Fa-f][0-9A-Fa-f]	{ return HEX; }
"<##"[^>]+">"			{ return IMG; }
[^\\{}<]+			{ return TXT; }
.				{ return TXT; }
%%

using namespace std;
using namespace SIM;

struct FontDef
{
    int		charset;
    QString	taggedName;
    QString	nonTaggedName;
};

class RTF2HTML;

enum TagEnum
{
    TAG_ALL = 0,
    TAG_FONT_SIZE,
    TAG_FONT_COLOR,
    TAG_FONT_FAMILY,
    TAG_BG_COLOR,
    TAG_BOLD,
    TAG_ITALIC,
    TAG_UNDERLINE
};

class ParStyle
{
public:
    ParStyle() { dir = DirLTR; }	
    void clearFormatting();

public:
    enum {DirLTR, DirRTL} dir;
};

void ParStyle::clearFormatting()
{
   // For now, do nothing.
   // dir is not a formatting item.
}

class Level
{
public:
    Level(RTF2HTML *_p);
    Level(const Level&);
    void setText(const char* str);
    void setFontTbl() { m_bFontTbl = true; }
    void setColors() { m_bColors = true; resetColors(); }
    void setRed(unsigned char val) { setColor(val, &m_nRed); }
    void setGreen(unsigned char val) { setColor(val, &m_nGreen); }
    void setBlue(unsigned char val) { setColor(val, &m_nBlue); }
    void setFont(unsigned nFont);
    void setEncoding(unsigned nFont);
    void setFontName();
    void setFontColor(unsigned short color);
    void setFontBgColor(unsigned short color);
    void setFontSizeHalfPoints(unsigned short sizeInHalfPoints);
    void setFontSize(unsigned short sizeInPoints);
    void setBold(bool);
    void setItalic(bool);
    void setUnderline(bool);
    void startParagraph();
    bool isParagraphOpen() const;
    void clearParagraphFormatting();
    void setParagraphDirLTR();
    void setParagraphDirRTL();
    void addLineBreak();
    void setAnsiCodePage(unsigned short cp);
    void flush();
    void reset();
    void resetTag(TagEnum tag);
protected:
    QString text;
    void Init();
    RTF2HTML *p;
    void resetColors() { m_nRed = m_nGreen = m_nBlue = 0; m_bColorInit = false; }
    void setColor(unsigned char val, unsigned char *p)
    { *p = val; m_bColorInit=true; }

    // Marks the position in m_tags where this level begun.
    unsigned m_nTagsStartPos;
    
    // True when parsing the fonts table
    bool m_bFontTbl;
    // True when parsing the colors table.
    bool m_bColors;
    // True when inside a 'fname' block.
    bool m_bFontName;
    // False until we get the tagged font name.
    bool m_bTaggedFontNameOk;
    
    unsigned char m_nRed;
    unsigned char m_nGreen;
    unsigned char m_nBlue;
    bool m_bColorInit;
    unsigned m_nFont; // 1-based
    unsigned m_nEncoding;
    unsigned m_nFontColor; // 1-based
    unsigned m_nFontSize;
    unsigned m_nFontBgColor; // 1-based
    bool m_bBold;
    bool m_bItalic;
    bool m_bUnderline;
};

class OutTag
{
public:
    OutTag(TagEnum _tag, unsigned _param) : tag(_tag), param(_param) {}
    TagEnum tag;
    unsigned param;
};

class RTF2HTML
{
    friend class Level;

public:
    RTF2HTML();
    QString Parse(const char *rtf, const char *encoding);

// Paragraph-specific functions:

    // Appends a string with formatting into the paragraph buffer.
#if defined __GNUC__
    void PrintUnquoted(const char *str, ...)
       __attribute__ ((__format__ (printf, 2, 3)));
#else
    void PrintUnquoted(const char *str, ...);
#endif      
    // Quotes and appends a string to the paragraph buffer.
    void PrintQuoted(const QString &str);
    // Writes down the tags from oTags into the paragraph buffer.
    void FlushOutTags();
    // Retrieves the top not-yet-written tag.
    OutTag* getTopOutTag(TagEnum tagType);
    // Writes down the paragraph buffer and resets the paragraph state.
    void FlushParagraph();
    //sets the codepage found in the rtf stream
    void setAnsiCodePage(unsigned short cp);

// Document-wide functions:

    void PutTag(TagEnum n)
    {
       tags.push(n);
    }

protected:

// Paragraph members

    // The paragraph's HTML buffer.
    QString sParagraph;
    // True if the paragraph was opened explicitly.
    bool bExplicitParagraph;
    // Whether we have an empty paragraph which we haven't flushed yet.
    // We do not haste with flushing it, since it might be the last
    // (empty) paragraph in the RTF message, which we want to discard.
    // This last paragraph is not appended by the user, but by the
    // client itself, to keep the official ICQ client's sanity,
    // and there's no reason our users should see it.
    bool bPendingEmptyParagraph;
    // Defines the paragraph's formatting.
    ParStyle parStyle;
    // Tags which weren't yet printed out.
    vector<OutTag> oTags;

// Document members

    // The document HTML buffer.
    QString s;
    // Fonts table.
    vector<FontDef> fonts;
    // Colors table.
    vector<QColor> colors;
    // Stack of tags (across all levels, not just current level)
    stack<TagEnum> tags;

// RTF parser internals

    const char *rtf_ptr;
    const char *encoding;
    Level cur_level;
    stack<Level> levels;
};

RTF2HTML::RTF2HTML()
    : cur_level(this)
{
    rtf_ptr = NULL;
    bExplicitParagraph = false;
    bPendingEmptyParagraph = false;
}

OutTag* RTF2HTML::getTopOutTag(TagEnum tagType)
{
    vector<OutTag>::iterator it, it_end;
    for(it = oTags.begin(), it_end = oTags.end(); it != it_end; ++it)
       if (it->tag == tagType)
        return &(*it);
    return NULL;
}

void RTF2HTML::FlushOutTags()
{
    vector<OutTag>::iterator iter;
    for (iter = oTags.begin(); iter != oTags.end(); iter++)
    {
        OutTag &t = *iter;
        switch (t.tag){
        case TAG_FONT_COLOR:
            {
               // RTF colors are 1-based; colors[] is a 0-based array.
               if (t.param > colors.size() || t.param == 0)
                   break;
               QColor &c = colors[t.param-1];
               PrintUnquoted("<span style=\"color:#%02X%02X%02X\">", c.red(), c.green(), c.blue());
            }
            break;
        case TAG_FONT_SIZE:
            PrintUnquoted("<span style=\"font-size:%upt\">", t.param);
            break;
        case TAG_FONT_FAMILY:
            {
               if (t.param > fonts.size() || t.param == 0)
                   break;
               FontDef &f = fonts[t.param-1];
               QString name = (!f.nonTaggedName.isEmpty()) ? f.nonTaggedName : f.taggedName;
               PrintUnquoted("<span style=\"font-family:%s\">", qPrintable(name));
            }
            break;
        case TAG_BG_COLOR:
            {
                if (t.param > colors.size() || t.param == 0)            
                    break;
                QColor &c = colors[t.param-1];
                PrintUnquoted("<span style=\"background-color:#%02X%02X%02X;\">", c.red(), c.green(), c.blue());
                break;
            }
        case TAG_BOLD:
            PrintUnquoted("<b>");
            break;
        case TAG_ITALIC:
            PrintUnquoted("<i>");
            break;
        case TAG_UNDERLINE:
            PrintUnquoted("<u>");
            break;
        default:
            break;
        }
    }
    oTags.clear();
}

// This function will close the already-opened tag 'tag'. It will take
// care of closing the tags which 'tag' contains first (ie. it will unroll
// the stack till the point where 'tag' is).
void Level::resetTag(TagEnum tag)
{
    // A stack which'll keep tags we had to close in order to reach 'tag'.
    // After we close 'tag', we will reopen them.
    stack<TagEnum> s;
    
    while (p->tags.size() > m_nTagsStartPos){ // Don't go further than the point where this level starts.
 
        TagEnum nTag = p->tags.top();

        /* A tag will be located in oTags if it still wasn't printed out.
           A tag will get printed out only if necessary (e.g. <I></I> will
           be optimized away).
           Thus, for each tag we remove from the actual tag stack, we also
           try to remove a yet-to-be-printed tag, and only if there are no
           yet-to-be-printed tags left, we start closing the tags we pop.
           The tags have one space - needed for umlaute (�) and .utf8()
        */
        if (p->oTags.empty()){
            switch (nTag){
            case TAG_FONT_COLOR:
            case TAG_FONT_SIZE:
            case TAG_BG_COLOR:
            case TAG_FONT_FAMILY:
                p->PrintUnquoted("</span>");
                break;
            case TAG_BOLD:
                p->PrintUnquoted("</b>");
                break;
            case TAG_ITALIC:
                p->PrintUnquoted("</i>");
                break;
            case TAG_UNDERLINE:
                p->PrintUnquoted("</u>");
                break;
            default:
                break;
            }
        }else{
            p->oTags.pop_back();
        }

        p->tags.pop();
        if (nTag == tag) break; // if we reached the tag we were looking to close.
        s.push(nTag); // remember to reopen this tag
    }
    
    if (tag == TAG_ALL) return;

    while (!s.empty()){
        TagEnum nTag = s.top();
        switch (nTag){
        case TAG_FONT_COLOR:{
                unsigned nFontColor = m_nFontColor;
                m_nFontColor = 0;
                setFontColor(nFontColor);
                break;
            }
        case TAG_FONT_SIZE:{
                unsigned nFontSize = m_nFontSize;
                m_nFontSize = 0;
                setFontSize(nFontSize);
                break;
            }
        case TAG_BG_COLOR:{
                unsigned nFontBgColor = m_nFontBgColor;
                m_nFontBgColor = 0;
                setFontBgColor(nFontBgColor);
                break;
            }
        case TAG_FONT_FAMILY:{
                unsigned nFont = m_nFont;
                m_nFont = 0;
                setFont(nFont);
                break;
            }
        case TAG_BOLD:{
                bool nBold = m_bBold;
                m_bBold = false;
                setBold(nBold);
                break;
            }
        case TAG_ITALIC:{
                bool nItalic = m_bItalic;
                m_bItalic = false;
                setItalic(nItalic);
                break;
            }
        case TAG_UNDERLINE:{
                bool nUnderline = m_bUnderline;
                m_bUnderline = false;
                setUnderline(nUnderline);
                break;
            }
        default:
            break;
        }
        s.pop();
    }
}

Level::Level(RTF2HTML *_p) :
        p(_p),
        m_bFontTbl(false),
        m_bColors(false),
        m_bFontName(false),
        m_bTaggedFontNameOk(false),
        m_nFont(0),
        m_nEncoding(0)
{
    m_nTagsStartPos = p->tags.size();
    Init();
}

Level::Level(const Level &l) :
        p(l.p),
        m_bFontTbl(l.m_bFontTbl),
        m_bColors(l.m_bColors),
        m_bFontName(false),
        m_bTaggedFontNameOk(l.m_bTaggedFontNameOk),
        m_nFont(l.m_nFont),
        m_nEncoding(l.m_nEncoding)
{
    m_nTagsStartPos = p->tags.size();
    Init();
}

void Level::Init()
{
    m_nFontColor = 0;
    m_nFontBgColor = 0;
    m_nFontSize = 0;
    m_bFontName = false;
    m_bBold = false;
    m_bItalic = false;
    m_bUnderline = false;
}

void RTF2HTML::PrintUnquoted(const char *str, ...)
{
    char buff[1024];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buff, sizeof(buff), str, ap);
    va_end(ap);
    sParagraph += buff;
}

void RTF2HTML::PrintQuoted(const QString &str)
{
    sParagraph += (str);
}

void RTF2HTML::FlushParagraph()
{
    if (bPendingEmptyParagraph)
    {
       s += "<p><br></p>";
       bPendingEmptyParagraph = false;
    }

    if (!sParagraph.isEmpty())
    {
       s += "<p dir=\"";
       // Note: Lower-case 'ltr' and 'rtl' are important for Qt.
       s += (parStyle.dir == ParStyle::DirRTL ? "rtl" : "ltr");
       s += "\">";
       s += sParagraph;
       s += "</p>";
    }
    else
    {
       if (bExplicitParagraph)
          // Delay the empty paragraph for later appending.
          // If this empty paragraph happens to be the last one
          // in the message, we'll end up discarding it -- which
          // is a good thing (since it's just a compatibility hack
          // for the official ICQ client).
          bPendingEmptyParagraph = true;
    }

    // Clear up the paragraph members
    sParagraph = "";
}

void RTF2HTML::setAnsiCodePage(unsigned short cp)
{
    for (const ENCODING *c = getContacts()->getEncodings(); c->language; c++){
        if (!c->bMain)
            continue;
        if ((unsigned)c->cp_code == cp){
            encoding = c->codec;
            return;
        }
    }
}

void Level::setFont(unsigned nFont)
{
    if (nFont <= 0)
        return;

    if (m_bFontTbl){
        if (nFont > p->fonts.size() +1){
            log(L_WARN, "Invalid font index (%u) while parsing font table.", nFont);
            return;
        }
        if (nFont > p->fonts.size()){
            FontDef f;
            f.charset = 0;
            p->fonts.push_back(f);
        }
        m_nFont = nFont;
    }
    else
    {
        if (nFont > p->fonts.size())
        {
           log(L_WARN, "Invalid font index (%u)",nFont);
           return;
        }
        if (m_nFont == nFont)
           return;
        m_nFont = nFont;
        if (m_nFont) resetTag(TAG_FONT_FAMILY);
        m_nEncoding = p->fonts[nFont-1].charset;
        p->oTags.push_back(OutTag(TAG_FONT_FAMILY, nFont));
        p->PutTag(TAG_FONT_FAMILY);
    }
}

void Level::setFontName()
{
    // This function is only valid during font table parsing.
    if (m_bFontTbl){
        if ((m_nFont > 0) && (m_nFont <= p->fonts.size()))
            // Be prepared to accept a font name.
            m_bFontName = true;
    }
}

void Level::setEncoding(unsigned nEncoding)
{
    if (m_bFontTbl){
        if ((m_nFont > 0) && (m_nFont <= p->fonts.size()))
            p->fonts[m_nFont-1].charset = nEncoding;
        return;
    }
    m_nEncoding = nEncoding;
}

void Level::setBold(bool bBold)
{
    if (m_bBold == bBold) return;
    if (m_bBold) resetTag(TAG_BOLD);
    m_bBold = bBold;
    if (!m_bBold) return;
    p->oTags.push_back(OutTag(TAG_BOLD, 0));
    p->PutTag(TAG_BOLD);
}

void Level::setItalic(bool bItalic)
{
    if (m_bItalic == bItalic) return;
    if (m_bItalic) resetTag(TAG_ITALIC);
    m_bItalic = bItalic;
    if (!m_bItalic) return;
    p->oTags.push_back(OutTag(TAG_ITALIC, 0));
    p->PutTag(TAG_ITALIC);
}

void Level::setUnderline(bool bUnderline)
{
    if (m_bUnderline == bUnderline) return;
    if (m_bUnderline) resetTag(TAG_UNDERLINE);
    m_bUnderline = bUnderline;
    if (!m_bUnderline) return;
    p->oTags.push_back(OutTag(TAG_UNDERLINE, 0));
    p->PutTag(TAG_UNDERLINE);
}

void Level::setFontColor(unsigned short nColor)
{
    if (m_nFontColor == nColor) return;
    if (m_nFontColor) resetTag(TAG_FONT_COLOR);
    if (nColor > p->colors.size()) return;
    m_nFontColor = nColor;
    p->oTags.push_back(OutTag(TAG_FONT_COLOR, m_nFontColor));
    p->PutTag(TAG_FONT_COLOR);
}

void Level::setFontBgColor(unsigned short nColor)
{
    if (m_nFontBgColor == nColor) return;
    if (m_nFontBgColor != 0) resetTag(TAG_BG_COLOR);
    if (nColor > p->colors.size()) return;
    m_nFontBgColor = nColor;
    p->oTags.push_back(OutTag(TAG_BG_COLOR, m_nFontBgColor));
    p->PutTag(TAG_BG_COLOR);
}

void Level::setFontSizeHalfPoints(unsigned short nSize)
{
    setFontSize(nSize / 2);
}

void Level::setFontSize(unsigned short nSize)
{
    if (m_nFontSize == nSize)
        return;
    if (m_nFontSize)
        resetTag(TAG_FONT_SIZE);
    p->oTags.push_back(OutTag(TAG_FONT_SIZE, nSize));
    p->PutTag(TAG_FONT_SIZE);
    m_nFontSize = nSize;
}

void Level::startParagraph()
{
    // Whatever tags we have open now, close them.
    // We cannot carry let character formatting tags wrap paragraphs,
    // since a formatting tag can close at any time and we cannot
    // close the paragraph any time we want.
    resetTag(TAG_ALL);

    // Flush the current paragraph HTML to the document HTML.
    p->FlushParagraph();

    // Mark this new paragraph as an explicit one (from \par etc.).
    p->bExplicitParagraph = true;
    
    // Restore character formatting
    p->oTags.push_back(OutTag(TAG_FONT_SIZE, m_nFontSize));
    p->PutTag(TAG_FONT_SIZE);
    p->oTags.push_back(OutTag(TAG_FONT_COLOR, m_nFontColor));
    p->PutTag(TAG_FONT_COLOR);
    p->oTags.push_back(OutTag(TAG_FONT_FAMILY, m_nFont));
    p->PutTag(TAG_FONT_FAMILY);
    if (m_nFontBgColor != 0)
    {
       p->oTags.push_back(OutTag(TAG_BG_COLOR, m_nFontBgColor));
       p->PutTag(TAG_BG_COLOR);
    }
    if (m_bBold)
    {
       p->oTags.push_back(OutTag(TAG_BOLD, 0));
       p->PutTag(TAG_BOLD);
    }
    if (m_bItalic)
    {
       p->PutTag(TAG_ITALIC);
       p->oTags.push_back(OutTag(TAG_ITALIC, 0));
    }    
    if (m_bUnderline)
    {
       p->oTags.push_back(OutTag(TAG_UNDERLINE, 0));
       p->PutTag(TAG_UNDERLINE);
    }
}

bool Level::isParagraphOpen() const
{
   return p->bExplicitParagraph;
}

void Level::clearParagraphFormatting()
{
    // implicitly start a paragraph
    if (!isParagraphOpen())
       startParagraph();
    // Since we don't implement any of the paragraph formatting tags (e.g. alignment),
    // we don't clean up anything here. Note that \pard does NOT clean character
    // formatting (such as font size, font weight, italics...).
    p->parStyle.clearFormatting();
}

void Level::setParagraphDirLTR()
{
    // implicitly start a paragraph
    if (!isParagraphOpen())
      startParagraph();
    p->parStyle.dir = ParStyle::DirLTR;
}

void Level::setParagraphDirRTL()
{
    // implicitly start a paragraph
    if (!isParagraphOpen()) 
      startParagraph();
    p->parStyle.dir = ParStyle::DirRTL;
}

void Level::addLineBreak()
{
    p->PrintUnquoted("<br/>");
}

void Level::setAnsiCodePage(unsigned short cp)
{
    p->setAnsiCodePage(cp);
}

void Level::reset()
{
    resetTag(TAG_ALL);
    if (m_bColors){
        if (m_bColorInit){
            QColor c(m_nRed, m_nGreen, m_nBlue);
            p->colors.push_back(c);
            resetColors();
        }
        return;
    }
}

void Level::setText(const char *str)
{
    if (m_bColors)
    {
        reset();
    }
    else if (m_bFontTbl)
    {
        if ((m_nFont <= 0) || (m_nFont > p->fonts.size()))
           return;
           
        FontDef& def = p->fonts[m_nFont-1];
           
        char *pp = (char *)strchr(str, ';');
        unsigned size;
        if (pp != NULL)
           size = (pp - str);
        else
           size = strlen(str);
        
        if (m_bFontName)
        {
            def.nonTaggedName += QString::fromLatin1(str, size);
            // We know we have the entire name
            if (pp != NULL)
               m_bFontName = false;
        }
        else if (!m_bTaggedFontNameOk)
        {
            def.taggedName += QString::fromLatin1(str, size);
            if (pp != NULL)
               m_bTaggedFontNameOk = true;
        }
    }
    else
    {
        for (; *str; str++)
            if ((unsigned char)(*str) >= ' ') break;
        if (!*str) return;
        p->FlushOutTags();
        text += str;
    }
}

void Level::flush()
{
    if (text.length() == 0)
        return;
    const char *encoding = NULL;
    if (m_nEncoding){
        for (const ENCODING *c = getContacts()->getEncodings(); c->language; c++){
			if (!c->bMain)
				continue;
            if ((unsigned)c->rtf_code == m_nEncoding){
                encoding = c->codec;
                break;
            }
        }
    }
    if (encoding == NULL)
		encoding = p->encoding;
	QTextCodec *codec = getContacts()->getCodecByName(encoding);
    p->PrintQuoted(codec->toUnicode(text.toLocal8Bit()));
    text = "";
}

const unsigned FONTTBL		= 0;
const unsigned COLORTBL		= 1;
const unsigned RED			= 2;
const unsigned GREEN		= 3;
const unsigned BLUE			= 4;
const unsigned CF			= 5;
const unsigned FS			= 6;
const unsigned HIGHLIGHT	= 7;
const unsigned PARD			= 8;
const unsigned PAR			= 9;
const unsigned I			= 10;
const unsigned B			= 11;
const unsigned UL			= 12;
const unsigned F			= 13;
const unsigned FCHARSET		= 14;
const unsigned FNAME		= 15;
const unsigned ULNONE		= 16;
const unsigned LTRPAR		= 17;
const unsigned RTLPAR		= 18;
const unsigned LINE             = 19;
const unsigned ANSICPG          = 20;

static char cmds[] =
    "fonttbl\x00"
    "colortbl\x00"
    "red\x00"
    "green\x00"
    "blue\x00"
    "cf\x00"
    "fs\x00"
    "highlight\x00"
    "pard\x00"
    "par\x00"
    "i\x00"
    "b\x00"
    "ul\x00"
    "f\x00"
    "fcharset\x00"
    "fname\x00"
    "ulnone\x00"
    "ltrpar\x00"
    "rtlpar\x00"
    "line\x00"
    "ansicpg\x00"
    "\x00";

int yywrap() { return 1; }

static char h2d(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return (c - 'A') + 10;
    if ((c >= 'a') && (c <= 'f'))
        return (c - 'a') + 10;
    return 0;
}

QString RTF2HTML::Parse(const char *rtf, const char *_encoding)
{
    encoding = _encoding;
    YY_BUFFER_STATE yy_current_buffer = yy_scan_string(rtf);
    yy_start = 1;	/* == BEGIN(INITIAL) - go to initial state since yy_start
                       is static and can have an old invalid value */
    rtf_ptr = rtf;
    int res;
    while ((res = yylex())) {
        switch (res){
        case UP:{
                cur_level.flush();
                levels.push(cur_level);
                break;
            }
        case DOWN:{
                if (!levels.empty()){
                   cur_level.flush();
                   cur_level.reset();
                   cur_level = levels.top();
                   levels.pop();
                }
                break;
            }
        case IMG:{
                cur_level.flush();
                const char ICQIMAGE[] = "icqimage";
                const char *p = yytext + 3;
                if ((strlen(p) > strlen(ICQIMAGE)) && !memcmp(p, ICQIMAGE, strlen(ICQIMAGE))){
                    unsigned n = 0;
                    for (p += strlen(ICQIMAGE); *p; p++){
                        if ((*p >= '0') && (*p <= '9')){
                            n = n << 4;
                            n += (*p - '0');
                            continue;
                        }
                        if ((*p >= 'A') && (*p <= 'F')){
                            n = n << 4;
                            n += (*p - 'A') + 10;
                            continue;
                        }
                        if ((*p >= 'a') && (*p <= 'f')){
                            n = n << 4;
                            n += (*p - 'a') + 10;
                            continue;
                        }
                        break;
                    }
					if (n < 26)
						PrintUnquoted("<img src=\"icon:smile%X\">", n);
                }else{
                    log(L_WARN, "Unknown image %s", yytext);
                }
                break;
            }
        case SKIP:
            break;
        case SLASH:
            cur_level.setText(yytext+1);
            break;
        case TXT:
            cur_level.setText(yytext);
            break;
        case UNICODE_CHAR:{
                 cur_level.flush();
                 sParagraph += QChar((unsigned short)(atol(yytext + 2)));
                 break;
            }
        case HEX:{
                char s[2];
                s[0] = (h2d(yytext[2]) << 4) + h2d(yytext[3]);
                s[1] = 0;
                cur_level.setText(s);
                break;
            }
        case CMD:
            {
                cur_level.flush();
                const char *cmd = yytext + 1;
                unsigned n_cmd = 0;
                unsigned cmd_size = 0;
                int cmd_value = -1;
                const char *p;
                for (p = cmd; *p; p++, cmd_size++)
                    if (((*p >= '0') && (*p <= '9')) || (*p == ' ')) break;
                if (*p && (*p != ' ')) cmd_value = atol(p);
                for (p = cmds; *p; p += strlen(p) + 1, n_cmd++){
                    if (strlen(p) >  cmd_size) continue;
                    if (!memcmp(p, cmd, cmd_size)) break;
                }
                cmd += strlen(p);
                switch (n_cmd){
                case FONTTBL:		// fonttbl
                    cur_level.setFontTbl();
                    break;
                case COLORTBL:
                    cur_level.setColors();
                    break;
                case RED:
                    cur_level.setRed(cmd_value);
                    break;
                case GREEN:
                    cur_level.setGreen(cmd_value);
                    break;
                case BLUE:
                    cur_level.setBlue(cmd_value);
                    break;
                case CF:
                    cur_level.setFontColor(cmd_value);
                    break;
                case FS:
                    cur_level.setFontSizeHalfPoints(cmd_value);
                    break;
                case HIGHLIGHT:
                    cur_level.setFontBgColor(cmd_value);
                    break;
                case PARD:
                    cur_level.clearParagraphFormatting();
                    break;
                case PAR:
                    cur_level.startParagraph();
                    break;
                case I:
                    cur_level.setItalic(cmd_value != 0);
                    break;
                case B:
                    cur_level.setBold(cmd_value != 0);
                    break;
                case UL:
                    cur_level.setUnderline(cmd_value != 0);
                    break;
                case ULNONE:
                    cur_level.setUnderline(false);
                    break;
                case F:
                    // RTF fonts are 0-based; our font index is 1-based.
                    cur_level.setFont(cmd_value+1);
                    break;
                case FCHARSET:
                    cur_level.setEncoding(cmd_value);
                    break;
                case FNAME:
                    cur_level.setFontName();
                    break;
                case LTRPAR:
                    cur_level.setParagraphDirLTR();
                    break;
                case RTLPAR:
                    cur_level.setParagraphDirRTL();
                    break;
                case LINE:
                    cur_level.addLineBreak();
                case ANSICPG:
                    cur_level.setAnsiCodePage(cmd_value);
                }
                break;
            }
        }
    };
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    FlushParagraph();
    return s;
}

bool ICQClient::parseRTF(const QByteArray &rtf, Contact *contact, QString &res)
{
	const char _RTF[] = "{\\rtf";
	// codec to use when no other information is in the rtf stream
	QTextCodec *codec = getContacts()->getCodec(contact);
	if (!qstrncmp(rtf.data(), _RTF, strlen(_RTF)))
	{
		RTF2HTML p;
		res = p.Parse(rtf.data(), codec->name().data());
		return true;
	}
	res = codec->toUnicode(rtf);
	return false;
}


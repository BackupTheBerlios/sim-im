%{
/***************************************************************************
                          html.ll  -  description
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

#include "html.h"
#include "buffer.h"
#include "log.h"

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN					0

#define TXT			1
#define TAG_START	2
#define TAG_CLOSE	3
#define ATTR		4
#define VALUE		5
#define TAG_END		6
#define SYMBOL		7
#define SKIP		8
#define SPACE		9
#define COMMENT		10

static QString current_tag;

%}

%option nostack
%option prefix="html"

%x s_tag
%x s_attr
%x s_value
%x s_string
%x s_string1
%x s_symbol
%x s_comment
%x s_script
%x s_style
%%

[\xC0-\xDF][\x80-\xBF]		{ return TXT; }
[\xE0-\xEF][\x00-\xFF]{2}	{ return TXT; }
[\xF0-\xF7][\x00-\xFF]{3}	{ return TXT; }
[\xF8-\xFB][\x00-\xFF]{4}	{ return TXT; }
[\xFC-\xFD][\x00-\xFF]{5}	{ return TXT; }
<s_script>"</"[Ss][Cc][Rr][Ii][Pp][Tt]">"	{ BEGIN(INITIAL); return TAG_END; }
<s_script>[^<]+				{ return TXT; }
<s_style>"</"[Ss][Tt][Yy][Ll][Ee]">"		{ BEGIN(INITIAL); return TAG_END; }
<s_style>[^<]+				{ return TXT; }
<INITIAL>[ ]+				{ return SPACE; }
"<!--"						{ BEGIN(s_comment); }
"</"[A-Za-z]+">"			{ return TAG_END; }
"<"[A-Za-z]+				{ BEGIN(s_tag); return TAG_START; }
<s_tag>">"					{ BEGIN(INITIAL); return TAG_CLOSE; }
<s_tag>[A-Za-z]+			{ BEGIN(s_attr); return ATTR; }
<s_tag>.					{ return SKIP; }
<s_attr>"="					{ BEGIN(s_value); return SKIP; }
<s_attr>">"					{ if (current_tag == QLatin1String("script")){
							    BEGIN(s_script);
							  }else if (current_tag == QLatin1String("style")){
							    BEGIN(s_style);
							  }else{
								BEGIN(INITIAL); 
							  }
							  return TAG_CLOSE; 
							}
<s_attr>[A-Za-z]			{ BEGIN(s_tag); unput(yytext[0]); return SKIP; }
<s_attr>.					{ return SKIP; }
<s_value>"\""				{ BEGIN(s_string); return SKIP; }
<s_value>"\'"				{ BEGIN(s_string1); return SKIP; }
<s_value>[^\ \'\">]+		{ BEGIN(s_tag); return VALUE; }
<s_value>.					{ return SKIP; }
<s_string>"\""				{ BEGIN(s_tag); return SKIP; }
<s_string>[^\"]+			{ return VALUE; }
<s_string1>"\'"				{ BEGIN(s_tag); return SKIP; }
<s_string1>[^\']+			{ return VALUE; }
<s_comment>"-->"			{ BEGIN(INITIAL); }
<s_comment>[^\-]+			{ return COMMENT; }
<s_comment>.				{ return COMMENT; }
"&gt";?						{ return SYMBOL; }
"&lt";?						{ return SYMBOL; }
"&amp";?					{ return SYMBOL; }
"&quot";?					{ return SYMBOL; }
"&nbsp";?					{ return SYMBOL; }
"&"[\#A-Za-z0-9]+";"		{ return SYMBOL; }
\r							{ return SKIP; }
\n							{ return SKIP; }
.							{ return TXT; }
%%

namespace SIM
{

using namespace std;

class HTMLParserPrivate
{
public:
	HTMLParserPrivate(HTMLParser *p);
	HTMLParser *p;
	void init();
	void flushText();
	QString text;
	QString tag;
	QString value;
	list<QString> attrs;
	unsigned start_pos;
	unsigned end_pos;
	unsigned text_pos;
};

HTMLParserPrivate::HTMLParserPrivate(HTMLParser *parser)
{
	p = parser;
}

void HTMLParserPrivate::init()
{
	text.clear();
	tag.clear();
	value.clear();
	attrs.clear();
}

void HTMLParserPrivate::flushText()
{
	p->start_pos = text_pos;
	p->end_pos   = end_pos;
	if (text.isEmpty())
		return;
	p->text(text);
	text.clear();
}

HTMLParser::HTMLParser()
{
	p = new HTMLParserPrivate(this);
}

HTMLParser::~HTMLParser()
{
	delete p;
}

void HTMLParser::parse(const QString &str)
{
	p->init();
	QByteArray cstr = str.toUtf8();
    YY_BUFFER_STATE yy_current_buffer = yy_scan_string(cstr);
	parse();
	yy_delete_buffer(yy_current_buffer);
}

void HTMLParser::parse(Buffer &buf)
{
	p->init();
	buf << (char)YY_END_OF_BUFFER_CHAR << (char)YY_END_OF_BUFFER_CHAR;
    YY_BUFFER_STATE yy_current_buffer = yy_scan_buffer((char*)buf.data(), buf.writePos());
	parse();
	yy_delete_buffer(yy_current_buffer);
}

struct Symbol
{
	const char		*name;
	unsigned short	value;
};

static Symbol symbols[] = 
{
	{ "lt", (unsigned short)'<' },
	{ "gt", (unsigned short)'>' },
	{ "amp", (unsigned short)'&' },
	{ "quot", (unsigned short)'\"' },
	{ "nbsp", (unsigned short)' ' },
	{ "deg",  176 },
	{ "reg",  174 },
	{ "copy", 169 },
	{ NULL, 0 }
};

void HTMLParser::parse()
{
	p->start_pos = 0;
	p->end_pos   = 0;
	p->text_pos	 = 0;
    yy_start = 1;	/* == BEGIN(INITIAL) - go to initial state since yy_start
                       is static and can have an old invalid value */
    int r;
    while ((r = yylex())) {;
		p->end_pos = p->start_pos + strlen(yytext);
		QString s;
		switch (r){
		case TXT:
			if (p->text.isEmpty())
				p->text_pos = p->start_pos;
			p->text += QString::fromUtf8(yytext);
			break;
		case SPACE:
			if (p->text.isEmpty())
				p->text_pos = p->start_pos;
			p->text += ' ';
			break;
		case SKIP:
			break;
		case TAG_START:
			p->flushText();
			p->text_pos = p->start_pos;
			s = yytext + 1;
			p->tag = s.toLower();
			p->value = "";
			current_tag = p->tag;
			break;
		case ATTR:
			if (!p->attrs.empty())
				p->attrs.push_back(p->value);
			p->value = "";
			p->attrs.push_back(yytext);
			break;
		case VALUE:
			p->value += QString::fromUtf8(yytext);
			break;
		case TAG_CLOSE:
			if (!p->attrs.empty())
				p->attrs.push_back(p->value);
			p->value = "";
			start_pos = p->text_pos;
			end_pos   = p->end_pos;
			tag_start(p->tag, p->attrs);
			p->attrs.clear();
			p->tag = "";
			break;
		case TAG_END:
			p->flushText();
			start_pos = p->start_pos;
			end_pos   = p->end_pos;
			s = yytext + 2;
			tag_end(s.left(s.length() - 1).toLower());
			break;
		case SYMBOL:
			if (p->text.isEmpty())
				p->text_pos = p->start_pos;
			s = yytext + 1;
			if (s[(int)(s.length() - 1)] == ';')
				s = s.left(s.length() - 1);
			s = s.toLower();
			Symbol *ss;
			for (ss = symbols; ss->name; ss++){
				if (s == ss->name){
					p->text += QChar(ss->value);
					break;
				}
			}
			if (ss->name == NULL){
				if (s[0] == '#'){
					bool bOk;				
					unsigned short code;
					if (s[1] == 'x')
						code = s.mid(2).toUShort(&bOk, 16); // hex
					else
						code = s.mid(1).toUShort(&bOk, 10); // decimal
					if (bOk)
						p->text += QChar(code);
				}else{
					log(L_WARN, "HTML: Unknown symbol &%s;", qPrintable(s));
				}
			}
			break;
		}
		p->start_pos = p->end_pos;
	};
	p->flushText();
}

list<QString> HTMLParser::parseStyle(const QString &str)
{
	list<QString> res;
	int i = 0;
	int end = str.length();
	while (i < end)
	{
		QString name;
		QString value;
		for (; i < end; i++)
		{
			if ((str[i] != ' ') && (str[i] != '\t'))
				break;
		}
		for (; i < end; i++)
		{
			if ((str[i] == ' ') || (str[i] == ':'))
				break;
			name += str[i];
		}
		for (; i < end; i++)
		{
			if (str[i] == ':')
				break;
		}
		if (i >= end)
			break;
		for (i++; i < end; i++)
		{
			if (str[i] != ' ')
				break;
		}
		if(i >= end)
			break;
		if (str[i] == '\'')
		{
			for (i++; i < end; i++){
				if (str[i] == '\'')
					break;
				value += str[i];
			}
		}
		else if (str[i] == '\"')
		{
			for (i++; i < end; i++)
			{
				if (str[i] == '\"')
					break;
				value += str[i];
			}
		}
		else
		{
			for (; i < end; i++){
				if (str[i] == ';')
					break;
				value += str[i];
			}
			int n;
			for (n = value.length() - 1; n >= 0; n--)
				if ((value[n] != ' ') && (value[n] != '\t'))
					break;
			value = value.left(n + 1);
		}
		for (; i < end; i++)
			if (str[i] == ';')
				break;
		i++;
		res.push_back(name.toLower());
		res.push_back(value);
	}
	return res;
}

QString HTMLParser::makeStyle(const list<QString> &opt)
{
	QString res;
	for (list<QString>::const_iterator it = opt.begin(); it != opt.end(); ++it){
		QString name = (*it);
		it++;
		if (it == opt.end())
			break;
		QString value = (*it);
		if (!res.isEmpty())
			res += ';';
		res += name;
		res += ':';
		int n;
		int end = value.length();
		for (n = 0; n < end; n++)
			if (value[n] == ' ')
				break;
		if (n < end)
			res += "\'";
		res += value;
		if (n < end)
			res += "\'";
	}
	return res;
}

}

int yywrap() { return 1; }

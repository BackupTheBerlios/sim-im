%{
/***************************************************************************
                          parse.ll  -  description
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

#include "unquot.h"

#include "navigate.h"
//Added by qt3to4:
#include <QByteArray>

#define TXT			1
#define URL			2
#define MAIL_URL	3
#define HTTP_URL	4
#define FTP_URL		5

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN         0

%}

%option nounput
%option nostack
%option prefix="parseurl"

%x x_tag
%x x_word
%x x_link
%%

(http|https|ftp)"://"[A-Za-z0-9/\,\.\?\@\&:\;\(\)\-_\+\'\%=~\#]+ { return URL; }
"file:///"[A-Za-z0-9/\,\.\?\@\&:\;\(\)\-_\+\'\%=\\~\#]+ { return URL; }
(mailto:)?[A-Za-z0-9\-_][A-Za-z0-9\-_\.]*\@([A-Za-z0-9\-]+\.)+[A-Za-z]+		{ return MAIL_URL; }
"www."[A-Za-z0-9/\,\.\?\&:\;\(\)\-_\+\%=~\#\']+			{ return HTTP_URL; }
"ftp."[A-Za-z0-9/\,\.:\;\-_\+~\']+				{ return FTP_URL; }
<INITIAL,x_word>"&quot;"					{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&amp;"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&lt;"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&gt;"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"\t"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>" "							{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>[\:\.\,\ \(\)]					{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"<a href=\""[^"]+"\">"			{ BEGIN(x_link); return TXT; }
<INITIAL,x_word>"<"						{ BEGIN(x_tag); return TXT; }
<x_tag>">"							{ BEGIN(INITIAL); return TXT; }
<x_tag>.							{ return TXT; }
<x_link>"</a>"						{ BEGIN(INITIAL); return TXT; }
<x_link>.							{ return TXT; }
<INITIAL,x_word>[\xC0-\xDF][\x80-\xBF]				{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xE0-\xEF][\x00-\xFF]{2}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xF0-\xF7][\x00-\xFF]{3}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xF8-\xFB][\x00-\xFF]{4}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xFC-\xFD][\x00-\xFF]{5}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>"\n"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>.						{ BEGIN(x_word); return TXT; }
%%

int yywrap() { return 1; }

QString NavigatePlugin::parseUrl(const QString &text)
{
    QByteArray str = text.toUtf8();
    YY_BUFFER_STATE yy_current_buffer = yy_scan_string(str);
    yy_start = 1;	/* == BEGIN(INITIAL) - go to initial state since yy_start
                       is static and can have an old invalid value */
    QString res;
    int r;
    while ((r = yylex())) {;
		if (r == TXT){
			res += QString::fromUtf8(yytext);
			continue;
		}
		QString url  = yytext;
		QString link = SIM::unquoteString(QString::fromUtf8(yytext));
        switch (r){
        case MAIL_URL:
            if (link.left(7) != "mailto:")
                link = QString("mailto:") + link;
            break;
        case HTTP_URL:
			link = QString("http://") + link;
			break;
        case FTP_URL:
			link = QString("ftp://") + link;
			break;
		}
		res += "<a href=\"";
		res += link;
		res += "\"><u>";
		res += url;
		res += "</u></a>";
    };
    yy_delete_buffer(yy_current_buffer);
    return res;
}





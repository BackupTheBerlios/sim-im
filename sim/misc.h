/***************************************************************************
                          misc.h  -  description
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

#ifndef _MISC_H
#define _MISC_H

#include "simapi.h"

#include "country.h"

#include <QColor>
#include <QDateTime>

class KAboutData;
class QComboBox;
class QIcon;
//class Data;

#ifdef Q_OS_WIN
	#ifndef snprintf
		#define snprintf _snprintf
	#endif
#endif


#ifdef USE_KDE4
	#include <klocale.h>
	EXPORT void resetPlural();
	EXPORT QString i18n(const char *text);
    inline QString i18n(const QString &text)
    { return i18n(qPrintable(text)); }
	EXPORT QString i18n(const char *text, const char *comment);
	EXPORT QString i18n(const char *singular, const char *plural, unsigned long n);
#else

	EXPORT void resetPlural();
	EXPORT QString i18n(const char *text);
    inline QString i18n(const QString &text)
    { return i18n(qPrintable(text)); }
	EXPORT QString i18n(const char *text, const char *comment);
	EXPORT QString i18n(const char *singular, const char *plural, unsigned long n);

    inline QString i18n(const char *text, const QString &comment)
    { return i18n(text, qPrintable(comment)); }

    inline QString i18n(const QString &text, const QString &comment)
    { return i18n(qPrintable(text), qPrintable(comment)); }

    inline QString tr2i18n(const char* message, const char* =0) 
    { return i18n(message); }

	#ifndef I18N_NOOP
		#define I18N_NOOP(A)  A
	#endif
#endif



namespace SIM {

EXPORT QString formatDateTime(QDateTime t);
inline QString formatDateTime(unsigned int t){ return formatDateTime(QDateTime::fromTime_t(t)); }
EXPORT QString formatDate(QDate t);
EXPORT QString formatAddr(unsigned long ip, unsigned port);

// _____________________________________________________________________________________
// User interface

#ifdef WIN32
	EXPORT void setWndProc(QWidget*);
	#define SET_WNDPROC(A)  SIM::setWndProc(this);
#else
    #ifndef Q_OS_MAC
		EXPORT void setWndClass(QWidget*, const char*);
		#define SET_WNDPROC(A)  SIM::setWndClass(this, A);
	#else
		#define SET_WNDPROC(A)
	#endif
#endif

EXPORT void setAboutData(KAboutData*);
EXPORT KAboutData *getAboutData();

EXPORT void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl, bool bAddEmpty = true, const ext_info *tbl1 = NULL);
EXPORT unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl, const ext_info *tbl1 = NULL);
EXPORT void disableWidget(QWidget *w);

EXPORT QString toTranslit(const QString&);
EXPORT bool isLatin(const QString&);
EXPORT QString getPart(QString&, unsigned size);

EXPORT unsigned screens();
EXPORT QRect screenGeometry(unsigned nScreen=~0U);

EXPORT unsigned get_random();

class EXPORT my_string
{
public:
    my_string(const char *str);
    my_string(const QString &str);
    ~my_string();
    bool operator < (const my_string &str) const;
    void operator = (const my_string &str);
    const QString &str() const { return m_str; }
protected:
    QString m_str;
};

EXPORT bool inResize();
EXPORT bool logEnabled();
EXPORT void setLogEnable(bool);

} // namespace SIM

EXPORT QString get_os_version();

#endif

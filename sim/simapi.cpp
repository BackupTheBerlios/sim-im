/***************************************************************************
                          simapi.cpp  -  description
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

#include "simapi.h"

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#if !defined(QT_MACOSX_VERSION) && !defined(QT_MAC)
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#endif

#include <time.h>

#include <stdio.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qicon.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qwidget.h>
#include <qdatetime.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qregexp.h>

#include <qdesktopwidget.h>

#ifdef USE_KDE
#include <kwin.h>
#include <kglobal.h>
#include "kdeisversion.h"
#endif

#include "stl.h"

// _____________________________________________________________________________________

#ifndef USE_KDE

QString i18n(const char *text)
{
    if ((text == NULL) || (*text == 0))
        return QString::null;
    QString res = QObject::tr(text);
    if (res != QString::fromLatin1(text))
        return res;
    return QString::fromUtf8(text);
}

QString i18n(const char *comment, const char *text)
{
    if (text == NULL)
        return QString::null;
    if (comment == NULL)
        return i18n(text);
    QString s;
    s = "_: ";
    s += comment;
    s += "\n";
    s += text;
    QString res = QObject::tr(s);
    if (res != s)
        return res;
    return i18n(text);
}


static bool bPluralInit = false;
static int plural_form = -1;

static void initPlural()
{
    if (bPluralInit) return;
    bPluralInit = true;
    QString pf = i18n("_: Dear translator, please do not translate this string "
                      "in any form, but pick the _right_ value out of "
                      "NoPlural/TwoForms/French... If not sure what to do mail "
                      "thd@kde.org and coolo@kde.org, they will tell you. "
                      "Better leave that out if unsure, the programs will "
                      "crash!!\nDefinition of PluralForm - to be set by the "
                      "translator of kdelibs.po");
    if ( pf == "NoPlural" )
        plural_form = 0;
    else if ( pf == "TwoForms" )
        plural_form = 1;
    else if ( pf == "French" )
        plural_form = 2;
    else if ( pf == "Gaeilge" )
        plural_form = 3;
    else if ( pf == "Russian" )
        plural_form = 4;
    else if ( pf == "Polish" )
        plural_form = 5;
    else if ( pf == "Slovenian" )
        plural_form = 6;
    else if ( pf == "Lithuanian" )
        plural_form = 7;
    else if ( pf == "Czech" )
        plural_form = 8;
    else if ( pf == "Slovak" )
        plural_form = 9;
    else if ( pf == "Maltese" )
        plural_form = 10;
}

void resetPlural()
{
    bPluralInit = false;
    initPlural();
}

QString put_n_in(const QString &orig, unsigned long n)
{
    QString ret = orig;
    int index = ret.find("%n");
    if (index == -1)
        return ret;
    ret.replace(index, 2, QString::number(n));
    return ret;
}

#define EXPECT_LENGTH(x)														\
	if (forms.count() != x){													\
		SIM::log(SIM::L_WARN, "Broken translation %s", singular);							\
		goto NoTranslate;														\
	}

QString i18n(const char *singular, const char *plural, unsigned long n)
{
    if (!singular || !singular[0] || !plural || !plural[0])
        return QString::null;
    char *newstring = new char[strlen(singular) + strlen(plural) + 6];
    sprintf(newstring, "_n: %s\n%s", singular, plural);
    QString r = i18n(newstring);
    if (r == newstring){
        delete[] newstring;
        goto NoTranslate;
    }
    delete [] newstring;
    initPlural();
    if ( r.isEmpty() || plural_form == -1) {
NoTranslate:
        if ( n == 1 )
            return put_n_in( QString::fromUtf8( singular ),  n );
        else
            return put_n_in( QString::fromUtf8( plural ),  n );
    }
    QStringList forms = QStringList::split( "\n", r, false );
    switch ( plural_form ) {
    case 0: // NoPlural
        EXPECT_LENGTH( 1 );
        return put_n_in( forms[0], n);
    case 1: // TwoForms
        EXPECT_LENGTH( 2 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 2: // French
        EXPECT_LENGTH( 2 );
        if ( n == 1 || n == 0 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 3: // Gaeilge
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n == 2 )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 4: // Russian, corrected by mok
        EXPECT_LENGTH( 3 );
        if ( n%10 == 1  &&  n%100 != 11)
            return put_n_in( forms[0], n); // odin fail
        else if (( n%10 >= 2 && n%10 <=4 ) && (n%100<10 || n%100>20))
            return put_n_in( forms[1], n); // dva faila
        else
            return put_n_in( forms[2], n); // desyat' failov
    case 5: // Polish
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n%10 >= 2 && n%10 <=4 && (n%100<10 || n%100>=20) )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 6: // Slovenian
        EXPECT_LENGTH( 4 );
        if ( n%100 == 1 )
            return put_n_in( forms[1], n); // ena datoteka
        else if ( n%100 == 2 )
            return put_n_in( forms[2], n); // dve datoteki
        else if ( n%100 == 3 || n%100 == 4 )
            return put_n_in( forms[3], n); // tri datoteke
        else
            return put_n_in( forms[0], n); // sto datotek
    case 7: // Lithuanian
        EXPECT_LENGTH( 3 );
        if ( n%10 == 0 || (n%100>=11 && n%100<=19) )
            return put_n_in( forms[2], n);
        else if ( n%10 == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 8: // Czech
        EXPECT_LENGTH( 3 );
        if ( n%100 == 1 )
            return put_n_in( forms[0], n);
        else if (( n%100 >= 2 ) && ( n%100 <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 9: // Slovak
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if (( n >= 2 ) && ( n <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 10: // Maltese
        EXPECT_LENGTH( 4 );
        if ( n == 1 )
            return put_n_in( forms[0], n );
        else if ( ( n == 0 ) || ( n%100 > 0 && n%100 <= 10 ) )
            return put_n_in( forms[1], n );
        else if ( n%100 > 10 && n%100 < 20 )
            return put_n_in( forms[2], n );
        else
            return put_n_in( forms[3], n );
    }
    return QString::null;
}

#endif

// ______________________________________________________________________________________

namespace SIM
{

using namespace std;

static list<EventReceiver*> *receivers = NULL;

EventReceiver::EventReceiver(unsigned priority)
{
    m_priority = priority;
    list<EventReceiver*>::iterator it;
    for (it = receivers->begin(); it != receivers->end(); ++it)
        if ((*it)->priority() >= priority)
            break;
    receivers->insert(it, this);
}

EventReceiver::~EventReceiver()
{
    list<EventReceiver*>::iterator it;
    for (it = receivers->begin(); it != receivers->end(); ++it){
        if ((*it) == this){
            receivers->erase(it);
            break;
        }
    }
}

void *Event::process(EventReceiver *from)
{
    if (receivers == NULL)
        return NULL;
    list<EventReceiver*>::iterator it = receivers->begin();
    if (from){
        for (; it != receivers->end(); ++it){
            if ((*it) == from){
                ++it;
                break;
            }
        }
    }
    for (; it != receivers->end(); ++it){
        EventReceiver *receiver = *it;
        if (receiver) {
            void *res = receiver->processEvent(this);
            if (res)
                return res;
        }
    }
    return NULL;
}

void EventReceiver::initList()
{
    receivers = new list<EventReceiver*>;
}

void EventReceiver::destroyList()
{
    delete receivers;
}

#ifdef WIN32

static WNDPROC oldWndProc = 0;
static bool bResize     = false;
static MSG m;

bool inResize()
{
    return bResize;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_ENTERSIZEMOVE)
        bResize = true;
    if (msg == WM_EXITSIZEMOVE)
        bResize = false;
    if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST)){
        m.hwnd = hWnd;
        m.message = msg;
        m.wParam = wParam;
        m.lParam = lParam;
    }
    return oldWndProc(hWnd, msg, wParam, lParam);
}

void setWndProc(QWidget *w)
{
    WNDPROC p;
    if (IsWindowUnicode(w->winId())){
        p = (WNDPROC)SetWindowLongW(w->winId(), GWL_WNDPROC, (LONG)WndProc);
    }else{
        p = (WNDPROC)SetWindowLongA(w->winId(), GWL_WNDPROC, (LONG)WndProc);
    }
    if (oldWndProc == NULL) oldWndProc = p;
}

#else
#if !defined(QT_MACOSX_VERSION) && !defined(QT_MAC)

void setWndClass(QWidget *w, const char *name)
{
    Display *dsp = w->x11Display();
    WId win = w->winId();

    XClassHint classhint;
    classhint.res_name  = (char*)"sim";
    classhint.res_class = (char*)name;
    XSetClassHint(dsp, win, &classhint);

    XWMHints *hints;  
    hints = XGetWMHints(dsp, win);  
    hints->window_group = win;  
    hints->flags = WindowGroupHint;  
    XSetWMHints(dsp, win, hints);  
    XFree( hints );

    const char *argv[2];
    argv[0] = name;
    argv[1] = NULL;
    XSetCommand(dsp, win, (char**)argv, 1);
}

#else

void setWndClass(QWidget*, const char*)
{
}

#endif
#endif

#ifdef USE_KDE
bool raiseWindow(QWidget *w, unsigned desk)
#else
bool raiseWindow(QWidget *w, unsigned)
#endif
{
    Event e(EventRaiseWindow, w);
    if (e.process())
        return false;
#ifdef USE_KDE
    /* info.currentDesktop is 0 when iconified :(
    also onAllDesktops is 0 when Objekt isn't
    shown already */
#if KDE_IS_VERSION(3,2,0)
    KWin::WindowInfo info = KWin::windowInfo(w->winId());
    if ((!info.onAllDesktops()) || (desk == 0)) {
        if (desk == 0) desk = KWin::currentDesktop();
        KWin::setOnDesktop(w->winId(), desk);
    }
#else
    KWin::Info info = KWin::info(w->winId());
    if ((!info.onAllDesktops) || (desk == 0)) {
        if (desk == 0) desk = KWin::currentDesktop();
        KWin::setOnDesktop(w->winId(), desk);
    }
#endif
#endif
#ifdef WIN32
    DWORD dwProcID = GetWindowThreadProcessId(GetForegroundWindow(),NULL);
    if(dwProcID != GetCurrentThreadId())
        AttachThreadInput(dwProcID, GetCurrentThreadId(), TRUE);
#endif
    w->show();
    if (w->isMinimized()) {
        if (w->isMaximized())
            w->showMaximized();
        else
            w->showNormal();
    }
    w->raise();
#ifdef WIN32
    SetForegroundWindow(w->winId());
    SetFocus(w->winId());
    if(dwProcID != GetCurrentThreadId())
        AttachThreadInput(dwProcID, GetCurrentThreadId(), FALSE);
#endif
    return true;
}

void setButtonsPict(QWidget *w)
{
    QObjectList *l = w->queryList( "QPushButton" );
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        QPushButton *btn = static_cast<QPushButton*>(obj);
        if (btn->pixmap()) continue;
        const QString &text = btn->text();
        const char *icon = NULL;
        if ((text == i18n("&OK")) || (text == i18n("&Yes")) ||
                (text == i18n("&Apply")) || (text == i18n("&Register"))){
            icon = "button_ok";
        }else if ((text == i18n("&Cancel")) || (text == i18n("&Close")) ||
                  (text == i18n("&No"))){
            icon = "button_cancel";
        }else if (text == i18n("&Help")){
            icon = "help";
        }
        if (icon == NULL) continue;
        btn->setIconSet(Icon(icon));
    }
    delete l;
}

EXPORT QString formatDateTime(unsigned long t)
{
    if (t == 0) return "";
    QDateTime time;
    time.setTime_t(t);
#ifdef USE_KDE
    return KGlobal::locale()->formatDateTime(time);
#else
    return time.toString();
#endif
}

EXPORT QString formatDate(unsigned long t)
{
    if (t == 0) return "";
    QDateTime time;
    time.setTime_t(t);
#ifdef USE_KDE
    return KGlobal::locale()->formatDate(time.date());
#else
    return time.date().toString();
#endif
}

EXPORT QString formatAddr(const Data &ip, unsigned port)
{
    QString res;
    if (ip.ip() == NULL) {
        log( L_ERROR, "formatAddr() with invalid data (%s)", ip.name().latin1() );
        return res;
    }
    struct in_addr inaddr;
    inaddr.s_addr = get_ip(ip);
    res += inet_ntoa(inaddr);
    if (port){
        res += ":";
        res += QString::number(port);
    }
    const char *host = get_host(ip);
    if (host && *host){
        res += " ";
        res += host;
    }
    return res;
}

void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl, bool bAddEmpty, const ext_info *tbl1)
{
    if (cmb->isEnabled()){
        cmb->clear();
        if (bAddEmpty)
            cmb->insertItem("");
        QStringList items;
        QString current;
        for (const ext_info *i = tbl; i->nCode; i++){
            if (tbl1){
                const ext_info *ii;
                for (ii = tbl1; ii->nCode; ii++)
                    if (ii->nCode == i->nCode)
                        break;
                if (ii->nCode == 0)
                    continue;
            }
            items.append(i18n(i->szName));
            if (i->nCode == code)
                current = i18n(i->szName);
        }
        items.sort();
        cmb->insertStringList(items);
        unsigned n = bAddEmpty ? 1 : 0;
        if (!current.isEmpty()){
            for (QStringList::Iterator it = items.begin(); it != items.end(); ++it, n++){
                if ((*it) == current){
                    cmb->setCurrentItem(n);
                    break;
                }
            }
        }
    }else{
        for (const ext_info *i = tbl; i->nCode; i++){
            if (i->nCode == code){
                cmb->insertItem(i18n(i->szName));
                return;
            }
        }
    }
}

unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl, const ext_info *tbl1)
{
    int res = cmb->currentItem();
    if (res <= 0) return 0;
    QStringList items;
    const ext_info *i;
    for (i = tbl; i->nCode; i++){
        if (tbl1){
            const ext_info *ii;
            for (ii = tbl1; ii->nCode; ii++)
                if (ii->nCode == i->nCode)
                    break;
            if (ii->nCode == 0)
                continue;
        }
        items.append(i18n(i->szName));
    }
    items.sort();
    if (cmb->text(0).isEmpty()) res--;
    QString current = items[res];
    for (i = tbl; i->nCode; i++)
        if (i18n(i->szName) == current) return i->nCode;
    return 0;
}

EXPORT void disableWidget(QWidget *w)
{
    QPalette pal = w->palette();
    pal.setDisabled(pal.active());
    w->setPalette(pal);
    if (w->inherits("QLineEdit")){
        static_cast<QLineEdit*>(w)->setReadOnly(true);
    }else if (w->inherits("QMulitLineEdit")){
        static_cast<QMulitLineEdit*>(w)->setReadOnly(true);
    }else{
        w->setEnabled(false);
    }
}

const unsigned char NOP = 172;

static unsigned char gsmToLatin1Table[] =
    {
        '@', 163, '$', 165, 232, 233, 249, 236,
        242, 199,  10, 216, 248,  13, 197, 229,
        NOP, '_', NOP, NOP, NOP, NOP, NOP, NOP,
        NOP, NOP, NOP, NOP, 198, 230, 223, 201,
        ' ', '!', '"', '#', 164, '%', '&', '\'',
        '(', ')', '*', '+', ',', '-', '.', '/',
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', ':', ';', '<', '=', '>', '?',
        161, 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', 196, 214, 209, 220, 167,
        191, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
        'x', 'y', 'z', 228, 246, 241, 252, 224
    };

EXPORT bool isLatin(const QString &str)
{
    for (int i = 0; i < (int)str.length(); i++){
        unsigned short c = str[i].unicode();
        if ((c > 255) || (c == NOP))
            return false;
        unsigned n;
        for (n = 0; n < sizeof(gsmToLatin1Table); n++){
            if (gsmToLatin1Table[n] == c)
                break;
        }
        if (n >= sizeof(gsmToLatin1Table))
            return false;
    }
    return true;
}

EXPORT QString getPart(QString &str, unsigned size)
{
    QString res;
    if (str.length() < size){
        res = str;
        str = "";
        return res;
    }
    int n = size;
    QChar c = str[(int)size];
    if (!c.isSpace()){
        for (; n >= 0; n--){
            c = str[n];
            if (c.isSpace())
                break;
        }
        if (n < 0)
            n = size;
    }
    res = str.left(n);
    str = str.mid(n);
    return res;
}

#if 0
I18N_NOOP("Smile")
I18N_NOOP("Surprised")
I18N_NOOP("Indifferent")
I18N_NOOP("Skeptical")
I18N_NOOP("Sad")
I18N_NOOP("Kiss")
I18N_NOOP("Annoyed")
I18N_NOOP("Crying")
I18N_NOOP("Wink")
I18N_NOOP("Angry")
I18N_NOOP("Embarrassed")
I18N_NOOP("Uptight")
I18N_NOOP("Teaser")
I18N_NOOP("Cool")
I18N_NOOP("Angel")
I18N_NOOP("Grin")
#endif

EXPORT unsigned screens()
{
    QDesktopWidget *desktop = QApplication::desktop();
    return desktop->numScreens();
}

EXPORT QRect screenGeometry(unsigned nScreen)
{
    QDesktopWidget *desktop = QApplication::desktop();
    if(nScreen == ~0U) {
        QRect rc;
        for (int i = 0; i < desktop->numScreens(); i++){
            rc |= desktop->screenGeometry(i);
        }
        return rc;
    }
    return desktop->screenGeometry(nScreen);
}

static bool bLog = true;

EXPORT bool logEnabled()
{
    return bLog;
}

EXPORT void setLogEnable(bool log)
{
    bLog = log;
}

static bool bRandInit = false;

EXPORT unsigned get_random()
{
    if (!bRandInit){
        bRandInit = true;
        srand(time(NULL));
    }
    return rand();
}

my_string::my_string(const char *str)
{
    m_str = QString::fromUtf8(str);
}

my_string::my_string(const QString &s)
{
    m_str = s;
}

void my_string::operator = (const my_string &s)
{
    m_str = s.m_str;
}

my_string::~my_string()
{
}

bool my_string::operator < (const my_string &a) const
{
    return QString::compare(m_str, a.m_str) < 0;
}

}


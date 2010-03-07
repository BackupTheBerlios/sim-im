/***************************************************************************
                          shortcuts.cpp  -  description
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

#include "simapi.h"

#include "log.h"
#include "cmddef.h"
#include "core.h"

#include "profile.h"
#include "profilemanager.h"

#include "shortcuts.h"
#include "shortcutcfg.h"
#include "mainwin.h"

#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QByteArray>
#include <QMenu>

#ifdef WIN32
	#include <windows.h>
#else
	#if !defined(Q_OS_MAC)
		#include <QX11Info>
		#ifdef USE_KDE
			#include <kglobalaccel.h>
		#else
			#define XK_MISCELLANY 1
			#include <X11/X.h>
			#include <X11/Xlib.h>
			#include <X11/Xutil.h>
			#include <X11/keysymdef.h>
		#endif
	#endif
#endif

using namespace std;
using namespace SIM;

Plugin *createShortcutsPlugin(unsigned base, bool, Buffer *config)
{
    return new ShortcutsPlugin(base, config);
}

static PluginInfo info =
    {
        I18N_NOOP("Shortcuts"),
        I18N_NOOP("Plugin provides keyboards shortcuts and global mouse functions"),
        VERSION,
        createShortcutsPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

//static DataDef shortcutsData[] =
//    {
//        { "Key", DATA_STRLIST, 1, 0 },
//        { "Global", DATA_STRLIST, 1, 0 },
//        { "Mouse", DATA_STRLIST, 1, 0 },
//        { NULL, DATA_UNKNOWN, 0, 0 }
//    };

void GlobalKey::execute()
{
    EventCommandExec(&m_cmd).process();;
}

list<GlobalKey*> *globalKeys = NULL;

#ifdef WIN32

struct vkCode
{
    int vk;
    Qt::Key key;
};

static vkCode vkCodes[] =
    {
        { VK_BACK,		Qt::Key_Backspace },
        { VK_TAB,		Qt::Key_Tab },
        { VK_RETURN,	Qt::Key_Return },
        { VK_SHIFT,		Qt::Key_Shift },
        { VK_CONTROL,	Qt::Key_Control },
        { VK_MENU,		Qt::Key_Menu },
        { VK_PAUSE,		Qt::Key_Pause },
        { VK_ESCAPE,	Qt::Key_Escape },
        { VK_SPACE,		Qt::Key_Space },
        { VK_PRIOR,		Qt::Key_PageUp },
        { VK_NEXT,		Qt::Key_PageDown },
        { VK_END,		Qt::Key_End },
        { VK_HOME,		Qt::Key_Home },
        { VK_LEFT,		Qt::Key_Left },
        { VK_UP,		Qt::Key_Up },
        { VK_RIGHT,		Qt::Key_Right },
        { VK_DOWN,		Qt::Key_Down },
        { VK_PRINT,		Qt::Key_Print },
        { VK_INSERT,	Qt::Key_Insert },
        { VK_DELETE,	Qt::Key_Delete },
        { VK_HELP,		Qt::Key_Help },
        { '0',			Qt::Key_0 },
        { '1',			Qt::Key_1 },
        { '2',			Qt::Key_2 },
        { '3',			Qt::Key_3 },
        { '4',			Qt::Key_4 },
        { '5',			Qt::Key_5 },
        { '6',			Qt::Key_6 },
        { '7',			Qt::Key_7 },
        { '8',			Qt::Key_8 },
        { '9',			Qt::Key_9 },
        { 'A',			Qt::Key_A },
        { 'B',			Qt::Key_B },
        { 'C',			Qt::Key_C },
        { 'D',			Qt::Key_D },
        { 'E',			Qt::Key_E },
        { 'F',			Qt::Key_F },
        { 'G',			Qt::Key_G },
        { 'H',			Qt::Key_H },
        { 'I',			Qt::Key_I },
        { 'J',			Qt::Key_J },
        { 'K',			Qt::Key_K },
        { 'L',			Qt::Key_L },
        { 'M',			Qt::Key_M },
        { 'N',			Qt::Key_N },
        { 'O',			Qt::Key_O },
        { 'P',			Qt::Key_P },
        { 'Q',			Qt::Key_Q },
        { 'R',			Qt::Key_R },
        { 'S',			Qt::Key_S },
        { 'T',			Qt::Key_T },
        { 'U',			Qt::Key_U },
        { 'V',			Qt::Key_V },
        { 'W',			Qt::Key_W },
        { 'X',			Qt::Key_X },
        { 'Y',			Qt::Key_Y },
        { 'Z',			Qt::Key_Z },
        { VK_ADD,		Qt::Key_Plus },
        { VK_SUBTRACT,	Qt::Key_Minus },
        { VK_DECIMAL,	Qt::Key_Period },
        { VK_F1,		Qt::Key_F1 },
        { VK_F2,		Qt::Key_F2 },
        { VK_F3,		Qt::Key_F3 },
        { VK_F4,		Qt::Key_F4 },
        { VK_F5,		Qt::Key_F5 },
        { VK_F6,		Qt::Key_F6 },
        { VK_F7,		Qt::Key_F7 },
        { VK_F8,		Qt::Key_F8 },
        { VK_F9,		Qt::Key_F9 },
        { VK_F10,		Qt::Key_F10 },
        { VK_F11,		Qt::Key_F11 },
        { VK_F12,		Qt::Key_F12 },
        { VK_F13,		Qt::Key_F13 },
        { VK_F14,		Qt::Key_F14 },
        { VK_F15,		Qt::Key_F15 },
        { VK_F16,		Qt::Key_F16 },
        { VK_F17,		Qt::Key_F17 },
        { VK_F18,		Qt::Key_F18 },
        { VK_F19,		Qt::Key_F19 },
        { VK_F20,		Qt::Key_F20 },
        { VK_F21,		Qt::Key_F21 },
        { VK_F22,		Qt::Key_F22 },
        { VK_F23,		Qt::Key_F23 },
        { VK_F24,		Qt::Key_F24 },
        { VK_NUMLOCK,	Qt::Key_NumLock },
        { VK_SCROLL,	Qt::Key_ScrollLock },
        { 0,			(Qt::Key)0 }
    };

static void getKey(const QString &key_str, int &mod, int &key)
{
    mod = 0;
    key = 0;
    int kkey = QKeySequence::fromString(key_str);
    if (kkey & Qt::ALT) mod |= MOD_ALT;
    if (kkey & Qt::CTRL) mod |= MOD_CONTROL;
    if (kkey & Qt::SHIFT) mod |= MOD_SHIFT;
    kkey &= ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL);
    for (const vkCode *k = vkCodes; k->vk; k++){
        if (k->key == kkey){
            key = k->vk;
            break;
        }
    }
}

GlobalKey::GlobalKey(CommandDef *cmd)
{
    m_cmd = *cmd;
    int mod, key;
    getKey(cmd->accel, mod, key);
    QWidget *main = ShortcutsPlugin::getMainWindow();
    if (key && main){
        QString atom = "sim_" + QString::number(cmd->id);
        m_key = GlobalAddAtom((LPCWSTR)atom.utf16());
        RegisterHotKey(main->winId(), m_key, mod, key);
    }
}

GlobalKey::~GlobalKey()
{
    QWidget *main = ShortcutsPlugin::getMainWindow();
    if (m_key && main){
        UnregisterHotKey(main->winId(), m_key);
        DeleteAtom((unsigned short)m_key);
    }
}

static WNDPROC oldProc = NULL;

LRESULT CALLBACK keysWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_HOTKEY){
        if (globalKeys){
            for (list<GlobalKey*>::iterator it = globalKeys->begin(); it != globalKeys->end(); ++it){
                if ((UINT)((*it)->key()) == wParam){
                    (*it)->execute();
                    break;
                }
            }
        }
    }
    if (oldProc)
        return oldProc(hWnd, msg, wParam, lParam);
    return 0;
}

#else  // WIN32
#ifdef USE_KDE

GlobalKey::GlobalKey(CommandDef *cmd)
{
    m_cmd = *cmd;
    QKeySequence keys(cmd->accel);
    if (keys != QKeySequence(0)){
        QString shortName = "sim_" + QString::number(cmd->id);
        accel = new KGlobalAccel(this);
        accel->insert(shortName,
                      i18n(cmd->text), i18n(cmd->text),
                      keys, keys, this, SLOT(execute()));
        accel->updateConnections();
    }
}

GlobalKey::~GlobalKey()
{
    if (accel)
        delete accel;
}

#else  // USE_KDE
#if !defined(Q_OS_MAC)

struct TransKey
{
    unsigned qt_key;
    unsigned x_key;
};

static const TransKey g_rgQtToSymX[] =
    {
        { Qt::Key_Escape,     XK_Escape },
        { Qt::Key_Tab,        XK_Tab },
        { Qt::Key_Backspace,  XK_BackSpace },
        { Qt::Key_Return,     XK_Return },
        { Qt::Key_Enter,      XK_KP_Enter },
        { Qt::Key_Insert,     XK_Insert },
        { Qt::Key_Delete,     XK_Delete },
        { Qt::Key_Pause,      XK_Pause },
        { Qt::Key_Print,      XK_Print },
        { Qt::Key_SysReq,     XK_Sys_Req },
        { Qt::Key_Home,       XK_Home },
        { Qt::Key_End,        XK_End },
        { Qt::Key_Left,       XK_Left },
        { Qt::Key_Up,         XK_Up },
        { Qt::Key_Right,      XK_Right },
        { Qt::Key_Down,       XK_Down },
        { Qt::Key_PageUp,      XK_Prior },
        { Qt::Key_PageDown,       XK_Next },
        { Qt::Key_CapsLock,   XK_Caps_Lock },
        { Qt::Key_NumLock,    XK_Num_Lock },
        { Qt::Key_ScrollLock, XK_Scroll_Lock },
        { Qt::Key_F1,         XK_F1 },
        { Qt::Key_F2,         XK_F2 },
        { Qt::Key_F3,         XK_F3 },
        { Qt::Key_F4,         XK_F4 },
        { Qt::Key_F5,         XK_F5 },
        { Qt::Key_F6,         XK_F6 },
        { Qt::Key_F7,         XK_F7 },
        { Qt::Key_F8,         XK_F8 },
        { Qt::Key_F9,         XK_F9 },
        { Qt::Key_F10,        XK_F10 },
        { Qt::Key_F11,        XK_F11 },
        { Qt::Key_F12,        XK_F12 },
        { Qt::Key_F13,        XK_F13 },
        { Qt::Key_F14,        XK_F14 },
        { Qt::Key_F15,        XK_F15 },
        { Qt::Key_F16,        XK_F16 },
        { Qt::Key_F17,        XK_F17 },
        { Qt::Key_F18,        XK_F18 },
        { Qt::Key_F19,        XK_F19 },
        { Qt::Key_F20,        XK_F20 },
        { Qt::Key_F21,        XK_F21 },
        { Qt::Key_F22,        XK_F22 },
        { Qt::Key_F23,        XK_F23 },
        { Qt::Key_F24,        XK_F24 },
        { Qt::Key_F25,        XK_F25 },
        { Qt::Key_F26,        XK_F26 },
        { Qt::Key_F27,        XK_F27 },
        { Qt::Key_F28,        XK_F28 },
        { Qt::Key_F29,        XK_F29 },
        { Qt::Key_F30,        XK_F30 },
        { Qt::Key_F31,        XK_F31 },
        { Qt::Key_F32,        XK_F32 },
        { Qt::Key_F33,        XK_F33 },
        { Qt::Key_F34,        XK_F34 },
        { Qt::Key_F35,        XK_F35 },
        { Qt::Key_Super_L,    XK_Super_L },
        { Qt::Key_Super_R,    XK_Super_R },
        { Qt::Key_Menu,       XK_Menu },
        { Qt::Key_Hyper_L,    XK_Hyper_L },
        { Qt::Key_Hyper_R,    XK_Hyper_R },
        { Qt::Key_Help,       XK_Help },
        { '/',                XK_KP_Divide },
        { '*',                XK_KP_Multiply },
        { '-',                XK_KP_Subtract },
        { '+',                XK_KP_Add },
        { Qt::Key_Return,     XK_KP_Enter },
        { 0, 0 }
    };

extern "C" {
    static int XGrabErrorHandler( Display *, XErrorEvent *e ) {
        if ( e->error_code != BadAccess ) {
            log(L_DEBUG, "grabKey: got X error %u instead of BadAccess", e->type);
        }
        return 0;
    }
}

// from KGlobalAccel
static uint g_keyModMaskXOnOrOff = 0;
static void initializeMods()
{
    uint g_modXNumLock, g_modXScrollLock, g_modXModeSwitch; 
    g_modXNumLock = g_modXScrollLock = g_modXModeSwitch = 0; 
    int min_keycode, max_keycode;
    int keysyms_per_keycode = 0;

    XModifierKeymap* xmk = XGetModifierMapping(QX11Info::display() );
    XDisplayKeycodes(QX11Info::display(), &min_keycode, &max_keycode );
    XFree( XGetKeyboardMapping(QX11Info::display(), min_keycode, 1, &keysyms_per_keycode ));
    for( int i = Mod2MapIndex; i < 8; i++ ) {
        uint mask = (1 << i);
        uint keySymX = NoSymbol;
        for( int j = 0; j < xmk->max_keypermod && keySymX == NoSymbol; ++j )
            for( int k = 0; k < keysyms_per_keycode && keySymX == NoSymbol; ++k )
                keySymX = XKeycodeToKeysym(QX11Info::display(), xmk->modifiermap[xmk->max_keypermod * i + j], k );
        switch(keySymX)
		{
            case XK_Num_Lock:    g_modXNumLock = mask; break;     // Normally Mod2Mask
            case XK_Scroll_Lock: g_modXScrollLock = mask; break;  // Normally Mod5Mask
            case XK_Mode_switch: g_modXModeSwitch = mask; break; 
        }
    }
    XFreeModifiermap(xmk);
    g_keyModMaskXOnOrOff = LockMask | g_modXNumLock | g_modXScrollLock | g_modXModeSwitch;
}

GlobalKey::GlobalKey(CommandDef *cmd)
{
    m_cmd = *cmd;
    m_key = QKeySequence::fromString(cmd->accel);
    m_state = 0;
    if (m_key & Qt::SHIFT){
        m_key &= ~Qt::SHIFT;
        m_state |= 1;
    }
    if (m_key & Qt::CTRL){
        m_key &= ~Qt::CTRL;
        m_state |= 4;
    }
    if (m_key & Qt::ALT){
        m_key &= ~Qt::ALT;
        m_state |= 8;
    }
    m_key &= ~Qt::UNICODE_ACCEL;
    for (const TransKey *t = g_rgQtToSymX; t->x_key; t++){
        if (t->qt_key == m_key){
            m_key = t->x_key;
            break;
        }
    }

    if(!g_keyModMaskXOnOrOff)
        initializeMods();

    m_key = XKeysymToKeycode(QX11Info::display(), m_key);
    XSync(QX11Info::display(), 0 );
    XErrorHandler savedErrorHandler = XSetErrorHandler(XGrabErrorHandler);

    uint keyModMaskX = ~g_keyModMaskXOnOrOff;
	/*
    for( uint irrelevantBitsMask = 0; irrelevantBitsMask <= 0xff; irrelevantBitsMask++ )
        if( (irrelevantBitsMask & keyModMaskX) == 0 )
            XGrabKey(QX11Info::display(), m_key, m_state | irrelevantBitsMask, qt_xrootwin(), True, GrabModeAsync, GrabModeSync);
			*/

    XSync(QX11Info::display(), 0 );
    XSetErrorHandler( savedErrorHandler );
}

GlobalKey::~GlobalKey()
{
    uint keyModMaskX = ~g_keyModMaskXOnOrOff;

	/*
    for( uint irrelevantBitsMask = 0; irrelevantBitsMask <= 0xff; irrelevantBitsMask++ )
        if( (irrelevantBitsMask & keyModMaskX) == 0 )
            XUngrabKey( qt_xdisplay(), m_key, m_state | irrelevantBitsMask, qt_xrootwin());
			*/
}

typedef int (*QX11EventFilter) (::XEvent*);
//QX11EventFilter qt_set_x11_event_filter (QX11EventFilter filter);
static QX11EventFilter oldFilter;

static int X11EventFilter(::XEvent *e)
{
    if ((e->type == KeyPress) && globalKeys){
        if ( !QWidget::keyboardGrabber() && !QApplication::activePopupWidget() ) {
                XUngrabKeyboard(QX11Info::display(), e->xkey.time );
                XFlush(QX11Info::display());
        }
        unsigned state = e->xkey.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask | 0x2000);

        for (list<GlobalKey*>::iterator it = globalKeys->begin(); it != globalKeys->end(); ++it){
            if (((*it)->key() == e->xkey.keycode) &&
                    ((*it)->state() == state)){
                (*it)->execute();
                return true;
            }
        }
    }
    if (oldFilter)
        return oldFilter(e);
    return false;
}

#else  // !Q_OS_MAC

GlobalKey::GlobalKey(SIM::CommandDef *cmd)
{
}

GlobalKey::~GlobalKey()
{
}

#endif // !Q_OS_MAC
#endif  // USE_KDE
#endif  // WIN32

ShortcutsPlugin::ShortcutsPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
{
    m_propertyHub = SIM::PropertyHub::create("shortcuts");
#ifdef WIN32
    m_bInit = false;
    init();
#else
    applyKeys();
#ifndef USE_KDE
//    oldFilter = qt_set_x11_event_filter(X11EventFilter);
#endif
#endif
}

ShortcutsPlugin::~ShortcutsPlugin()
{
#ifdef WIN32
    QWidget *main = getMainWindow();
    if (main && oldProc){
        if (IsWindowUnicode(main->winId())){
            SetWindowLongW(main->winId(), GWL_WNDPROC, (LONG)oldProc);
        }else{
            SetWindowLongA(main->winId(), GWL_WNDPROC, (LONG)oldProc);
        }
    }
#else
#ifndef USE_KDE
    //qt_set_x11_event_filter(oldFilter);
#endif
#endif
    releaseKeys();
}

QByteArray ShortcutsPlugin::getConfig()
{
    return QByteArray();//Fixmee
}

QWidget *ShortcutsPlugin::createConfigWindow(QWidget *parent)
{
    return new ShortcutsConfig(parent, this);
}

#ifdef WIN32

void ShortcutsPlugin::init()
{
    if (m_bInit)
        return;
    QWidget *main = getMainWindow();
    if (main){
        if (IsWindowUnicode(main->winId())){
            oldProc = (WNDPROC)SetWindowLongW(main->winId(), GWL_WNDPROC, (LONG)keysWndProc);
        }else{
            oldProc = (WNDPROC)SetWindowLongA(main->winId(), GWL_WNDPROC, (LONG)keysWndProc);
        }
        m_bInit = true;
        applyKeys();
    }
}

#endif

bool ShortcutsPlugin::processEvent(Event *e)
{
#ifdef WIN32
    if (e->type() == eEventInit){
        init();
        return false;
    } else
#endif
    if (e->type() == eEventCommandCreate){
        EventCommandCreate *ecc = static_cast<EventCommandCreate*>(e);
        CommandDef *cmd = ecc->cmd();
        if ((cmd->menu_id == MenuMain) ||
                (cmd->menu_id == MenuContact) ||
                (cmd->menu_id == MenuStatus) ||
                (cmd->menu_id == MenuGroup)){
            applyKey(cmd);
        }
    } else
    if (e->type() == eEventCommandRemove){
        EventCommandRemove *ecr = static_cast<EventCommandRemove*>(e);
        unsigned long id = ecr->id();
        MAP_STR::iterator it_key = oldKeys.find(id);
        if (it_key != oldKeys.end())
            oldKeys.erase(it_key);
        MAP_BOOL::iterator it_global = oldGlobals.find(id);
        if (it_global != oldGlobals.end())
            oldGlobals.erase(it_global);
        if (globalKeys){
            list<GlobalKey*>::iterator it;
            for (it = globalKeys->begin(); it != globalKeys->end();){
                if ((*it)->id() != id){
                    ++it;
                    continue;
                }
                delete *it;
                globalKeys->erase(it);
                it = globalKeys->begin();
            }
        }
        for (MAP_CMDS::iterator it = mouseCmds.begin(); it != mouseCmds.end();){
            if (it->second.id != id){
                ++it;
                continue;
            }
            mouseCmds.erase(it);
            it = mouseCmds.begin();
        }
        if (mouseCmds.size() == 0)
            qApp->removeEventFilter(this);
    }
	if(e->type() == eEventPluginLoadConfig)
	{
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("shortcut");
        if(!hub.isNull())
            setPropertyHub(hub);
	}
    return false;
}

QString ShortcutsPlugin::getOldKey(CommandDef *cmd)
{
    MAP_STR::iterator it = oldKeys.find(cmd->id);
    if (it != oldKeys.end())
        return it->second;
    return cmd->accel;
}

bool ShortcutsPlugin::getOldGlobal(CommandDef *cmd)
{
    MAP_BOOL::iterator it = oldGlobals.find(cmd->id);
    if (it != oldGlobals.end())
        return it->second;
    return ((cmd->flags & COMMAND_GLOBAL_ACCEL) != 0);
}

void ShortcutsPlugin::applyKeys()
{
    applyKeys(MenuMain);
    applyKeys(MenuGroup);
    applyKeys(MenuContact);
    applyKeys(MenuStatus);
}

void ShortcutsPlugin::releaseKeys()
{
    releaseKeys(MenuMain);
    releaseKeys(MenuGroup);
    releaseKeys(MenuContact);
    releaseKeys(MenuStatus);
    oldKeys.clear();
    oldGlobals.clear();
    if (globalKeys){
        list<GlobalKey*>::iterator it;
        for (it = globalKeys->begin(); it != globalKeys->end(); ++it){
            delete *it;
        }
        delete globalKeys;
        globalKeys = NULL;
    }
    mouseCmds.clear();
    qApp->removeEventFilter(this);
}

void ShortcutsPlugin::applyKeys(unsigned long id)
{
    EventMenuGetDef eMenu(id);
    eMenu.process();
    CommandsDef *def = eMenu.defs();
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if (s->id == 0) continue;
            applyKey(s);
        }
    }
}

static const char *states[] =
    {
        "Left",
        "Right",
        "Mid",
        "LeftDbl",
        "RightDbl",
        "MidDbl",
        NULL
    };

unsigned ShortcutsPlugin::stringToButton(const QString &cfg)
{
    Qt::KeyboardModifiers res = Qt::NoModifier;
    QStringList config = cfg.split('+');
    Q_FOREACH(const QString &t, config) {
        if (t == QLatin1String("Alt")){
            res |= Qt::AltModifier;
            continue;
        }
        if (t == QLatin1String("Ctrl")){
            res |= Qt::ControlModifier;
            continue;
        }
        if (t == QLatin1String("Shift")){
            res |= Qt::ShiftModifier;
            continue;
        }
        unsigned i = 1;
        for (const char **p = states; *p; p++, i++) {
            if (t == *p){
                res |= (Qt::KeyboardModifier)i;
                return res;
            }
        }
        return 0;
    }
    return 0;
}

QString ShortcutsPlugin::buttonToString(unsigned n)
{
    QString res;
// ToDo: Restore this
/*
    if (n & Qt::AltButton)
        res = "Alt+";
    if (n & Qt::ControlButton)
        res = "Ctrl+";
    if (n & Qt::ShiftButton)
        res = "Shift+";
*/
    n = n & 7;
    if (n == 0)
        return QString::null;
    n--;
    const char **p;
    for (p = states; *p && n; p++, n--) {}
    if (*p == NULL)
        return QString::null;
    res += *p;
    return res;
}

void ShortcutsPlugin::applyKey(CommandDef *s)
{
    if (s->popup_id){
        QString cfg = value("Mouse").toMap().value(QString::number(s->id)).toString();
        if (!cfg.isEmpty()){
            unsigned btn = stringToButton(cfg);
            if (mouseCmds.size() == 0)
                qApp->installEventFilter(this);
            mouseCmds.insert(MAP_CMDS::value_type(btn, *s));
        }
        return;
    }
        QString cfg = value("Key").toMap().value(QString::number(s->id)).toString();
    if (!cfg.isEmpty()){
        oldKeys.insert(MAP_STR::value_type(s->id, s->accel));
        if (cfg != "-"){
            s->accel = cfg;
        }else{
            s->accel = QString::null;
        }
    }
        cfg = value("Global").toMap().value(QString::number(s->id)).toString();
    if (!cfg.isEmpty()){
        oldGlobals.insert(MAP_BOOL::value_type(s->id, (s->flags & COMMAND_GLOBAL_ACCEL) != 0));
        if (cfg.startsWith("-")){
            s->flags &= ~COMMAND_GLOBAL_ACCEL;
        }else{
            s->flags |= COMMAND_GLOBAL_ACCEL;
        }
    }
    if (!s->accel.isEmpty() && (s->flags & COMMAND_GLOBAL_ACCEL))
	{
        if (globalKeys == NULL)
            globalKeys = new list<GlobalKey*>;
        globalKeys->push_back(new GlobalKey(s));
    }
}

void ShortcutsPlugin::releaseKeys(unsigned long id)
{
    EventMenuGetDef eMenu(id);
    eMenu.process();
    CommandsDef *def = eMenu.defs();
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if ((s->id == 0) || s->popup_id)
                continue;
            MAP_STR::iterator it_key = oldKeys.find(s->id);
            if (it_key != oldKeys.end())
                s->accel = (*it_key).second;
            MAP_BOOL::iterator it_global = oldGlobals.find(s->id);
            if (it_global != oldGlobals.end()){
                s->flags &= ~COMMAND_GLOBAL_ACCEL;
                if ((*it_global).second)
                    s->flags |= COMMAND_GLOBAL_ACCEL;
            }
        }
    }
}

bool ShortcutsPlugin::eventFilter(QObject *o, QEvent *e)
{
    unsigned button = 0;
    QMouseEvent *me = NULL;
    if (e->type() == QEvent::MouseButtonPress){
        me = static_cast<QMouseEvent*>(e);
        switch (me->button()){
        case Qt::LeftButton:
            button = 1;
            break;
        case Qt::RightButton:
            button = 2;
            break;
        case Qt::MidButton:
            button = 3;
            break;
        default:
            break;
        }
    }
    if (e->type() == QEvent::MouseButtonDblClick){
        me = static_cast<QMouseEvent*>(e);
        switch (me->button()){
        case Qt::LeftButton:
            button = 4;
            break;
        case Qt::RightButton:
            button = 5;
            break;
        case Qt::MidButton:
            button = 6;
            break;
        default:
            break;
        }
    }
    if (me){
        button |= me->modifiers();
        MAP_CMDS::iterator it = mouseCmds.find(button);
        if (it != mouseCmds.end()){
            CommandDef *cmd = &it->second;
            EventMenuGet e(cmd);
            e.process();
            QMenu *popup = e.menu();
            if (popup){
                popup->popup(me->globalPos());
                return true;
            }
        }
    }
    return QObject::eventFilter(o, e);
}

QWidget *ShortcutsPlugin::getMainWindow()
{
    CorePlugin *core = GET_CorePlugin();
    return core->getMainWindow();
}

void ShortcutsPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr ShortcutsPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant ShortcutsPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void ShortcutsPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

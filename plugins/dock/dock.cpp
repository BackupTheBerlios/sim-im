/***************************************************************************
                          dock.cpp  -  description
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

#include "dock.h"
#include "dockcfg.h"
#include "dockwnd.h"
#include "mainwin.h"
#include "misc.h"
#include "core_consts.h"
#include "cmddef.h"
#include "core.h"

#include "profile.h"
#include "profilemanager.h"

#include <QApplication>
#include <QWidget>
#include <QTimer>
#include <QMenu>

using namespace SIM;

Plugin *createDockPlugin(unsigned base, bool, Buffer *config)
{
    return new DockPlugin(base, config);
}

static PluginInfo info =
    {
        I18N_NOOP("System tray"),
        I18N_NOOP("Plugin provides dock main window in system tray"),
        VERSION,
        createDockPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

DockPlugin::DockPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
    , EventReceiver()
    , m_dock(NULL)
    , m_popup(NULL)
{
    m_propertyHub = SIM::PropertyHub::create("dock");
    DockMenu     = registerType();
    CmdTitle	 = registerType();
    CmdToggle    = registerType();
    CmdCustomize = registerType();

    EventMenu(DockMenu, EventMenu::eAdd).process();

    Command cmd;
    cmd->id          = CmdTitle;
    cmd->text        = I18N_NOOP("Sim-IM");
    cmd->icon        = "SIM";
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_TITLE;
    EventCommandCreate(cmd).process();

    cmd->id          = CmdCustomize;
    cmd->text        = I18N_NOOP("Customize menu");
    cmd->icon        = "configure";
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x10000;
    cmd->accel       = QString::null;
    cmd->flags       = COMMAND_DEFAULT;
    EventCommandCreate(cmd).process();

    EventMenuGetDef eMenu(MenuMain);
    eMenu.process();
    CommandsDef *def = eMenu.defs();
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if (s->id == 0)
                continue;
            cmd = *s;
            if (!(cmd->flags & COMMAND_IMPORTANT))
                cmd->menu_grp = 0;
            cmd->bar_id  = 0;
            cmd->menu_id = DockMenu;
            EventCommandCreate(cmd).process();
        }
    }

    cmd->id          = CmdToggle;
    cmd->text        = I18N_NOOP("Toggle main window");
    cmd->icon        = QString::null;
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0;
    cmd->accel		 = "Ctrl+Shift+A";
    cmd->flags		 = COMMAND_CHECK_STATE | COMMAND_GLOBAL_ACCEL | COMMAND_IMPORTANT;
    EventCommandCreate(cmd).process();

    init();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer()));
    timer->start(2000);
}

DockPlugin::~DockPlugin()
{
    EventCommandRemove(CmdToggle).process();
    EventMenu(DockMenu, EventMenu::eRemove).process();
    delete m_dock;
}

void DockPlugin::init()
{
    if (m_dock)
        return;
    m_main = getMainWindow();
    if (!m_main)
        return;
    m_main->installEventFilter(this);
    m_dock = new DockWnd(this, "inactive", I18N_NOOP("Inactive"));
    connect(m_dock, SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
    connect(m_dock, SIGNAL(toggleWin()), this, SLOT(toggleWin()));
    connect(m_dock, SIGNAL(doubleClicked()), this, SLOT(doubleClicked()));
    m_bQuit = false;
    QApplication::syncX();
}

bool DockPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_popup)
    {
        if (e->type() == QEvent::Hide)
        {
            m_popup->removeEventFilter(this);
            m_popup = NULL;
        }
    }else{
        switch (e->type())
        {
        case QEvent::Close:
            if (!m_bQuit)
            {
                QWidget *main = static_cast<QWidget*>(o);
                setValue("ShowMain", false); 
                //setShowMain(false);
                main->hide();
				e->ignore();
                return true;
            }
            break;
        case QEvent::WindowDeactivate:
            m_inactiveTime = QDateTime::currentDateTime();
            break;
        case QEvent::WindowActivate:
            m_inactiveTime = QDateTime();
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(o, e);
}

bool DockPlugin::isMainShow()
{
    QWidget *main = getMainWindow();
    if (main && main->isVisible())
        return true;
    return false;
}

bool DockPlugin::processEvent(Event *e)
{
    switch (e->type())
    {
    case eEventInit:
        init();
        break;
    case eEventQuit:
        if (m_dock){
            delete m_dock;
            m_dock = NULL;
        }
        break;
    case eEventRaiseWindow: 
    {
        EventRaiseWindow *w = static_cast<EventRaiseWindow*>(e);
        if (w->widget() == getMainWindow()){
            if (!m_dock)
                init();
            if (!value("ShowMain").toBool())
                return true;
        }
        break;
    }
    case eEventCommandCreate: 
    {
        EventCommandCreate *ecc = static_cast<EventCommandCreate*>(e);
        CommandDef *def = ecc->cmd();
        if (def->menu_id == MenuMain)
        {
            CommandDef d = *def;
            if (def->flags & COMMAND_IMPORTANT)
            {
                if (d.menu_grp == 0)
                    d.menu_grp = 0x1001;
            }
            else
                d.menu_grp = 0;
            d.menu_id = DockMenu;
            d.bar_id  = 0;
            EventCommandCreate(&d).process();
        }
        break;
    }
    case eEventCheckCommandState: 
    {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *def = ecs->cmd();
        if (def->id == CmdToggle)
        {
            def->flags &= ~COMMAND_CHECKED;
            def->text = isMainShow() ?
                        I18N_NOOP("Hide main window") :
                        I18N_NOOP("Show main window");
            return true;
        }
        break;
    }
    case eEventCommandExec: 
    {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *def = ece->cmd();
        if (def->id == CmdToggle)
        {
            QWidget *main = getMainWindow();
            if(!main)
                return false;
            if (isMainShow())
            {
                setValue("ShowMain", false);
                main->hide();
            }
            else
            {
                m_inactiveTime = QDateTime();
                setValue("ShowMain", true);
                raiseWindow(main, value("Desktop").toUInt());
            }
            return true;
        }
        if (def->id == CmdCustomize){
            EventMenu(DockMenu, EventMenu::eCustomize).process();
            return true;
        }
        if (def->id == CmdQuit)
            m_bQuit = true;
        break;
    }
    case eEventPluginLoadConfig:
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("dock");
        if(!hub.isNull())
            setPropertyHub(hub);
        break;
    }
    default:
        break;
    }
    return false;
}

QByteArray DockPlugin::getConfig()
{
    return QByteArray(); //Fixme
}

QMenu *DockPlugin::createMenu()
{
    Command cmd;
    cmd->popup_id = DockMenu;
    EventMenuGet e(cmd);
    e.process();
    return e.menu();
}

void DockPlugin::showPopup(QPoint p)
{
    if (m_popup)
        return;
    m_popup = createMenu();
    if (m_popup){
        m_popup->installEventFilter(this);
        m_popup->popup(p);
    }
}

void DockPlugin::toggleWin()
{
    if (m_popup)
        return;

    Command cmd;
    cmd->id          = CmdToggle;
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_CHECK_STATE;

    EventCommandExec(cmd).process();
}

void DockPlugin::doubleClicked()
{
    if (m_popup)
        return;

    CorePlugin *core = GET_CorePlugin();
    if ( 0 == core->unread.size() )
        return;

    Command cmd;
    cmd->id          = CmdUnread;
    cmd->menu_id     = DockMenu;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_CHECK_STATE;

    EventCommandExec(cmd).process();
}

QWidget *DockPlugin::getMainWindow()
{
    CorePlugin *core = GET_CorePlugin();
    return core->getMainWindow();
}

QWidget *DockPlugin::createConfigWindow(QWidget *parent)
{
    return new DockCfg(parent, this);
}

void DockPlugin::timer()
{
    if (!isMainShow())  // already hidden
        return;
    if (!value("AutoHide").toBool() || (m_inactiveTime.isNull()))  // no autohide
        return;
    m_main = getMainWindow();
    m_main->installEventFilter(this);

    if (QDateTime::currentDateTime() > m_inactiveTime.addSecs(value("AutoHideInterval").toUInt())){
        if (m_main)
        {
            setValue("ShowMain", false);
            m_main->hide();
        }
    }
}

void DockPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr DockPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant DockPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void DockPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

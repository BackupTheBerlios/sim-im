/***************************************************************************
                          ontop.cpp  -  description
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
#include "mainwin.h"
#include "core.h"

#include "profile.h"
#include "profilemanager.h"

#include <QApplication>
#include <QWidget>
#include <QByteArray>
#include <QEvent>

#ifdef WIN32
#include <windows.h>
#else
#if defined(USE_KDE)
#include "kdeisversion.h"
#include <kwin.h>
#endif
#endif

#include "misc.h"
#include "core_consts.h"

#include "ontop.h"
#include "ontopcfg.h"

using namespace SIM;

Plugin *createOnTopPlugin(unsigned base, bool, Buffer *config)
{
#if defined(WIN32) || defined(USE_KDE)
    return new OnTopPlugin(base, config);
#else
    return NULL;
#endif
}

static PluginInfo info =
    {
        I18N_NOOP("On Top"),
        I18N_NOOP("Plugin provides main window and containers always on top"),
        VERSION,
        createOnTopPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

//static DataDef onTopData[] =
//    {
//        { "OnTop", DATA_BOOL, 1, DATA(1) },
//        { "InTask", DATA_BOOL, 1, 0 },
//        { "ContainerOnTop", DATA_BOOL, 1, 0 },
//        { NULL, DATA_UNKNOWN, 0, 0 }
//    };

OnTopPlugin::OnTopPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
{
    m_propertyHub = SIM::PropertyHub::create("ontop");

    CmdOnTop = registerType();

    Command cmd;
    cmd->id          = CmdOnTop;
    cmd->text        = I18N_NOOP("Always on top");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x7000;
    cmd->flags		= COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

#ifdef WIN32
    m_state = HWND_NOTOPMOST;
#endif

#if defined(WIN32) || defined (USE_KDE)
    qApp->installEventFilter(this);
#endif

    setState();
}

OnTopPlugin::~OnTopPlugin()
{
    EventCommandRemove(CmdOnTop).process();

    setValue("OnTop", false);
    setState();
}

bool OnTopPlugin::processEvent(Event *e)
{
    // FIXME what a mess...
    if (e->type() == eEventInit)
        setState();
    else if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("ontop");
        if(!hub.isNull())
            setPropertyHub(hub);
        if(!value("OnTop").isValid())
            setValue("OnTop", true);
    }
    else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdOnTop){
            setValue("OnTop", !value("OnTop").toBool());
            setState();
            return true;
        }
    } else
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdOnTop){
            getState();
            cmd->flags &= ~COMMAND_CHECKED;
            if (value("OnTop").toBool())
                cmd->flags |= COMMAND_CHECKED;
            return true;
        }
    }
#ifdef WIN32
    if (e->type() == eEventOnTop){
        EventOnTop *eot = static_cast<EventOnTop*>(e);
        QWidget *main = getMainWindow();
        if (main == NULL)
            return false;
        HWND hState = HWND_NOTOPMOST;
        if (value("OnTop").toBool())
            hState = HWND_TOPMOST;
        if (eot->showOnTop())
            hState = HWND_BOTTOM;
        if (m_state != hState)
        {
            SetWindowPos(main->winId(), hState, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            m_state = hState;
        }
    } else
    if (e->type() == eEventInTaskManager){
        EventInTaskManager *eitm = static_cast<EventInTaskManager*>(e);
        QWidget *main = getMainWindow();
        if (main == NULL)
            return false;
        if (IsWindowUnicode(main->winId())){
            if (eitm->showInTaskmanager() && value("InTask").toBool()){
                SetWindowLongW(main->winId(), GWL_EXSTYLE,
                               (GetWindowLongW(main->winId(), GWL_EXSTYLE) | WS_EX_APPWINDOW) & (~WS_EX_TOOLWINDOW));
            }else{
                DWORD exStyle = GetWindowLongW(main->winId(), GWL_EXSTYLE);
                if ((exStyle & WS_EX_TOOLWINDOW) == 0){
                    SetWindowLongW(main->winId(), GWL_EXSTYLE, (exStyle  & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);
                    QPoint p = main->pos();
                    QSize s = main->size();
                    main->resize(s.width() + 1, s.height());
                    main->resize(s.width(), s.height());
                    main->move(p);
                }
            }
        }else{
            if (eitm->showInTaskmanager() && value("InTask").toBool()){
                SetWindowLongA(main->winId(), GWL_EXSTYLE,
                               (GetWindowLongA(main->winId(), GWL_EXSTYLE) | WS_EX_APPWINDOW) & (~WS_EX_TOOLWINDOW));
            }else{
                DWORD exStyle = GetWindowLongA(main->winId(), GWL_EXSTYLE);
                if ((exStyle & WS_EX_TOOLWINDOW) == 0){
                    SetWindowLongA(main->winId(), GWL_EXSTYLE, (exStyle  & ~WS_EX_APPWINDOW) | WS_EX_TOOLWINDOW);
                    QSize s = main->size();
                    main->resize(s.width() + 1, s.height());
                    main->resize(s.width(), s.height());
                }
            }
        }
        return true;
    }
#endif
    return false;
}

QByteArray OnTopPlugin::getConfig()
{
    getState();
    return QByteArray();
}

QWidget *OnTopPlugin::getMainWindow()
{
    CorePlugin *core = GET_CorePlugin();
    return core->getMainWindow();
}

void OnTopPlugin::getState()
{
#ifdef USE_KDE
    QWidget *main = getMainWindow();
    if (main == NULL) return;
#if KDE_IS_VERSION(3,2,0)
    setOnTop(KWin::windowInfo(main->winId()).state() & NET::StaysOnTop);
#else
    setOnTop(KWin::info(main->winId()).state & NET::StaysOnTop);
#endif
#endif
}

void OnTopPlugin::setState()
{
    QWidget *main = getMainWindow();
    if (main){
#ifdef WIN32
        EventOnTop(false).process();
        EventInTaskManager(value("InTask").toBool()).process();
#else
#ifdef USE_KDE
        if (getOnTop()){
            KWin::setState(main->winId(), NET::StaysOnTop);
        }else{
            KWin::clearState(main->winId(), NET::StaysOnTop);
        }
        if (getInTask()){
            KWin::clearState(main->winId(), NET::SkipTaskbar);
        }else{
            KWin::setState(main->winId(), NET::SkipTaskbar);
        }
#endif
#endif
    }
    QWidgetList list = QApplication::topLevelWidgets();
    QWidget *w;
    foreach(w,list)
	{
			if (w->inherits("Container")){
#ifdef WIN32
				HWND hState = HWND_NOTOPMOST;
                                if (value("ContainerOnTop").toBool()) hState = HWND_TOPMOST;
				SetWindowPos(w->winId(), hState, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#else
#ifdef USE_KDE
				if (getContainerOnTop())
				{
					KWin::setState(w->winId(), NET::StaysOnTop);
				}
				else
				{
					KWin::clearState(w->winId(), NET::StaysOnTop);
				}
#endif
#endif
			}
		}
	}

#if defined(USE_KDE) || defined(WIN32)
QWidget *OnTopPlugin::createConfigWindow(QWidget *parent)
{
    return new OnTopCfg(parent, this);
}
#endif

bool OnTopPlugin::eventFilter(QObject *o, QEvent *e)
{
#ifdef WIN32
    if ((e->type() == QEvent::WindowActivate) &&
            (value("OnTop").toBool() || value("ContainerOnTop").toBool() ) &&
            o->inherits("QDialog")){
        QWidget *w = static_cast<QWidget*>(o);
        SetWindowPos(w->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    if ((e->type() == QEvent::WindowDeactivate) &&
            (value("OnTop").toBool() || value("ContainerOnTop").toBool()) &&
            o->inherits("QDialog")){
        QWidget *w = static_cast<QWidget*>(o);
        SetWindowPos(w->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
    if ((e->type() == QEvent::Show) &&
            value("ContainerOnTop").toBool() &&
            o->inherits("Container")){
        QWidget *w = static_cast<QWidget*>(o);
        SetWindowPos(w->winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    }
#endif
#ifdef USE_KDE
    if ((e->type() == QEvent::WindowActivate) &&
            (value("OnTop").toBool() || value("ContainerOnTop").toBool()) &&
            o->inherits("QDialog")){
        QWidget *w = static_cast<QWidget*>(o);
        KWin::setState(w->winId(), NET::StaysOnTop);
    }
    if ((e->type() == QEvent::WindowDeactivate) &&
            (value("OnTop").toBool() || value("ContainerOnTop").toBool()) &&
            o->inherits("QDialog")){
        QWidget *w = static_cast<QWidget*>(o);
        KWin::clearState(w->winId(), NET::StaysOnTop);
    }
    if ((e->type() == QEvent::Show) &&
            value("ContainerOnTop").toBool() &&
            o->inherits("Container")){
        QWidget *w = static_cast<QWidget*>(o);
        KWin::setState(w->winId(), NET::StaysOnTop);
    }
#endif
    return QObject::eventFilter(o, e);
}

void OnTopPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr OnTopPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant OnTopPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void OnTopPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}


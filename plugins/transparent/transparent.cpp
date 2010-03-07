/***************************************************************************
                          transparent.cpp  -  description
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

#include <QApplication>
#include <QWidgetList>
#include <QTimer>

#include "log.h"
#include "misc.h"
#include "core.h"

#include "transparent.h"
#include "mainwin.h"
#include "transparentcfg.h"
#include "../floaty/floatywnd.h" //Handle Floatings
#include "profile.h"
#include "profilemanager.h"


#define SHOW_TIMEOUT	300
#define HIDE_TIMEOUT	1000

using namespace SIM;

Plugin *createTransparentPlugin(unsigned base, bool, Buffer *config)
{
    return new TransparentPlugin(base, config);
}

static PluginInfo info =
    {
        I18N_NOOP("Transparent"),
#ifdef WIN32
        I18N_NOOP("Plugin provides windows transparency\n"
                  "This plugin works only on Windows 2000 or Windows XP")
#else
        I18N_NOOP("Plugin provides windows transparency")
#endif
        ,
        VERSION,
        createTransparentPlugin,
#ifdef WIN32
        PLUGIN_DEFAULT
#else
        PLUGIN_NOLOAD_DEFAULT
#endif
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

TransparentPlugin::TransparentPlugin(unsigned base, Buffer *config)
    : QObject(), Plugin(base)
    , timer (NULL) //Fixme? look below
    , m_bHaveMouse (false)
    , m_bActive    (false)
{
    m_propertyHub = SIM::PropertyHub::create("transparent");
    if (value("Transparency").toUInt() >100)
        setValue("Transparency", 100);

    QTimer *timer = new QTimer(this); //Fixme? look above
    connect(timer, SIGNAL(timeout()), this, SLOT(tickMouse()));
    timer->start(1000);
    setState();
}

TransparentPlugin::~TransparentPlugin()
{
    delete timer;

    // reset opacity for all toplevel widgets
    QWidgetList list = QApplication::topLevelWidgets();
    foreach(QWidget *w,list) 
    {
        w->setWindowOpacity(1.0);
    }
}

QByteArray TransparentPlugin::getConfig()
{
    return QByteArray();
}

QWidget *TransparentPlugin::getMainWindow()
{
    CorePlugin *core = GET_CorePlugin();
    return core->getMainWindow();
}

QWidget *TransparentPlugin::createConfigWindow(QWidget *parent)
{
    return new TransparentCfg(parent, this);
}

void TransparentPlugin::tickMouse()
{
    QPoint p = QCursor::pos();
    bool bMouse = false;
    QWidget *main = getMainWindow();
    if (main && main->isVisible()){
        if (main->frameGeometry().contains(p))
            bMouse = true;
    }

    //Handle Floatings//
    QWidgetList list = QApplication::topLevelWidgets();
    foreach (QWidget *w,list) {
        if (bMouse)
            break;
        if (FloatyWnd *flt = dynamic_cast<FloatyWnd *>(w))
            bMouse = flt->frameGeometry().contains(p);
    }
    //Handle Floatings//

    if (bMouse != m_bHaveMouse){
        m_bHaveMouse = bMouse;
        setState();
    }
}

bool TransparentPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (value("IfInactive").toBool()){
        switch (e->type()){
        case QEvent::WindowActivate:
            m_bActive = true;
            setState();
            break;
        case QEvent::WindowDeactivate:
            m_bActive = false;
            setState();
            break;
        case QEvent::Show:{
                QWidget *main = getMainWindow();
                if (main){
                    setState();
                }
                break;
            }
        default:
            break;
        }
    }
    return QObject::eventFilter(o, e);
}

void TransparentPlugin::setState()
{
    QWidget *main = getMainWindow();
    QWidgetList list = QApplication::topLevelWidgets();
        
    if (main == NULL)
        return;
    if (timer == NULL) {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(tick()));
        main->installEventFilter(this);
        main->setMouseTracking(true);
        m_bActive = main->isActiveWindow();
        m_bState  = !m_bActive;
    }
    bool bNewState = m_bActive || m_bHaveMouse;
    if (bNewState == m_bState){
        qreal transparency = (100 - value("Transparency").toUInt()) / 100.;
        if(value("IfMainWindow").toBool())
            main->setWindowOpacity(transparency);
        else
            main->setWindowOpacity(1.0);

        QWidgetList list = QApplication::topLevelWidgets();
        foreach (QWidget *w,list) {
            if (FloatyWnd *refwnd = dynamic_cast<FloatyWnd *>(w)){
                refwnd->installEventFilter(this);
                if (value("IfFloatings").toBool())
                    refwnd->setWindowOpacity(transparency);
                else
                    refwnd->setWindowOpacity(1.0);
            }
        }
        return;
    }
    m_bState = bNewState;
    startTime = QTime::currentTime();
    timer->start(10);
}

void TransparentPlugin::tick()
{
    QWidget *main = getMainWindow();
    if (main == NULL){
        timer->stop();
        return;
    }
    qreal timeout = m_bActive ? SHOW_TIMEOUT : HIDE_TIMEOUT;
    qreal difftime = startTime.msecsTo( QTime::currentTime() );
    if (difftime >= timeout){
        difftime = timeout;
        timer->stop();
    }
    if (m_bState)
        difftime = timeout - difftime;

    qreal transparency = (100 - value("Transparency").toUInt()) / 100.;
    qreal diff_to_opaque = 1. - transparency;
    transparency = transparency + diff_to_opaque * (1 - difftime / timeout);

    //log(L_DEBUG, "transparency: %f, diff_to_opaque %f, time %d, timeout %d",
    //             transparency, diff_to_opaque, time, timeout);
    if (value("IfMainWindow").toBool())
        main->setWindowOpacity(transparency);
    else
        main->setWindowOpacity(1.0);

    //Handle Floatings
    QWidgetList list = QApplication::topLevelWidgets();
    foreach (QWidget *w,list) {
        if (FloatyWnd *refwnd = qobject_cast<FloatyWnd *>(w)){
            refwnd->installEventFilter(this);
            if (value("IfFloatings").toBool())
                refwnd->setWindowOpacity(transparency);
            else
                refwnd->setWindowOpacity(1.0);
        }
    }
    //Handle Floatings//
}

bool TransparentPlugin::processEvent(Event *e)
{
    if (e->type() == eEventInit) {
        setState();
    }
    else if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("trnsparent");
        if(!hub.isNull())
            setPropertyHub(hub);
    }
    return false;
}


void TransparentPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr TransparentPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant TransparentPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void TransparentPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

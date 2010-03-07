/***************************************************************************
                          autoaway.cpp  -  description
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
 ***************************************************************************

Detect idle time for MAC:
Copyright (C) 2003  Tarkvara Design Inc.

*/

// This is required to use Xlibint (which isn't very clean itself)
#define QT_CLEAN_NAMESPACE

#include "simapi.h"

#include "log.h"
#include "autoaway.h"
#include "autoawaycfg.h"
#include "core.h"

#include "contacts/client.h"

#ifdef WIN32
#define _WIN32_WINNT 0x0500
#include <windows.h>

#include <QLibrary>

#include "idleui.h"

static BOOL (WINAPI * _GetLastInputInfo)(PLASTINPUTINFO);

#elif defined(HAVE_CARBON_CARBON_H) && !defined(HAVE_X)
#include <Carbon/Carbon.h>
#elif defined(__OS2__)
#define INCL_WIN
#include <os2.h>
#include "sysglit.h"
#else
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlibint.h>
#include <X11/extensions/scrnsaver.h>
#endif

#include <time.h>
#include <QApplication>
#include <QTimer>
#include <QWidget>

using namespace SIM;

const unsigned AUTOAWAY_TIME	= 10000;

Plugin *createAutoAwayPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new AutoAwayPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("AutoAway"),
        I18N_NOOP("Plugin provides set away and N/A status after some idle time"),
        VERSION,
        createAutoAwayPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

#if defined(HAVE_CARBON_CARBON_H) && !defined(HAVE_X)

static unsigned mSecondsIdle = 0;
static EventLoopTimerRef mTimerRef;

static OSStatus LoadFrameworkBundle(CFStringRef framework, CFBundleRef *bundlePtr) {
    OSStatus  err;
    FSRef   frameworksFolderRef;
    CFURLRef baseURL;
    CFURLRef bundleURL;

    if ( bundlePtr == nil ) return( -1 );

    *bundlePtr = nil;

    baseURL = nil;
    bundleURL = nil;

    err = FSFindFolder(kOnAppropriateDisk, kFrameworksFolderType, true, &frameworksFolderRef);
    if (err == noErr) {
        baseURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &frameworksFolderRef);
        if (baseURL == nil) {
            err = coreFoundationUnknownErr;
        }
    }
    if (err == noErr) {
        bundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, baseURL, framework, false);
        if (bundleURL == nil) {
            err = coreFoundationUnknownErr;
        }
    }
    if (err == noErr) {
        *bundlePtr = CFBundleCreate(kCFAllocatorSystemDefault, bundleURL);
        if (*bundlePtr == nil) {
            err = coreFoundationUnknownErr;
        }
    }
    if (err == noErr) {
        if ( ! CFBundleLoadExecutable( *bundlePtr ) ) {
            err = coreFoundationUnknownErr;
        }
    }

    // Clean up.
    if (err != noErr && *bundlePtr != nil) {
        CFRelease(*bundlePtr);
        *bundlePtr = nil;
    }
    if (bundleURL != nil) {
        CFRelease(bundleURL);
    }
    if (baseURL != nil) {
        CFRelease(baseURL);
    }
    return err;
}

pascal void IdleTimerAction(EventLoopTimerRef, EventLoopIdleTimerMessage inState, void* inUserData)
{
    switch (inState) {
    case kEventLoopIdleTimerStarted:
    case kEventLoopIdleTimerStopped:
        // Get invoked with this constant at the start of the idle period,
        // or whenever user activity cancels the idle.
        mSecondsIdle = 0;
        break;
    case kEventLoopIdleTimerIdling:
        // Called every time the timer fires (i.e. every second).
        mSecondsIdle++;
        break;
    }
}

typedef OSStatus (*InstallEventLoopIdleTimerPtr)(EventLoopRef inEventLoop,
        EventTimerInterval   inFireDelay,
        EventTimerInterval   inInterval,
        EventLoopIdleTimerUPP    inTimerProc,
        void *               inTimerData,
        EventLoopTimerRef *  outTimer);

#endif

static DataDef autoAwayData[] =
    {
        { "AwayTime",	      DATA_ULONG,   1, DATA(3) },
        { "EnableAway",       DATA_BOOL,    1, DATA(1) },
        { "NATime", 	      DATA_ULONG,   1, DATA(10)},
        { "EnableNA", 	      DATA_BOOL,    1, DATA(1) },
        { "OffTime", 	      DATA_ULONG,   1, DATA(10)},
        { "EnableOff", 	      DATA_BOOL,    1, 0       },
        { "DisableAlert",     DATA_BOOL,    1, DATA(1) },
        { "RealManualStatus", DATA_ULONG,   1, DATA(STATUS_UNKNOWN) },
        { NULL,		      DATA_UNKNOWN, 0, 0 }
    };

AutoAwayPlugin::AutoAwayPlugin(unsigned base, Buffer *config)
        : Plugin(base), EventReceiver(HighPriority)
{
    load_data(autoAwayData, &data, config);
#ifdef WIN32
    (DWORD&)_GetLastInputInfo = (DWORD)QLibrary::resolve("user32.dll", "GetLastInputInfo");
#elif defined(HAVE_CARBON_CARBON_H) && !defined(HAVE_X)
/* ToDo:
    CFBundleRef carbonBundle;
    if (LoadFrameworkBundle( CFSTR("Carbon.framework"), &carbonBundle ) == noErr) {
        InstallEventLoopIdleTimerPtr myInstallEventLoopIdleTimer = (InstallEventLoopIdleTimerPtr)CFBundleGetFunctionPointerForName(carbonBundle, CFSTR("InstallEventLoopIdleTimer"));
        if (myInstallEventLoopIdleTimer){
            EventLoopIdleTimerUPP timerUPP = NewEventLoopIdleTimerUPP(Private::IdleTimerAction);
            (*myInstallEventLoopIdleTimer)(GetMainEventLoop(), kEventDurationSecond, kEventDurationSecond, timerUPP, 0, &mTimerRef);
        }
    }
*/
#endif
    bAway = false;
    bNA   = false;
    bOff  = false;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    m_timer->start(AUTOAWAY_TIME);
}

AutoAwayPlugin::~AutoAwayPlugin()
{
#ifdef WIN32
	// nothing to do
#elif defined(HAVE_CARBON_CARBON_H) && !defined(HAVE_X)
	RemoveEventLoopTimer(mTimerRef);
#elif defined(__OS2__)
	// ---
#else
        // We load static Xss in our autoaway.so's process space, but the bastard
        // registers for shutdown in the XDisplay variable, so after autoaway.so
        // unloads, its code will still be called (as part of the XCloseDisplay).
        // As Xss offers no function to unregister itself, we'll have to be a little
        // messy here:
        Display* dpy = QX11Info::display();
        LockDisplay(dpy);
        // Original code from Xlib's ClDisplay.c
        _XExtension *ext, *prev_ext = NULL;
        for (ext = dpy->ext_procs; ext; prev_ext = ext, ext = ext->next)
        {
                if (ext->name && (strcmp(ext->name, ScreenSaverName) == 0))
                {
                        if (ext->close_display)
                                (*ext->close_display)(dpy, &ext->codes);
                        if (prev_ext)
                                prev_ext->next = ext->next;
                        else
                                dpy->ext_procs = ext->next;
                        Xfree((char*)ext);
                        break;
                }
        }
        UnlockDisplay(dpy);
#endif
	free_data(autoAwayData, &data);
}

QByteArray AutoAwayPlugin::getConfig()
{
    return save_data(autoAwayData, &data);
}

QWidget *AutoAwayPlugin::createConfigWindow(QWidget *parent)
{
    return new AutoAwayConfig(parent, this);
}

void AutoAwayPlugin::timeout()
{
    CorePlugin *core = GET_CorePlugin();
    unsigned long newStatus = core->getManualStatus();
    unsigned long oldStatus =getRealManualStatus();
    unsigned idle_time = getIdleTime() / 60;
    if (oldStatus != STATUS_UNKNOWN && !bAway && !bNA && !bOff){
      // If fake ManualStatus were saved in config by chace, we should replace it by real value...
      newStatus = oldStatus;
      oldStatus = STATUS_UNKNOWN;
    }
    if ((bAway && (idle_time < getAwayTime())) ||
            (bNA && (idle_time < getNATime())) ||
            (bOff && (idle_time < getOffTime()))){
        bAway = false;
        bNA   = false;
        bOff  = false;
        newStatus = oldStatus;
        oldStatus = STATUS_UNKNOWN;
    }else if (!bAway && !bNA && !bOff && getEnableAway() && (idle_time >= getAwayTime())){
        unsigned long status = core->getManualStatus();
        if ((status == STATUS_AWAY) || (status == STATUS_NA) || (status == STATUS_OFFLINE))
            return;
        oldStatus = status;
        newStatus = STATUS_AWAY;
        bAway = true;
    }else  if (!bNA && !bOff && getEnableNA() && (idle_time >= getNATime())){
        unsigned long status = core->getManualStatus();
        if ((status == STATUS_NA) || (status == STATUS_OFFLINE))
            return;
        if (!bAway)
            oldStatus = status;
        bNA = true;
        newStatus = STATUS_NA;
    }else if (!bOff && getEnableOff() && (idle_time >= getOffTime())){
        unsigned long status = core->getManualStatus();
        if (status == STATUS_OFFLINE)
            return;
        if (!bNA)
            oldStatus = status;
        bOff = true;
        newStatus = STATUS_OFFLINE;
    }
    if (newStatus == core->getManualStatus())
        return;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (!client->getCommonStatus())
            continue;
        client->setStatus(newStatus, true);
    }
    if (core->getManualStatus() == newStatus)
        return;
    core->setValue("StatusTime", (unsigned int)time(NULL)); //data.StatusTime.asULong() = time(NULL);
    //core->data.ManualStatus.asULong() = newStatus;
    core->setValue("ManualStatus", (unsigned int)newStatus);
    setRealManualStatus(oldStatus);
    EventClientStatus().process();
}

bool AutoAwayPlugin::processEvent(Event *e)
{
    switch (e->type()) {
    case eEventPlaySound: {
        if (getDisableAlert() && (bAway || bNA || bOff))
            return true;
        break;
    }
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eOnline)
            break;
        unsigned long commonStatus = STATUS_UNKNOWN;
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (!client->getCommonStatus())
                continue;
            commonStatus = client->getManualStatus();
            break;
        }
        if ((commonStatus == STATUS_ONLINE) || (commonStatus == STATUS_OFFLINE))
            return false;
        if (getDisableAlert() && (bAway || bNA || bOff))
            return true;
        break;
    }
    default:
        break;
    }
    return false;
}

unsigned AutoAwayPlugin::getIdleTime()
{
#ifdef WIN32
    if (_GetLastInputInfo){
        LASTINPUTINFO lii;
        ZeroMemory(&lii,sizeof(lii));
        lii.cbSize=sizeof(lii);
        _GetLastInputInfo(&lii);
        return (GetTickCount()-lii.dwTime) / 1000;
    }
    return IdleUIGetLastInputTime();
#elif defined(HAVE_CARBON_CARBON_H) && !defined(HAVE_X)
    return mSecondsIdle;
#elif defined(__OS2__)
    ULONG lastInp = WinGetLastInputTime();
    if ( lastInp == 0 ) {
        return 0;
    }
    return (WinGetCurrentTime(WinQueryAnchorBlock(HWND_DESKTOP)) - lastInp) / 1000;
#else
    static XScreenSaverInfo *mit_info = NULL;
    if (mit_info == NULL) {
        int event_base, error_base;
		/*
        if(XScreenSaverQueryExtension(QX11Info::display(), &event_base, &error_base)) {
            mit_info = XScreenSaverAllocInfo ();
        }
		*/
    }
    if (mit_info == NULL){
        log(L_WARN, "No XScreenSaver extension found on current XServer, disabling auto-away.");
        m_timer->stop();
        return 0;
    }
    if (!XScreenSaverQueryInfo(QX11Info::display(), QX11Info::appRootWindow(), mit_info)) {
        log(L_WARN, "XScreenSaverQueryInfo failed, disabling auto-away.");
        m_timer->stop();
        return 0;
    }
    return (mit_info->idle / 1000);
#endif
}


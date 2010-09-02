/***************************************************************************
                          windock.cpp  -  description
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

#include <windows.h>
#include <windowsx.h>

#include <QApplication>
#include <QWidgetlist>
#include <QMessageBox>
#include <QTimer>

#include "misc.h"

#include "windock.h"
#include "ontop.h"
#include "core.h"
#include "core_consts.h"
#include "mainwin.h"

using namespace std;
using namespace SIM;

const unsigned short ABE_FLOAT   = USHRT_MAX;

static WinDockPlugin *dock = NULL;

Plugin *createWinDockPlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new WinDockPlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Dock"),
        I18N_NOOP("Plugin provides dock main window to left or right side of screen"),
        VERSION,
        createWinDockPlugin,
        0
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static QWidget *pMain   = NULL;
static UINT WM_APPBAR   = 0;
static WNDPROC oldProc	= NULL;
static bool bOldVisible      = true;
static bool bAutoHideVisible = true;
static bool bFullScreen = false;
static bool bMoving = false;
static bool bSizing = false;

UINT appBarMessage(DWORD dwMessage, UINT uEdge=ABE_FLOAT, LPARAM lParam=0, QRect *rc=NULL)
{


    APPBARDATA abd;
    abd.cbSize           = sizeof(abd);
    abd.hWnd             = pMain->winId();
    abd.uCallbackMessage = WM_APPBAR;
    abd.uEdge            = uEdge;
    if (rc)
    {
        abd.rc.left = rc->left();
        abd.rc.top = rc->top();
        abd.rc.right = rc->right();
        abd.rc.bottom = rc->bottom();
    }
    else
    {
        abd.rc.left = 0;
        abd.rc.top = 0;
        abd.rc.right = 0;
        abd.rc.bottom = 0;
    }
    abd.lParam           = lParam;
      

    UINT uRetVal         = SHAppBarMessage(dwMessage, &abd);

    //if (!rc)
    //return 0;
    //if (rc && rc->left()>2 && rc->left()<200)
    log(L_DEBUG, QString("dwMessage=%1").arg(dwMessage));
    //if (dwMessage==3)
    //    __asm int 3;  
    
           

    if (rc != NULL)
    {
        rc->setCoords(abd.rc.left, abd.rc.top, abd.rc.right, abd.rc.bottom);
        log(L_DEBUG, QString("rc-coords_5 (left, top, width, height): %1 %2 %3 %4").arg(rc->left()).arg(rc->top()).arg(rc->width()).arg(rc->height()));
    }
    return uRetVal;
}

static unsigned short getEdge(RECT *rcWnd = NULL)
{
    RECT rc;
    if (!rcWnd)
    {
        GetWindowRect(pMain->winId(), &rc);
        rcWnd = &rc;
    }
    if (rcWnd->left <= 0) 
        return ABE_LEFT;
    if (rcWnd->right >= GetSystemMetrics(SM_CXSCREEN)) 
        return ABE_RIGHT;
    return ABE_FLOAT;
}

static void getBarRect(UINT state, QRect &rc, RECT *rcWnd = NULL)
{
    RECT rcWork;
    SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0);
    rc.setCoords(0, rcWork.top, GetSystemMetrics(SM_CXSCREEN), rcWork.bottom);
    appBarMessage(ABM_QUERYPOS, state, FALSE, &rc);
    int w;
    if (rcWnd)
        w = rcWnd->right - rcWnd->left;
    else
    {
        GetWindowRect(pMain->winId(), &rcWork);
#ifdef WIN32
        w = rcWork.right - rcWork.left-1;
#else
        w = rcWork.right - rcWork.left;
#endif
    }
    switch (state)
    {
    case ABE_LEFT:
        rc.setRight(rc.left() + w);
        break;
    case ABE_RIGHT:
        rc.setLeft(rc.right() - w);
        break;
    }
}

const int SLIDE_INTERVAL = 800;

void slideWindow (const QRect &rcEnd, bool bAnimate)
{
    BOOL fFullDragOn;

    // Only slide the window if the user has FullDrag turned on
    SystemParametersInfoA(SPI_GETDRAGFULLWINDOWS, 0, &fFullDragOn, 0);

    // Get the current window position
    RECT rcWnd;
    GetWindowRect(pMain->winId(), &rcWnd);
    QRect rcStart;
    rcStart.setCoords(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);

    if (bAnimate && fFullDragOn && (rcStart != rcEnd)) 
    {
        static_cast<MainWindow*>(pMain)->m_bNoResize = true;
        // Get our starting and ending time.
        DWORD dwTimeStart = GetTickCount();
        DWORD dwTimeEnd = dwTimeStart + SLIDE_INTERVAL;
        DWORD dwTime;

        while ((dwTime = ::GetTickCount()) < dwTimeEnd) 
        {
            int delta = (int)(dwTime - dwTimeStart);
            QRect rc = rcStart;
            rc.setLeft(rcStart.left() +
                       (rcEnd.left() - rcEnd.left()) * delta / SLIDE_INTERVAL);
            rc.setTop(rcStart.top() +
                      (rcEnd.top() - rcEnd.top()) * delta / SLIDE_INTERVAL);
            rc.setWidth(rcStart.width() +
                        (rcEnd.width() - rcEnd.width()) * delta / SLIDE_INTERVAL);
            rc.setHeight(rcStart.height() +
                         (rcEnd.height() - rcEnd.height()) * delta / SLIDE_INTERVAL);
            SetWindowPos(pMain->winId(), NULL,
                         rc.left(), rc.top(), rc.width(), rc.height(),
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
            UpdateWindow(pMain->winId());
        }
        static_cast<MainWindow*>(pMain)->m_bNoResize = false;
    }
    EventInTaskManager((dock->getState() == ABE_FLOAT)).process();
    SetWindowPos(pMain->winId(), NULL,
                 rcEnd.left(), rcEnd.top(), rcEnd.width(), rcEnd.height(),
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
    UpdateWindow(pMain->winId());
}

void setBarState(bool bAnimate = false)
{
    
    if ((dock->getState() == ABE_FLOAT) || !pMain->isVisible())
    {
        appBarMessage(ABM_SETPOS, ABE_FLOAT, FALSE);
        return;
    }

    if (dock->getAutoHide() && !appBarMessage(ABM_SETAUTOHIDEBAR, dock->getState(), TRUE, NULL))
    {
        dock->setAutoHide(false);
        QMessageBox::warning(NULL, i18n("Error"),
                             i18n("There is already an auto hidden window on this edge.\nOnly one auto hidden window is allowed on each edge."),
                             QMessageBox::Ok, 0);
    }
    QRect rc;
    getBarRect(dock->getState(), rc);
    log(L_DEBUG, QString("rc-coords_1 (left, top, width, height): %1 %2 %3 %4").arg(rc.left()).arg(rc.top()).arg(rc.width()).arg(rc.height()));
    if (dock->getAutoHide())
    {
        QRect rcAutoHide = rc;
        int w = 4 * GetSystemMetrics(SM_CXBORDER);
        if (dock->getState() == ABE_LEFT)
            rcAutoHide.setRight(rcAutoHide.left() + w);
        else
            rcAutoHide.setLeft(rcAutoHide.right() - w);

        appBarMessage(ABM_SETPOS, dock->getState(), FALSE, &rcAutoHide);
        if (!bAutoHideVisible)
        {
            if (bOldVisible)
                dock->setWidth(rc.width() - GetSystemMetrics(SM_CXBORDER) * 2);
            MINMAXINFO mmInfo;
            rc = rcAutoHide;
            SendMessageA(pMain->winId(), WM_GETMINMAXINFO, 0, (LPARAM)&mmInfo);
            if (dock->getState() == ABE_LEFT)
                rc.setLeft(rc.right() - mmInfo.ptMinTrackSize.x);
            else
                rc.setRight(rc.left() + mmInfo.ptMinTrackSize.x);
            log(L_DEBUG, QString("rc-coords_2 (left, top, width, height): %1 %2 %3 %4").arg(rc.left()).arg(rc.top()).arg(rc.width()).arg(rc.height()));
        }
        else if (dock->getState() == ABE_LEFT)
            rc.setRight(rc.left() + dock->getWidth());
        else
            rc.setLeft(rc.right() - dock->getWidth());
        log(L_DEBUG, QString("rc-coords_3 (left, top, width, height): %1 %2 %3 %4").arg(rc.left()).arg(rc.top()).arg(rc.width()).arg(rc.height()));
        bOldVisible = bAutoHideVisible;
        return;
    }
    else
    {
        log(L_DEBUG, QString("rc-coords_4 (left, top, width, height): %1 %2 %3 %4").arg(rc.left()).arg(rc.top()).arg(rc.width()).arg(rc.height()));
        appBarMessage(ABM_SETPOS, dock->getState(), FALSE, &rc); //this causes the final docking and the jump to wrong offset
        return;
    }


    slideWindow(rc, bAnimate);
    
    if (pMain->isVisible())
    {
        EventOnTop(bFullScreen).process();
        if (!bFullScreen && (qApp->activeWindow() == pMain))
            appBarMessage(ABM_ACTIVATE);
    }
}

LRESULT CALLBACK dockWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT res;
    if (msg == WM_APPBAR)
    {
        switch (wParam)
        {
        case ABN_FULLSCREENAPP:
            if ((lParam != 0) == bFullScreen)
                break;
            bFullScreen = (lParam != 0);
            setBarState();
            break;
        case ABN_POSCHANGED:
            if (dock->getState() != ABE_FLOAT)
                setBarState();
            break;
        }
    }
    if(!oldProc)
        return DefWindowProc(hWnd, msg, wParam, lParam);
    unsigned type;
    RECT  *prc;
    RECT  rcWnd;
    QRect rc;
    unsigned oldState;
    switch (msg)
    {
    case WM_DESTROY:
        res = oldProc(hWnd, msg, wParam, lParam);
        appBarMessage(ABM_REMOVE);
        WNDPROC p;
        p = (WNDPROC)SetWindowLongW(hWnd, GWL_WNDPROC, (LONG)oldProc);
        if (p == 0)
            p = (WNDPROC)SetWindowLongA(hWnd, GWL_WNDPROC, (LONG)oldProc);
        oldProc = NULL;
        return res;
    case WM_SHOWWINDOW:
        res = oldProc(hWnd, msg, wParam, lParam);
        if (dock->getState() != ABE_FLOAT)
            QTimer::singleShot(0, dock, SLOT(slotSetState()));
        return res;
    case WM_ACTIVATE:
        if (dock->getState() != ABE_FLOAT)
        {
            if ((wParam == WA_INACTIVE) && dock->getAutoHide() && bAutoHideVisible)
            {
                bAutoHideVisible = false;
                setBarState();
                dock->enableAutoHide(false);
            }
            appBarMessage(ABM_ACTIVATE);
        }
        break;
    case WM_NCMOUSEMOVE:
        if ((dock->getState() != ABE_FLOAT) && dock->getAutoHide() && !bAutoHideVisible)
        {
            bAutoHideVisible = true;
            setBarState(true);
            dock->enableAutoHide(true);
        }
        break;
    case WM_ENTERSIZEMOVE:
        bMoving = true;
        bSizing = true;
        if (dock->getState() == ABE_FLOAT)
        {
            bSizing = false;
            GetWindowRect(hWnd, &rcWnd);
            dock->setHeight(rcWnd.bottom - rcWnd.top);
        }
        return DefWindowProc(hWnd, msg, wParam, lParam);
    case WM_EXITSIZEMOVE:
        bMoving = false;
        oldState = dock->getState();
        dock->setState(getEdge());
        GetWindowRect(hWnd, &rcWnd);
        if ((dock->getState() == ABE_FLOAT) && (oldState != ABE_FLOAT))
        {
            rcWnd.bottom = rcWnd.top + dock->getHeight();
            SetWindowPos(pMain->winId(), NULL,
                         rcWnd.left, rcWnd.top, rcWnd.right - rcWnd.left, rcWnd.bottom - rcWnd.top,
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME);
        }
        dock->setWidth(rcWnd.right - rcWnd.left);
        setBarState(true);
        
        return DefWindowProc(hWnd, msg, wParam, lParam);
    case WM_MOVING:
    case WM_SIZING:
        if (!bMoving) break;
        prc = (RECT*)lParam;
        type = getEdge(prc);
        if (type == ABE_FLOAT)
        {
            if (bSizing)
            {
                prc->bottom = prc->top + dock->getHeight();
                bSizing = false;
            }
        }
        else
        {
            getBarRect(type, rc, prc);
            prc->left = rc.left();
            prc->top = rc.top();
            prc->right = rc.right();
            prc->bottom = rc.bottom();
            bSizing = true;
        }
        return 1;
    case WM_WINDOWPOSCHANGED:
        res = oldProc(hWnd, msg, wParam, lParam);
        if (dock->getState() != ABE_FLOAT)
            appBarMessage(ABM_WINDOWPOSCHANGED);
        return res;
    }
    log(L_DEBUG,"setBarState before calling last oldProc...");
    return oldProc(hWnd, msg, wParam, lParam);
 }

static DataDef winDockData[] =
    {
        { "AutoHide", DATA_BOOL, 1, 0 },
        { "State", DATA_ULONG, 1, DATA(-1) },
        { "Height", DATA_ULONG, 1, 0 },
        { "Width", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

WinDockPlugin::WinDockPlugin(unsigned base, Buffer *config)
        : Plugin(base), EventReceiver(DefaultPriority - 1)
{
    dock = this;

    load_data(winDockData, &data, config);

    CmdAutoHide = registerType();

    Command cmd;
    cmd->id          = CmdAutoHide;
    cmd->text        = I18N_NOOP("AutoHide");
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x7001;
    cmd->flags		= COMMAND_CHECK_STATE;

    m_bInit = false;

    m_autoHide = new QTimer(this);
    connect(m_autoHide, SIGNAL(timeout()), this, SLOT(slotAutoHide()));

    EventCommandCreate(cmd).process();

    WM_APPBAR = RegisterWindowMessageA("AppBarNotify");
    //getEventHub()->getEvent("init")->connectTo(this, SLOT(init())); //uncomment using new event-system
    init();
}

WinDockPlugin::~WinDockPlugin()
{
    uninit();
    EventCommandRemove(CmdAutoHide).process();
    free_data(winDockData, &data);
}

void WinDockPlugin::uninit()
{
    QWidget *main = getMainWindow();
    if (main && oldProc)
    {
        appBarMessage(ABM_REMOVE);
        if (IsWindowUnicode(pMain->winId()))
            SetWindowLongW(main->winId(), GWL_WNDPROC, (LONG)oldProc);
        else
            SetWindowLongA(main->winId(), GWL_WNDPROC, (LONG)oldProc);
        oldProc = NULL;
    }
}


bool WinDockPlugin::processEvent(Event *e)
{
    if (e->type() == eEventCommandExec)
    {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdAutoHide)
        {
            dock->setAutoHide((cmd->flags & COMMAND_CHECKED) != 0);
            bAutoHideVisible = true;
            setBarState();
            enableAutoHide(getAutoHide());
            return true;
        }
    } 
    else if (e->type() == eEventCheckCommandState)
    {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdAutoHide && dock->getState() != ABE_FLOAT)
        {
            cmd->flags &= ~COMMAND_CHECKED; //Strokes the bit
            if (dock->getAutoHide())
                cmd->flags |= COMMAND_CHECKED; //Sets the bit
            return true;
        }
    } 
    else if ((e->type() == eEventInit) && !m_bInit)
        init();
    if (e->type() == eEventInTaskManager)
    {
        EventInTaskManager *eitm = static_cast<EventInTaskManager*>(e);
        if (dock->getState() != ABE_FLOAT && eitm->showInTaskmanager())
        {
            EventInTaskManager(false).process();
            return true;
        }
    }
    return false;
}

void WinDockPlugin::init()
{
    if (m_bInit)
        return;
    pMain = getMainWindow();
    if (pMain)
    {
        if (IsWindowUnicode(pMain->winId()))
            oldProc = (WNDPROC)SetWindowLongW(pMain->winId(), GWL_WNDPROC, (LONG)dockWndProc);
        else
            oldProc = (WNDPROC)SetWindowLongA(pMain->winId(), GWL_WNDPROC, (LONG)dockWndProc);
        appBarMessage(ABM_NEW);
        m_bInit = true;
        setBarState();
        pMain->installEventFilter(this);
    }
}

void WinDockPlugin::slotSetState()
{
    setBarState();
}

void WinDockPlugin::slotAutoHide()
{
    if (pMain->isActiveWindow())
        return;
    DWORD pos = GetMessagePos();
    int x = GET_X_LPARAM(pos);
    int y = GET_Y_LPARAM(pos);
    RECT rc;
    GetWindowRect(pMain->winId(), &rc);
    rc.left  -= GetSystemMetrics(SM_CXDOUBLECLK) * 2;
    rc.right += GetSystemMetrics(SM_CXDOUBLECLK) * 2;
    if (x >= rc.left && x <= rc.right && y >= rc.top && y <= rc.bottom) 
        return;
    if (getState() != ABE_FLOAT && getAutoHide() && bAutoHideVisible)
    {
        bAutoHideVisible = false;
        setBarState(true);
        enableAutoHide(false);
    }
}

void WinDockPlugin::enableAutoHide(bool bState)
{
    if (bState)
        m_autoHide->start(1000);
    else
        m_autoHide->stop();
}

bool WinDockPlugin::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Hide && getAutoHide())
        return true;
    return QObject::eventFilter(o, e);
}

QByteArray WinDockPlugin::getConfig()
{
    return save_data(winDockData, &data);
}

QWidget *WinDockPlugin::getMainWindow()
{
	CorePlugin *core = GET_CorePlugin();
    return core->getMainWindow();
}


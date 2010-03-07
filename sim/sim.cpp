/***************************************************************************
                          sim.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; e<ither version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "simapi.h"
#include "log.h"
#include "misc.h"
#include "profilemanager.h"
#include "clientmanager.h"
#include "simfs.h"
#include "paths.h"
#include "socket/socketfactory.h"
#include "contacts/protocolmanager.h"

#include <QDir>

#include <QLibrary>
#include <QSettings>
#include <QMessageBox>

#ifdef WIN32
//#include <windows.h>
//#include <shlobj.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef USE_KDE
#include <QWidget>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kuniqueapplication.h>
#include "simapp/kdesimapp.h"
#else
#include "aboutdata.h"
#include "simapp/simapp.h"
#endif
#include <QApplication>

#if !defined(WIN32) && !defined(Q_OS_MAC) && !defined(__OS2__)
#include <X11/Xlib.h>
#endif

#include <QSettings>

using namespace SIM;

void simMessageOutput(QtMsgType, const char *msg)
{
    if (logEnabled())
        log(SIM::L_DEBUG, "QT: %s", msg);
}

#ifndef REVISION_NUMBER
	#define REVISION_NUMBER 
#endif

#ifdef CVS_BUILD
#define _VERSION	VERSION " SVN " __DATE__
#else
#define _VERSION	VERSION
#endif

int main(int argc, char *argv[])
{
    SimFileEngineHandler simfs;

    int res = 1;
	QCoreApplication::setOrganizationDomain("sim-im.org");
	QCoreApplication::setApplicationName("Sim-IM");
    new SIM::ProfileManager(SIM::PathManager::configRoot());
    qInstallMsgHandler(simMessageOutput);

#ifdef USE_KDE
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options[] =
        {
            { 0, 0, 0 }
        };
    KCmdLineArgs::addCmdLineOptions( options );
    KUniqueApplication::addCmdLineOptions();
    if (!KUniqueApplication::start())
        exit(-1);
#endif
    SimApp app(argc, argv);
#ifdef Q_OS_MAC
    QString sPluginPath = app.applicationDirPath() + "/../";
#else
    QString sPluginPath = app.applicationDirPath() + "/plugins";
#endif
    QApplication::addLibraryPath(sPluginPath);
    SIM::createSocketFactory();
    SIM::createContactList();
    SIM::createProtocolManager();
    SIM::createPluginManager(argc, argv);
    SIM::createClientManager();

    if(!getPluginManager()->initialize())
        return 1;
    app.setQuitOnLastWindowClosed(false);
    if (SIM::getPluginManager()->isLoaded())
        res = app.exec();
    
    SIM::destroyClientManager();
    SIM::destroyPluginManager();
    SIM::destroyProtocolManager();
    SIM::destroyContactList();
    SIM::destroySocketFactory();
    return res;
}

// vim: set expandtab:


/***************************************************************************
                          homedir.cpp  -  description
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

#include "homedir.h"

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>

#include <QLibrary>
#include <QSettings>

#include "homedircfg.h"

static BOOL (WINAPI *_SHGetSpecialFolderPathA)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
static BOOL (WINAPI *_SHGetSpecialFolderPathW)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;

#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#endif

#include <QDir>
#include <QFile>

#include "log.h"
#include "misc.h"

using namespace std;
using namespace SIM;

Plugin *createHomeDirPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new HomeDirPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
#ifdef WIN32
        I18N_NOOP("Home directory"),
        I18N_NOOP("Plugin provides select directory for store config files"),
#else
        NULL,
        NULL,
#endif
        VERSION,
        createHomeDirPlugin,
        PLUGIN_NO_CONFIG_PATH | PLUGIN_NODISABLE
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

#ifdef WIN32

static const char key_name[] = "SIM";
static const char path_value[] = "Path";

#endif

HomeDirPlugin::HomeDirPlugin(unsigned base)
        : Plugin(base)
{
#ifdef WIN32
    m_bSave    = true;
    QSettings setting( QSettings::NativeFormat, QSettings::UserScope, key_name );
    m_homeDir = setting.value( path_value, QString("%APPDATA%") ).toString();
    m_bDefault = m_homeDir.isNull();
#endif
    QString d;
    EventArg e("-b:", I18N_NOOP("Set home directory"));
    if (e.process() && !e.value().isEmpty()){
        d = e.value();
#ifdef WIN32
        m_bSave   = false;
#endif
    } else {
        d = m_homeDir;
    }
    QDir dir( d );
    if ( d.isEmpty() || !dir.exists() ) {
        m_homeDir = defaultPath();
#ifdef WIN32
        m_bDefault = true;
        m_bSave   = false;
#endif
    }
}

QString HomeDirPlugin::defaultPath()
{
    QString s;
#ifndef WIN32
    struct passwd *pwd = getpwuid(getuid());
    if (pwd){
        s = QFile::decodeName(pwd->pw_dir);
    }else{
        log(L_ERROR, "Can't get pwd");
    }
    if (!s.endsWith("/"))
        s += '/';
#ifdef USE_KDE
    char *kdehome = getenv("KDEHOME");
    if (kdehome){
        s = kdehome;
    }else{
        s += ".kde/";
    }
    if (!s.endsWith("/"))
        s += '/';
    s += "share/apps/sim";
#else // USE_KDE
    
#ifdef __OS2__
    char *os2home = getenv("HOME");
    if (os2home) {
        s = os2home;
        s += "\\";
    }
    s += ".sim-qt4";
    if ( access( s, F_OK ) != 0 ) {
    	mkdir( s, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
#else // __OS2__

#ifdef Q_OS_MAC
    s += "Library/Sim-IM";
#else // Q_OS_MAC
    s += ".sim-qt4";
#endif // Q_OS_MAC

#endif // __OS2__

#endif // USE_KDE
#else
    char szPath[1024];
    szPath[0] = 0;
    QString defPath;
	
	//Fixme:
	//FOLDERID_RoamingAppData <<== this is used in Vista.. should be fixed
	//otherwise the config is stored in "Downloads" per default :-/
	//Windows 2008 Server tested, simply works...
    
	(DWORD&)_SHGetSpecialFolderPathW   = (DWORD)QLibrary::resolve("Shell32.dll","SHGetSpecialFolderPathW");
    (DWORD&)_SHGetSpecialFolderPathA   = (DWORD)QLibrary::resolve("Shell32.dll","SHGetSpecialFolderPathA");
	//(DWORD&)_SHGetKnownFolderPath	   = (DWORD)QLibrary::resolve("Shell32.dll","SHGetKnownFolderPath"); //for Vista :-/

    if (_SHGetSpecialFolderPathW && _SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, true)){
        defPath = QString::fromUtf16((unsigned short*)szPath);
    }else if (_SHGetSpecialFolderPathA && _SHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, true)){
        defPath = QFile::decodeName(szPath);
	}
    //}else if (_SHGetKnownFolderPath && _SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0x00008000, NULL, szPath)){
	//	defPath = QFile::decodeName(szPath);

	/*HRESULT SHGetKnownFolderPath(          REFKNOWNFOLDERID rfid,
    DWORD dwFlags,
    HANDLE hToken,
    PWSTR *ppszPath );*/

	if (!defPath.isEmpty()){
        if (!defPath.endsWith("\\"))
            defPath += '\\';
        defPath += "sim";
        makedir(defPath + '\\');
        QString lockTest = defPath + "\\.lock";
        QFile f(lockTest);
        if (!f.open(QIODevice::ReadWrite|QIODevice::Truncate))
            defPath.clear();
        f.close();
        QFile::remove(lockTest);
    }
    if (!defPath.isEmpty()){
        s = defPath;
    }else{
        s = app_file(QString());
    }
#endif
#ifdef HAVE_CHMOD
    chmod(QFile::encodeName(s), 0700);
#endif
    return QDir::convertSeparators(s);
}

#ifdef WIN32

QWidget *HomeDirPlugin::createConfigWindow(QWidget *parent)
{
	return new HomeDirConfig(parent, this);
}

QByteArray HomeDirPlugin::getConfig()
{
    if (!m_bSave)
        return QByteArray();
    QSettings setting( QSettings::NativeFormat, QSettings::UserScope, key_name );

    if (!m_bDefault){
        setting.setValue( path_value, m_homeDir );
    }else{
        setting.remove( path_value );
    }
    return QByteArray();
}

#endif

QString HomeDirPlugin::buildFileName(const QString &name)
{
    QString s;
    QString fname = name;
    if(QDir(fname).isRelative()) {
        s += m_homeDir;
        s += '/';
    }
    s += fname;
    return QDir::convertSeparators(s);
}

bool HomeDirPlugin::processEvent(Event *e)
{
    if (e->type() == eEventHomeDir){
        EventHomeDir *homedir = static_cast<EventHomeDir*>(e);
        homedir->setHomeDir(buildFileName(homedir->homeDir()));
        return true;
    }
    return false;
}

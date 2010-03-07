
#include <QDir>
#include <QLibrary>
#include <QFile>
#include "paths.h"


#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
static BOOL (WINAPI *_SHGetSpecialFolderPathA)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
static BOOL (WINAPI *_SHGetSpecialFolderPathW)(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate) = NULL;
#else
#include <sys/stat.h>
#endif

#include <cstdlib>

using namespace std;

namespace SIM
{
    QString PathManager::configRoot()
    {
        QString s;
#ifndef WIN32
# ifdef USE_KDE4
        char *kdehome = getenv("KDEHOME");
        if (kdehome){
            s = kdehome;
        }else{
            s += ".kde/";
        }
        s.prepend(QDir::homePath() + QDir::separator());
        if (!s.endsWith("/"))
            s += '/';
        s += "share/apps/sim";
# elif defined(__OS2__)
        char *os2home = getenv("HOME");
        if (os2home) {
            s = os2home;
            s += "\\";
        }
        s += ".sim-qt4";
        if ( access( s, F_OK ) != 0 ) {
            mkdir( s, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
        }
# elif defined( Q_OS_MAC )
        s = QDir::homePath() + "/Library/Sim-IM";
# else
        s = QDir::homePath() + "/.sim-qt4";
# endif
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

        if (_SHGetSpecialFolderPathW && _SHGetSpecialFolderPathW(NULL, szPath, CSIDL_APPDATA, true)){
            defPath = QString::fromUtf16((unsigned short*)szPath);
        }else if (_SHGetSpecialFolderPathA && _SHGetSpecialFolderPathA(NULL, szPath, CSIDL_APPDATA, true)){
            defPath = QFile::decodeName(szPath);
        }

        if (!defPath.isEmpty()){
            if (!defPath.endsWith("\\"))
                defPath += '\\';
            defPath += "sim";
            makedir(defPath + '\\');
            QString lockTest = defPath + "\\.lock";
            QFile f(lockTest);
            if (!f.open(QIODevice::ReadWrite | QIODevice::Truncate))
                defPath = "";
            f.close();
            QFile::remove(lockTest);
        }
        if (!defPath.isEmpty()){
            s = defPath;
        }else{
            s = app_file("");
        }
#endif
        QDir().mkpath(s);
#ifdef HAVE_CHMOD
        chmod(QFile::encodeName(s), 0700);
#endif
        return QDir::convertSeparators(s);
    }
}

// vim: set expandtab:


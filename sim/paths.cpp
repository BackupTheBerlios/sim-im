
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
            //s = appFile(""); //Fixme, what is meant here? QString appFile or PathManager::appFile(const QString &) ???
        }
#endif
        QDir().mkpath(s);
#ifdef HAVE_CHMOD
        chmod(QFile::encodeName(s), 0700);
#endif
        return QDir::convertSeparators(s);
    }

    QString PathManager::appFile(const QString& filename)
    {
        QString appFile_name;
        QString fname = filename;
    #if defined( WIN32 ) || defined( __OS2__ )
        if ((fname[1] == ':') || (fname.left(2) == "\\\\"))
            return filename;
    #ifdef __OS2__
        CHAR buff[MAX_PATH];
        PPIB pib;
        PTIB tib;
        DosGetInfoBlocks(&tib, &pib);
        DosQueryModuleName(pib->pib_hmte, sizeof(buff), buff);
    #else
        WCHAR buff[MAX_PATH];
        GetModuleFileNameW(NULL, buff, MAX_PATH);
    #endif
    #ifdef __OS2__
        QString b = buff;
    #else
        QString b = QString::fromUtf16((unsigned short*)buff);
    #endif
        int idx = b.lastIndexOf('\\');
        if(idx != -1)
            b = b.left(idx+1);
        appFile_name = b;
        if (!appFile_name.endsWith('\\') && !appFile_name.endsWith('/'))
            appFile_name += '\\';
    #else
        if (fname[0] == '/')
            return filename;
    #ifdef USE_KDE
        if (qApp){
            QStringList lst = KGlobal::dirs()->findDirs("data", "sim");
            for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
                QFile fi(*it + f);
                if (fi.exists()){
                    appFile_name = QDir::convertSeparators(fi.name());
                    return appFile_name;
                }
            }
        }
    #endif
    #if !defined( __OS2__ ) && !defined( Q_OS_MAC )
        appFile_name = PREFIX "/share/apps/sim/";
    #endif

    #ifdef Q_OS_MAC
        appFile_name = QApplication::applicationDirPath();
        appFile_name += "/../Resources/";
    #endif

    #endif
        appFile_name += filename;
        return QDir::convertSeparators(appFile_name);
    }
}

// vim: set expandtab:


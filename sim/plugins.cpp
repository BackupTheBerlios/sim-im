/***************************************************************************
                          plugins.cpp  -  description
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

#include "plugins.h"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QLibrary>
#include <QMessageBox>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QByteArray>
#include <QVector>
#include <memory>

#include "socket/simsockets.h"
#include "fetch.h"
#include "exec.h"
#include "misc.h"
#include "xsl.h"
#include "builtinlogger.h"
#include "profilemanager.h"

#include <ctype.h>
#include <errno.h>

#ifndef  LTDL_SHLIB_EXT
# if defined(Q_OS_MAC) /* MacOS needs .a */
#  define  LTDL_SHLIB_EXT ".dylib"
# else
#  if defined(_WIN32) || defined(_WIN64)
#   define  LTDL_SHLIB_EXT ".dll"
#  else
#   define  LTDL_SHLIB_EXT ".so"
#   include <dlfcn.h>
#  endif
# endif
#endif

namespace SIM
{
    typedef QWeakPointer<Plugin> PluginWeakPtr;
	struct pluginInfo
	{
            QString name;
            QString title;
            QString description;
            bool alwaysEnabled;
            bool protocolPlugin;
            PluginWeakPtr plugin;
            Buffer *cfg;           // configuration data
            unsigned base;           // base for plugin types
            QString filePath;
            PluginInfo *info;
            QLibrary *module;        // so or dll handle
            friend class PluginManagerPrivate;
	};

    class PluginPrivate
    {
    public:
        PluginPrivate()
        {
        }

        virtual ~PluginPrivate()
        {
        }

        void setName(const QString& n)
        {
            m_name = n;
        }

        QString name()
        {
            return m_name;
        }

        bool isProtocolPlugin()
        {
            return m_protocol;
        }

        void setProtocolPlugin(bool proto)
        {
            m_protocol = proto;
        }

        void setAlwaysEnabled(bool ae)
        {
            m_alwaysEnabled = ae;
        }

        bool isAlwaysEnabled()
        {
            return m_alwaysEnabled;
        }
        
    private:
        QString m_name;
        bool m_protocol;
        bool m_alwaysEnabled;
    };

    Plugin::Plugin(unsigned base)
        : m_current(base)
        , m_base(base)
        , p(new PluginPrivate)
    {
        p->setProtocolPlugin(false);
        p->setAlwaysEnabled(false);
    }

    Plugin::~Plugin()
    {
        log(L_DEBUG, "Plugin::~Plugin(%s)", qPrintable(name()));
        delete p;
    }

    unsigned Plugin::registerType()
    {
        return m_current++;
    }

    void Plugin::boundTypes()
    {
        m_current = (m_current | 0x3F) + 1;
    }

    void Plugin::setName(const QString& name)
    {
        p->setName(name);
    }

    QString Plugin::name()
    {
        return p->name();
    }

    PluginInfo* Plugin::getInfo()
    {
        return getPluginManager()->getPluginInfo(name());
    }

    class PluginManagerPrivate : public EventReceiver
    {
        public:
            PluginManagerPrivate(int argc, char **argv);
            ~PluginManagerPrivate();


            bool initialize();
            QStringList enumPluginPaths();
            QStringList enumPluginNames();
            PluginPtr plugin(const QString& pluginname);
            QString pluginTitle(const QString& pluginname);
            QString pluginDescription(const QString& pluginname);
            bool isPluginAlwaysEnabled(const QString& pluginname);
            bool isPluginProtocol(const QString& pluginname);

        protected:
            virtual bool processEvent(Event *e);

            bool findParam(EventArg *a);
            void usage(const QString &);

            void scan();
            PluginPtr create( pluginInfo& );
            PluginPtr createPlugin(pluginInfo&);

            void release(pluginInfo&, bool bFree = true);
            void release(const QString &name);
            void release_all();

            bool load( pluginInfo * );
            bool load( const QString &name );
            bool unload( pluginInfo * );
            bool unload( const QString &name );

            void saveState();
            PluginInfo *getPluginInfo(const QString &name);

            pluginInfo *getInfo(const QString &name);
            pluginInfo *getInfo(int n);
            bool setInfo(const QString &name);

            bool arePluginsInBuildDirectory(QStringList& pluginsList);
            bool findPluginsInDir(const QDir &appDir, const QString &subdir, QStringList &pluginsList);
            bool findPluginsInDirPlain(const QDir &dir, QStringList &pluginsList);
            bool initPluginList(const QStringList& pluginsList);
            QString getPluginName(pluginInfo &info);

#ifndef WIN32
            Q_PID execute(const QString &prg, const QStringList &args);
#endif
            void collectGarbage();
       private:
            QString app_name;
            QStringList args;
            QVector<pluginInfo> plugins;
            QStringList cmds;
            QStringList descrs;
            PluginPtr m_core;
            PluginPtr m_homedir; // HACK

            unsigned m_base;
            bool m_bLoaded;
            bool m_bInInit;
            bool m_bAbort;
            bool m_bPluginsInBuildDir;  // plugins in build dir -> full path in pluginInfo.filePath

            ExecManager	*m_exec;
            std::auto_ptr<BuiltinLogger> builtinLogger;

            friend class PluginManager;
    };

    PluginPtr PluginManagerPrivate::plugin( const QString& pluginname ) {
        pluginInfo *info = getInfo( pluginname );
        if( NULL == info )
            return PluginPtr();

        return create( *info );
    }

    QStringList PluginManagerPrivate::enumPluginNames()
    {
        QStringList names;
        foreach(pluginInfo info, plugins)
        {
            names.append(info.name);
        }
        return names;
    }

    QString PluginManagerPrivate::pluginTitle(const QString& pluginname)
    {
        return getInfo(pluginname)->title;
    }

    QString PluginManagerPrivate::pluginDescription(const QString& pluginname)
    {
        return getInfo(pluginname)->description;
    }

    bool PluginManagerPrivate::isPluginAlwaysEnabled(const QString& pluginname)
    {
        return getInfo(pluginname)->alwaysEnabled;
    }

    bool PluginManagerPrivate::isPluginProtocol(const QString& pluginname)
    {
        return getInfo(pluginname)->protocolPlugin;
    }

    PluginInfo* PluginManagerPrivate::getPluginInfo(const QString &name)
    {
        return getInfo(name)->info;
    }

    void PluginManagerPrivate::collectGarbage()
    {
        for(int i = 0; i < plugins.size(); i++)
        {
            if(plugins[i].plugin.isNull() && plugins[i].module)
            {
                release(plugins[i]);
            }
        }
    }
    
    QStringList PluginManagerPrivate::enumPluginPaths()
    {
        QStringList pluginsList;
        if(arePluginsInBuildDirectory(pluginsList))
        {
            m_bPluginsInBuildDir = true;
        }
        else
        {
#if defined( WIN32 ) || defined( __OS2__ )
            QString pluginDir(app_file("plugins"));
#else
            QString pluginDir(PLUGIN_PATH);
#endif
            QStringList pl = QDir(pluginDir).entryList( QStringList( QString("*") + LTDL_SHLIB_EXT ) );
            foreach( QString name, pl ) {
                pluginsList.append( pluginDir + QDir::separator() + name );
            }
        }
        qSort(pluginsList);
        return pluginsList;
    }

    static bool cmp_plugin(pluginInfo p1, pluginInfo p2)
    {
        return QString::compare(p1.name, p2.name, Qt::CaseInsensitive) < 0;
    }

    bool PluginManagerPrivate::findPluginsInDir(const QDir &appDir, const QString &subdir, QStringList &pluginsList)
    {
        QString pluginsDir(appDir.absolutePath());
        log(L_DEBUG, "Searching for plugins in directory '%s'...", qPrintable(pluginsDir));
        int count = 0;
        // trunk/plugins/*
        QDirIterator it(pluginsDir, QDir::Dirs|QDir::NoDotAndDotDot);
        while (it.hasNext()) {
            const QString dir = it.next();
            // trunk/plugins/$plugin_name/$plugin_name.so
            QString pluginFilename;
            if(!subdir.isEmpty())
                pluginFilename = dir + QDir::separator() + subdir + QDir::separator() + it.fileName() + LTDL_SHLIB_EXT;
            else
                pluginFilename = dir + QDir::separator() + it.fileName() + LTDL_SHLIB_EXT;

            if (QFile::exists(pluginFilename)) {
                log(L_DEBUG, "Found '%s'...", qPrintable(pluginFilename));
                pluginsList.append(pluginFilename);
                count++;
            }
        }
        log(L_DEBUG, "%i plugins found.", count);
        return count > 0;
    }

    bool PluginManagerPrivate::findPluginsInDirPlain(const QDir &dir, QStringList &pluginsList)
    {
        log(L_DEBUG, "Searching for plugins in directory '%s'...", qPrintable(dir.absolutePath()));
        int count = 0;
        QStringList files = dir.entryList(QStringList(QString("*").append(LTDL_SHLIB_EXT)), QDir::Files | QDir::NoSymLinks);
        foreach(const QString& file, files)
        {
            log(L_DEBUG, "Found '%s'...", qPrintable(file));
            QString fullPath = dir.absolutePath() + QDir::separator() + file;
            pluginsList.append(fullPath);
            count++;
        }
        log(L_DEBUG, "%i plugins found.", count);
        return count > 0;

    }

    bool PluginManagerPrivate::arePluginsInBuildDirectory(QStringList& pluginsList)
    {
        QDir appDir(qApp->applicationDirPath());
        if(findPluginsInDir(appDir, ".", pluginsList)                    // cmake location is source dir itself
                || findPluginsInDir(appDir.path() + "/../plugins", ".", pluginsList)  // 
                || findPluginsInDir(appDir, ".libs", pluginsList)             // autotools location is .libs subdur
                || findPluginsInDir(appDir.path() + "/../plugins", ".libs", pluginsList) 
                || findPluginsInDir(appDir.path() + "/../plugins", "debug", pluginsList)   // msvc + cmake
                || findPluginsInDir(appDir.path() + "/../plugins", "release", pluginsList) // msvc + cmake
                || findPluginsInDir(appDir.path() + "/../plugins", "relwithdebinfo", pluginsList) // msvc + cmake
                || findPluginsInDirPlain(appDir.path() + "/../lib", pluginsList)
          )
            return true;
        return false;
    }

    QString PluginManagerPrivate::getPluginName(pluginInfo &info)
    {
#if defined( WIN32 ) || defined( __OS2__ )
        QString pluginName = info.filePath;
        if(!m_bPluginsInBuildDir)
            pluginName = "plugins\\" + info.name;
#else
        QString pluginName = info.filePath;
        if(pluginName[0] != '/')
        {
            pluginName = PLUGIN_PATH;
            pluginName += '/';
            pluginName += info.name;
            pluginName += LTDL_SHLIB_EXT;
        }
#endif
        return pluginName;
    }

    bool PluginManagerPrivate::initPluginList(const QStringList& pluginsList)
    {
        foreach( QString path, pluginsList ) {
            QString name = QFileInfo(path).baseName().toLower();
            if( NULL != getInfo( name ) )
                continue;
            pluginInfo info;
            info.plugin         = PluginPtr();
            info.name           = name;
            info.filePath       = path;
            info.cfg            = NULL;
            info.module         = NULL;
            info.info           = NULL;
            info.base           = 0;
            info.alwaysEnabled  = false;
            info.protocolPlugin = false;

            if( !load( &info ) )
                continue;

            info.alwaysEnabled = ( info.info->flags & PLUGIN_NODISABLE ) == PLUGIN_NODISABLE;
            info.title = info.info->title;
            info.description = info.info->description;
            info.protocolPlugin = ( info.info->flags & PLUGIN_PROTOCOL ) == PLUGIN_PROTOCOL;
            log( L_DEBUG, "Found plugin '%s' (%s)", qPrintable(info.name), qPrintable(info.filePath) );
            if( !info.alwaysEnabled ) {
                unload( &info );
            }
            plugins.push_back( info );
        }

        return true;
    }

    bool PluginManagerPrivate::initialize()
    {
        m_base = 0;
        m_bLoaded = false;
        m_bInInit = true;

        QStringList pluginsList = enumPluginPaths();
        initPluginList(pluginsList);
        qSort(plugins.begin(), plugins.end(), cmp_plugin);

        m_core = plugin("_core");
        if( m_core.isNull() ) {
            log(L_ERROR, "Fatal error: Core plugin failed to load. Aborting!");
            m_bAbort = true;
            return false;
        }

        m_homedir = plugin("__homedir");

        EventInit eStart;
        eStart.process();
        if( eStart.abortLoading() ) {
            log(L_ERROR,"EventInit failed - aborting!");
            m_bAbort = true;
            return false;
        }
        return true;
    }

    PluginManagerPrivate::PluginManagerPrivate(int argc, char **argv)
        : EventReceiver(LowPriority), m_bPluginsInBuildDir(false)
    {
        m_bAbort = false;
        unsigned logLevel = L_ERROR | L_WARN;
        // #ifdef DEBUG // zowers: commented out ifdef to be able to get some output from users even on production systems
        logLevel |= L_DEBUG;
        // #endif
        builtinLogger.reset(new BuiltinLogger(logLevel));

        m_exec = new ExecManager;

        app_name = QString::fromLocal8Bit(*argv);
        for (argv++, argc--; argc > 0; argv++, argc--)
            args.push_back(QString::fromLocal8Bit(*argv));

        for (QStringList::iterator it_args = args.begin(); it_args != args.end(); ++it_args){
            if ((*it_args).length()){
                usage(*it_args);
                break;
            }
        }
        m_bInInit = false;
    }

    PluginManagerPrivate::~PluginManagerPrivate()
    {
        release_all();
        delete m_exec;
        setLogEnable(false);
        XSL::cleanup();
    }

    bool PluginManagerPrivate::processEvent(Event *e)
    {
        switch (e->type()){
            case eEventArg:
                {
                    EventArg *a = static_cast<EventArg*>(e);
                    return findParam(a);
                }
            case eEventSaveState:
                saveState();
                break;
            case eEventGetArgs:
                {
                    EventGetArgs *ga = static_cast<EventGetArgs*>(e);
                    ga->setArgs(qApp->argc(), qApp->argv());
                    return true;
                }
#ifndef WIN32
            case eEventExec:
                {
                    EventExec *exec = static_cast<EventExec*>(e);
                    exec->setPid(execute(exec->cmd(), exec->args()));
                    return true;
                }
#endif
            default:
                break;
        }
        return false;
    }

    pluginInfo *PluginManagerPrivate::getInfo(const QString &name)
    {
        if (name.isEmpty())
            return NULL;
        for (int n = 0; n < plugins.size(); n++){
            pluginInfo &info = plugins[n];
            if (info.name == name)
                return &info;
        }
        return NULL;
    }

    void PluginManagerPrivate::release_all()
    {
        if(!plugins.size())
            return;
        for(int n = plugins.size() - 1; n > 0; n--)
        {
            pluginInfo &info = plugins[n];
            release(info);
            if (info.cfg) {
                delete info.cfg;
                info.cfg = NULL;
            }
        }
    }

    bool PluginManagerPrivate::load( const QString &name ) {
        return load( getInfo( name ) );
    }

    bool PluginManagerPrivate::load( pluginInfo *info ) {
        if( NULL == info )
            return false;

        if( NULL == info->module ) {
            info->module = new QLibrary( info->filePath );
            if( !info->module->load()  ) {
                log( L_ERROR, "Can't load plugin %s\n", qPrintable( info->name ) );
                unload( info );
                return false;
            }
        }

        if( NULL == info->info ) {
            PluginInfo* (*getInfo)() = NULL;
            getInfo = (PluginInfo* (*)()) info->module->resolve("GetPluginInfo");
            if( NULL == getInfo ) {
                log( L_ERROR, "Plugin %s doesn't have the GetPluginInfo entry (%s)\n", qPrintable(info->name), qPrintable(info->module->errorString()) );
                unload( info );
                return false;
            }
            info->info = getInfo();
#ifndef WIN32
#ifdef USE_KDE
            if (!(info->info->flags & PLUGIN_KDE_COMPILE)){
                log( L_ERROR, "Plugin %s is compiled without KDE support!\n", qPrintable(info->name));
                unload( info );
                return false;
            }
#else
            if (info->info->flags & PLUGIN_KDE_COMPILE){
                log( L_ERROR, "Plugin %s is compiled with KDE support!\n", qPrintable(info->name));
                unload( info );
                return false;
            }
#endif
#endif
        }

        return true;
    }

    bool PluginManagerPrivate::unload( pluginInfo *info ) {
        if( NULL == info )
            return false;

        if( NULL != info->module ) {
            delete info->module;
            info->module = NULL;
        }
        info->info = NULL;

        return true;
    }

    bool PluginManagerPrivate::unload( const QString &name ) {
        return unload( getInfo( name ) );
    }

    PluginPtr PluginManagerPrivate::create(pluginInfo &info)
    {
        if (!info.plugin.isNull())
            return info.plugin.toStrongRef();
        if( !load( &info ) )
            return PluginPtr();
        return createPlugin(info);
    }

    PluginPtr PluginManagerPrivate::createPlugin(pluginInfo &info)
    {
        log(L_DEBUG, "[1]Load plugin %s", qPrintable(info.name));
        if (info.base == 0){
            m_base += 0x1000;
            info.base = m_base;
        }
        PluginPtr plugin = PluginPtr(info.info->create(info.base, m_bInInit, info.cfg));
        if( plugin == NULL )
            return PluginPtr();
        info.plugin = plugin.toWeakRef();
        plugin->setName(info.name);
        if (info.plugin == NULL) {
            return PluginPtr();
        }
        if (info.cfg){
            delete info.cfg;
            info.cfg = NULL;
        }

        EventPluginChanged e(info.name);
        e.process();
        return plugin;
    }

    void PluginManagerPrivate::release(const QString &name)
    {
        pluginInfo *info = getInfo(name);
        if (info)
            release(*info);
    }

    void PluginManagerPrivate::release(pluginInfo &info, bool bFree)
    {
        if( info.plugin ) {
            info.plugin.clear();
            EventPluginChanged e(info.name);
            e.process();
        }
        if( NULL != info.module ) {
            if( bFree ) {
                log( L_DEBUG, "[2]Unload plugin %s", qPrintable(info.name) );
                delete info.module;
                info.module = NULL;
            }
        }

        info.info = NULL;
    }

    pluginInfo *PluginManagerPrivate::getInfo(int n)
    {
        if (n >= plugins.size())
            return NULL;
        pluginInfo &info = plugins[n];
        return &info;
    }

    bool PluginManagerPrivate::setInfo(const QString &name)
    {
        pluginInfo *info = getInfo(name);
        if (info == NULL)
            return false;
        bool disabled = !ProfileManager::instance()->currentProfile()->enabledPlugins().contains(name);
        if (disabled)
        {
            if (info->plugin == NULL)
                return false;
            release(*info);
            load( info );
            return true;
        }
        if (info->plugin)
            return false;
        load( info );
        createPlugin(*info);
        return true;
    }

    void PluginManagerPrivate::saveState()
    {
        if(!ProfileManager::instance()->currentProfile())
        {
            // if current profile name is empty then should not write config for it.
            log(L_DEBUG, "Refusing writing plugins.conf for empty profile");
            return;
        }

        if (m_bAbort)
            return;
        getContacts()->save();
        ProfileManager::instance()->sync();
    }

    const unsigned long NO_PLUGIN = (unsigned long)(-1);

    bool PluginManagerPrivate::findParam(EventArg *a)
    {
        bool bRet = false;
        if (!a->desc().isEmpty()){
            cmds.push_back(a->arg());
            descrs.push_back(a->desc());
        }
        QString value = QString::null;
        if (a->arg().endsWith(":")){
            unsigned size = a->arg().length();
            QString arg = a->arg().left(size - 1);
            for (QStringList::iterator it = args.begin(); it != args.end(); ++it){
                if (!(*it).startsWith(arg))
                    continue;
                value = (*it).mid(size);
                if (value.length()){
                    *it = QString::null;
                    bRet = true;
                    break;
                }
                ++it;
                if (it != args.end()){
                    value = (*it);
                    *it = QString::null;
                    --it;
                    *it = QString::null;
                }
                bRet = true;
                break;
            }
        }else{
            int idx = args.indexOf(a->arg());
            if(idx >= 0) {
                value = args[idx];
                args[idx].clear();
                bRet = true;
            }
        }
        a->setValue(value);
        return bRet;
    }

    void PluginManagerPrivate::usage(const QString &err)
    {
        QString title = i18n("Bad option %1").arg(err);
        QString text = i18n("Usage: %1 ").arg(app_name);
        QString comment;
        QStringList::iterator itc = cmds.begin();
        QStringList::iterator itd = descrs.begin();
        for (; itc != cmds.end(); ++itc, ++itd){
            QString p = *itc;
            bool bParam = false;
            if (p.endsWith(":")){
                bParam = true;
                p = p.left(p.length() - 1);
            }
            text += '[' + p;
            if (bParam)
                text += "<arg>";
            text += "] ";
            comment += '\t' + p;
            if (bParam)
                comment += "<arg>";
            comment += "\t - ";
            comment += i18n((*itd));
            comment += '\n';
        }
        text += '\n';
        text += comment;
        QMessageBox::critical(NULL, title, text, "Quit", 0);
    }

#ifndef WIN32
    Q_PID PluginManagerPrivate::execute(const QString &prg, const QStringList &args)
    {
        if (prg.isEmpty())
            return (Q_PID)-1;  /* return  invalid pid */

        log(L_DEBUG, "Exec: %s", qPrintable(prg));

        ulong child;
        QStringList arglist;

        // split prg to program name and optional arguments
        const QStringList s = prg.split(" ");

        arglist = s + args;
        const QString prog = arglist[0];
        arglist.pop_front();

        QProcess *proc = new QProcess();
        proc->start(prog, arglist, QIODevice::NotOpen);
        if( proc->waitForStarted(100) ) {
            child = proc->pid();
            QObject::connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), proc, SLOT(deleteLater()));
        } else {
            child = (Q_PID)-1;
            log(L_DEBUG, "can't execute %s: %d", qPrintable(arglist[0]), proc->error());
            delete proc;
        }

        return child;
    }
#endif

    PluginManager::PluginManager(int argc, char **argv)
    {
        EventReceiver::initList();
        p = new PluginManagerPrivate(argc, argv);
    }

    bool PluginManager::initialize()
    {
        return p->initialize();
    }

    void deleteResolver();

    PluginManager::~PluginManager()
    {
        EventQuit().process();
        delete p;
        EventReceiver::destroyList();
        deleteResolver();
        ProfileManager::instance()->sync();
    }

    bool PluginManager::isLoaded()
    {
        return !p->m_bAbort;
    }

    QStringList PluginManager::enumPlugins()
    {
        return p->enumPluginNames();
    }

    PluginPtr PluginManager::plugin(const QString& pluginname)
    {
        return p->plugin(pluginname);
    }

    QString PluginManager::pluginTitle(const QString& pluginname)
    {
        return p->pluginTitle(pluginname);
    }

    QString PluginManager::pluginDescription(const QString& pluginname)
    {
        return p->pluginDescription(pluginname);
    }

    bool PluginManager::isPluginAlwaysEnabled(const QString& pluginname)
    {
        return p->isPluginAlwaysEnabled(pluginname);
    }

    bool PluginManager::isPluginProtocol(const QString& pluginname)
    {
        return p->isPluginProtocol(pluginname);
    }

    PluginInfo* PluginManager::getPluginInfo(const QString& pluginname)
    {
        return p->getPluginInfo(pluginname);
    }

    static PluginManager* g_pluginManager = 0;

    PluginManager* getPluginManager()
    {
        return g_pluginManager;
    }

    void createPluginManager(int argc, char** argv)
    {
        Q_ASSERT(g_pluginManager == 0);
        g_pluginManager = new PluginManager(argc, argv);
    }

    void destroyPluginManager()
    {
        Q_ASSERT(g_pluginManager != 0);
        delete g_pluginManager;
    }
}

// vim: set expandtab:


/***************************************************************************
                          plugins.h  -  description
                             -------------------
    begin                : Sat Oct 28 2006
    copyright            : (C) 2006 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PLUGINS_H
#define _PLUGINS_H

#include <QSharedPointer>
#include <QString>
#include <QWidget>
#include "simapi.h"

class QLibrary;
class QWidget;
class Buffer;

namespace SIM
{
    struct PluginInfo;

    class EXPORT Plugin
    {
    public:
        Plugin();
        virtual ~Plugin();
        virtual QWidget *createConfigWindow(QWidget* /* *parent */ ) { return NULL; }
        virtual QByteArray getConfig() { return QByteArray(); }

        void setName(const QString& n);
        QString name();

        PluginInfo* getInfo();


    private:
        class PluginPrivate* p;
    };

    typedef QSharedPointer<Plugin> PluginPtr;

    class EXPORT PluginManager : public QObject
    {
        Q_OBJECT
    public:
        PluginManager(int argc, char **argv);
        ~PluginManager();

        bool initialize();
        bool isLoaded();
        PluginPtr plugin(const QString& pluginname);
        QStringList enumPlugins();

        QString pluginTitle(const QString& pluginname);
        QString pluginDescription(const QString& pluginname);

        bool isPluginAlwaysEnabled(const QString& pluginname);
        bool isPluginProtocol(const QString& pluginname);

        PluginInfo* getPluginInfo(const QString& pluginname);

    private slots:
        void eventInitAbort();

    private:
        class PluginManagerPrivate *p;

        COPY_RESTRICTED(PluginManager)
    };

    /* Plugin prototype */
    typedef Plugin* createPlugin(unsigned base, bool bStart, Buffer *cfg);
    typedef Plugin* (*createPluginObject)();

	const unsigned PLUGIN_KDE_COMPILE    = 0x0001;
#ifdef USE_KDE
	const unsigned PLUGIN_DEFAULT        = PLUGIN_KDE_COMPILE;
#else
	const unsigned PLUGIN_DEFAULT        = 0x0000;
#endif
	const unsigned PLUGIN_NO_CONFIG_PATH = 0x0002 | PLUGIN_DEFAULT;
	const unsigned PLUGIN_NOLOAD_DEFAULT = 0x0004 | PLUGIN_DEFAULT;

	const unsigned PLUGIN_PROTOCOL       = 0x0008 | PLUGIN_NOLOAD_DEFAULT;
	const unsigned PLUGIN_NODISABLE      = 0x0010;
	const unsigned PLUGIN_RELOAD         = 0x0020;

    struct PluginInfo           // Information in plugin
    {
        const char      *title;         // User title
        const char      *description;   // Description
        const char      *version;       // Version
        unsigned        flags;          // plugin flags
        createPluginObject createObject;
    };

	SIM_EXPORT PluginInfo *GetPluginInfo();

    SIM_EXPORT PluginManager* getPluginManager();
    void EXPORT createPluginManager(int argc, char** argv);
    void EXPORT destroyPluginManager();

} // namespace SIM

// vim: set expandtab:

#endif


#ifndef SIM_PROFILE_H
#define SIM_PROFILE_H

#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include "cfg.h"
#include "plugins.h"

namespace SIM
{
    class EXPORT Profile
    {
    public:
        Profile(const QString& name = "");
        Profile(const ConfigPtr& conf, const QString& name = "");
        virtual ~Profile();

        ConfigPtr config();
        void setConfig(const ConfigPtr& conf);

        QString name();

        QStringList enabledPlugins();
        void enablePlugin(const QString& name);
        void disablePlugin(const QString& name);
        void loadPlugins();

    protected:
        void addPlugin(const QString& name);
        void removePlugin(const QString& name);

    private:
        QString m_name;
        ConfigPtr m_config;
        QList<PluginPtr> m_plugins;
	};

    typedef QSharedPointer<Profile> ProfilePtr;
}


#endif

// vim: set expandtab:



#include "profile.h"
#include "log.h"

namespace SIM
{
    Profile::Profile(const QString& name) 
        : m_name(name)
        , m_config(NULL)
    {

    }

	Profile::Profile(const ConfigPtr& conf, const QString& name) 
        : m_name(name)
        , m_config(conf)
    {
    }

	Profile::~Profile()
    {
    }

	ConfigPtr Profile::config()
    {
        return m_config;
    }

	void Profile::setConfig(const ConfigPtr& conf)
    {
        m_config = conf;
    }

    QString Profile::name()
    {
        return m_name;
    }

    QStringList Profile::enabledPlugins()
    {
        if(m_config.isNull())
            return QStringList();
        QStringList list = m_config->rootPropertyHub()->value("EnabledPlugins").toStringList();
        return list;
    }

    void Profile::enablePlugin(const QString& name)
    {
        if (!this) 
            return; //Fixmee! Where is my instance?

        if(m_config.isNull())
            return;
        QStringList list = enabledPlugins();
        if(!list.contains(name))
        {
            log(L_DEBUG, "enablePlugin(%s)", qPrintable(name));
            list.append(name);
            addPlugin(name);
            m_config->rootPropertyHub()->setValue("EnabledPlugins", list);
        }
    }

    void Profile::disablePlugin(const QString& name)
    {
        if(m_config.isNull())
            return;
        QStringList list = enabledPlugins();
        if(list.contains(name))
        {
            log(L_DEBUG, "disablePlugin(%s)", qPrintable(name));
            list.removeOne(name);
            removePlugin(name);
            m_config->rootPropertyHub()->setValue("EnabledPlugins", list);
        }
    }

    void Profile::addPlugin(const QString& name)
    {
        PluginPtr plugin = getPluginManager()->plugin(name);
        if(!plugin.isNull())
            m_plugins.append(plugin);
    }

    void Profile::removePlugin(const QString& name)
    {
        int i = 0;
        foreach(PluginPtr p, m_plugins) // FIXME
        {
            if(p->name() == name)
            {
                m_plugins.removeAt(i);
                break;
            }
            i++;
        }
    }

    void Profile::loadPlugins()
    {
        m_plugins.clear();
        QStringList plugins = enabledPlugins();
        foreach(QString name, plugins)
        {
            addPlugin(name);
        }
    }
}

// vim: set expandtab:


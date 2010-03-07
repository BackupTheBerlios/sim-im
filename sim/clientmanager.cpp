
#include <QDir>
#include "clientmanager.h"
#include "profilemanager.h"
#include "contacts/protocolmanager.h"
#include "log.h"

namespace SIM
{
    ClientManager::ClientManager()
    {
    }

    ClientManager::~ClientManager()
    {
        save();
    }

    void ClientManager::addClient(ClientPtr client)
    {
        m_clients.insert(client->name(), client);
    }
    
    ClientPtr ClientManager::client(const QString& name)
    {
        ClientMap::iterator it = m_clients.find(name);
        if(it != m_clients.end())
            return it.value();
        return ClientPtr();
    }
    
    void ClientManager::load()
    {
        load_old();
        // TODO
    }

    void ClientManager::load_old()
    {
        QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "clients.conf";
        QFile f(cfgName);
        if (!f.open(QIODevice::ReadOnly)){
            log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
            return;
        }

        PropertyHubPtr currenthub;
        while(!f.atEnd())
        {
            QString line = f.readLine();
            line = line.trimmed();
            if(line.startsWith("["))
            {
                QString section = line.mid(1, line.length() - 2);
                ClientPtr client = createClient(section);
                currenthub = client->properties();
            }
            else
            {
                if(!currenthub.isNull())
                {
                    QStringList keyval = line.split('=');
                    QString val = keyval.at(1);
                    if(val.startsWith('"') && val.endsWith('"'))
                        currenthub->setValue(keyval.at(0), val.mid(1, val.length() - 2));
                    else
                        currenthub->setValue(keyval.at(0), val);
                }
            }
        }
    }

    void ClientManager::save()
    {
        if(!ProfileManager::instance())
            return;
        QDomDocument doc;
        QDomElement root = doc.createElement("clients");
        doc.appendChild(root);
        foreach(const ClientPtr& client, m_clients)
        {
            QDomElement el = doc.createElement("client");
            el.setAttribute("plugin", client->protocol()->plugin()->name());
            el.setAttribute("protocol", client->protocol()->description()->text);
            client->properties()->serialize(el);
            root.appendChild(el);
        }
        QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "clients.xml";
        QFile f(cfgName);
        if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
            return;
        }
        f.write(doc.toByteArray());
        f.close();
    }

    ClientPtr ClientManager::createClient(const QString& name)
    {
        QString pluginname = name.section('/', 0, 0);
        QString protocolname = name.section('/', 1, 1);

        if (pluginname.isEmpty() || protocolname.length() == 0)
            return ClientPtr();
        if(!getPluginManager()->isPluginProtocol(pluginname))
        {
            log(L_DEBUG, "Plugin %s is not a protocol plugin", qPrintable(pluginname));
            return ClientPtr();
        }
        PluginPtr plugin = getPluginManager()->plugin(pluginname);
        if(plugin.isNull())
        {
            log(L_WARN, "Plugin %s not found", qPrintable(pluginname));
            return ClientPtr();
        }
        ProfileManager::instance()->currentProfile()->enablePlugin(pluginname);
        ProtocolPtr protocol = getProtocolManager()->protocol(protocolname);
        if(protocol)
            return protocol->createClient(0);
        log(L_DEBUG, "Protocol %s not found", qPrintable(protocolname));
        return ClientPtr();
    }

    static ClientManager* g_clientManager = 0;

    ClientManager* getClientManager()
    {
        return g_clientManager;
    }

    void createClientManager()
    {
        if(!g_clientManager)
            g_clientManager = new ClientManager();
    }

    void destroyClientManager()
    {
        if(g_clientManager)
        {
            delete g_clientManager;
            g_clientManager = 0;
        }
    }
}

// vim: set expandtab:


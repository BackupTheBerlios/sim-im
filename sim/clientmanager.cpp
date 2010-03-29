
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
        log(L_DEBUG, "ClientManager::load()");
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

        Buffer cfg;
        ClientPtr client;
        while(!f.atEnd()) {
            QByteArray l = f.readLine();
            QString line = l.trimmed();
            if(line.startsWith("[")) {
                if(client) {
                    cfg.setWritePos(cfg.size() - 1);
                    client->deserialize(&cfg);
                    addClient(client);
                    cfg.clear();
                }
                QString clientName = line.mid(1, line.length() - 2);
                QString pluginName = getToken(clientName, '/');
                if (pluginName.isEmpty() || clientName.length() == 0)
                    return;
                if(!getPluginManager()->isPluginProtocol(pluginName))
                {
                    log(L_DEBUG, "Plugin %s is not a protocol plugin", qPrintable(pluginName));
                    return;
                }
                PluginPtr plugin = getPluginManager()->plugin(pluginName);
                if(plugin.isNull())
                {
                    log(L_WARN, "Plugin %s not found", qPrintable(pluginName));
                    return;
                }
                ProfileManager::instance()->currentProfile()->enablePlugin(pluginName);
                ProtocolPtr protocol;
                ProtocolIterator it;
                while ((protocol = ++it) != NULL)
                    if (protocol->description()->text == clientName)
                        client = protocol->createClient(0);
            }
            else {
                if(!l.isEmpty()) {
                    cfg += l;
                }
            }
        }
        if(client) {
            cfg.setWritePos(cfg.size() - 1);
            client->deserialize(&cfg);
            addClient(client);
            cfg.clear();
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
            el.setAttribute("name", client->name());
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
            return protocol->createClient(name);
        log(L_DEBUG, "Protocol %s not found", qPrintable(protocolname));
        return ClientPtr();
    }

    QStringList ClientManager::clientList()
    {
        return m_clients.keys();
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


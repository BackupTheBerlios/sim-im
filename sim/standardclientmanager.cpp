
#include <QDir>
#include "standardclientmanager.h"
#include "clientmanager.h"
#include "profilemanager.h"
#include "contacts/protocolmanager.h"
#include "buffer.h"

namespace SIM {

StandardClientManager::StandardClientManager()
{
}

StandardClientManager::~StandardClientManager()
{
}

void StandardClientManager::addClient(ClientPtr client)
{
    log(L_DEBUG, "Adding client: %s", qPrintable(client->name()));
    m_clients.insert(client->name(), client);
	m_sortedClientNamesList << client->name();
}

ClientPtr StandardClientManager::client(const QString& name)
{
    ClientMap::iterator it = m_clients.find(name);
    if(it != m_clients.end())
        return it.value();
    return ClientPtr();
}

QList<ClientPtr> StandardClientManager::allClients() const
{
    return m_clients.values();
}

bool StandardClientManager::load()
{
    log(L_DEBUG, "ClientManager::load()");
    m_clients.clear();
	m_sortedClientNamesList.clear();
    if(!load_new())
    {
        if(!load_old())
            return false;
    }
    return true;
}

bool StandardClientManager::load_new()
{
    log(L_DEBUG, "ClientManager::load_new()");
    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "clients.xml";
    QFile f(cfgName);
    if (!f.open(QIODevice::ReadOnly)){
        log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
        return false;
    }

    QDomDocument doc;
    doc.setContent(f.readAll());

    QDomElement clients = doc.elementsByTagName("clients").at(0).toElement();
    if(clients.isNull())
        return false;

    QDomNodeList clientlist = clients.elementsByTagName("client");
    for(int clnum = 0; clnum < clientlist.size(); clnum++)
    {
        QDomElement thisClient = clientlist.at(clnum).toElement();
        if(thisClient.isNull())
            return false;
        QString protocolName = thisClient.attribute("protocol");
        QString pluginName = thisClient.attribute("plugin");
        QString clientName = thisClient.attribute("name");
        if(pluginName.isEmpty() || protocolName.isEmpty())
            return false;

        if(!getPluginManager()->isPluginProtocol(pluginName))
        {
            log(L_DEBUG, "Plugin %s is not a protocol plugin", qPrintable(pluginName));
            continue;
        }
        PluginPtr plugin = getPluginManager()->plugin(pluginName);
        if(plugin.isNull())
        {
            log(L_WARN, "Plugin %s not found", qPrintable(pluginName));
            continue;
        }
        ClientPtr newClient;
        ProfileManager::instance()->currentProfile()->enablePlugin(pluginName);

        for(int i = 0; i < getProtocolManager()->protocolCount(); i++)
        {
            ProtocolPtr protocol = getProtocolManager()->protocol(i);
            if (protocol->name() == protocolName)
            {
                newClient = protocol->createClient(clientName);
                addClient(newClient);
            }
        }

        QDomElement clientData = thisClient.elementsByTagName("clientdata").at(0).toElement();
        if(clientData.isNull())
        {
            log(L_WARN, "No client data");
            continue;
        }

        if(!newClient->deserialize(clientData))
        {
            log(L_WARN, "Deserialization error");
            return false;
        }
    }
    return true;
}

bool StandardClientManager::load_old()
{
    log(L_DEBUG, "ClientManager::load_old()");
    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "clients.conf";
    QFile f(cfgName);
    if (!f.open(QIODevice::ReadOnly)){
        log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
        return false;
    }

    Buffer cfg;
    ClientPtr client;
    while(!f.atEnd()) {
        QByteArray l = f.readLine();
        QString line = l.trimmed();
        log(L_DEBUG, "line: %s", qPrintable(line));
        if(line.startsWith("[")) {
            if(client) {
                cfg.setWritePos(cfg.size() - 1);
                client->deserialize(&cfg);
                addClient(client);
                cfg.clear();
				client.clear();
            }
            QString clientName = line.mid(1, line.length() - 2);
            QString pluginName = getToken(clientName, '/');
            if (pluginName.isEmpty() || clientName.length() == 0)
                continue;
            if(!getPluginManager()->isPluginProtocol(pluginName))
            {
                log(L_DEBUG, "Plugin %s is not a protocol plugin", qPrintable(pluginName));
				cfg.clear();    //Fixme: Make sure here, the dropped configuration converted later, if plugin is available again...
				client.clear(); 
                continue;
            }
            PluginPtr plugin = getPluginManager()->plugin(pluginName);
            if(plugin.isNull())
            {
                log(L_WARN, "Plugin %s not found", qPrintable(pluginName));
                continue;
            }
            ProfileManager::instance()->currentProfile()->enablePlugin(pluginName);
            ProtocolPtr protocol;
            ProtocolIterator it;
            while ((protocol = ++it) != NULL)
                if (protocol->name() == clientName)
				{
					cfg.clear();
                    client = protocol->createClient(0);
				}
        }
        else {
            if(!l.isEmpty()) {
                cfg += l;
            }
        }
    }
    if(client) {
        cfg.setReadPos(0);
        cfg.setWritePos(cfg.size() - 1);
        client->deserialize(&cfg);
        addClient(client);
        cfg.clear();
		client.clear();
    }
    return m_clients.count() > 0;
}


bool StandardClientManager::save()
{
    if(!ProfileManager::instance() ||
		m_clients.isEmpty() )
        return false;
    log(L_DEBUG, "ClientManager::save(): %d", m_clients.count());
    QDomDocument doc;
    QDomElement root = doc.createElement("clients");
    doc.appendChild(root);
    foreach(const ClientPtr& client, m_clients)
    {
        QDomElement el = doc.createElement("client");
        el.setAttribute("name", client->name());
        el.setAttribute("plugin", client->protocol()->plugin()->name());
        el.setAttribute("protocol", client->protocol()->name());
        client->properties()->serialize(el);
        QDomElement clientDataElement = doc.createElement("clientdata");
        client->serialize(clientDataElement);
        el.appendChild(clientDataElement);
        root.appendChild(el);
    }
    QString cfgName = ProfileManager::instance()->profilePath() + QDir::separator() + "clients.xml";
    QFile f(cfgName);
    if(!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        log(L_ERROR, "[2]Can't open %s", qPrintable(cfgName));
        return false;
    }
    f.write(doc.toByteArray());
    f.close();
    return true;
}

ClientPtr StandardClientManager::createClient(const QString& name)
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

QStringList StandardClientManager::clientList()
{
	//m_clients.keys();
    return m_sortedClientNamesList;
	
}

ClientPtr StandardClientManager::getClientByProfileName(const QString& name)
{
	return m_clients[name];
}

ClientPtr StandardClientManager::deleteClient(const QString& name)
{

	ClientPtr delClient(getClientByProfileName(name));
	m_clients.erase(m_clients.find(name));
	m_sortedClientNamesList.removeOne(QString(name));
	return delClient;
}

} // namespace SIM

#ifndef STANDARDCLIENTMANAGER_H
#define STANDARDCLIENTMANAGER_H

#include "clientmanager.h"

namespace SIM {

class StandardClientManager : public ClientManager
{
public:
    StandardClientManager();
    virtual ~StandardClientManager();

    virtual void addClient(ClientPtr client);
	virtual ClientPtr deleteClient(const QString& name);
	ClientPtr getClientByProfileName(const QString& name);
    virtual ClientPtr client(const QString& name);
    virtual QStringList clientList();
    virtual QList<ClientPtr> allClients() const;

    virtual bool load();
    virtual bool save();

protected:
    bool load_old();
    bool load_new();

private:
    ClientPtr createClient(const QString& name);
    typedef QMap<QString, ClientPtr> ClientMap;
    ClientMap m_clients;
	QStringList m_sortedClientNamesList;
};

} // namespace SIM

#endif // STANDARDCLIENTMANAGER_H


#ifndef SIM_CLIENTMANAGER_H
#define SIM_CLIENTMANAGER_H

#include <QString>
#include <QMap>
#include "simapi.h"
#include "contacts/client.h"

namespace SIM
{
    class EXPORT ClientManager
    {
    public:
        ClientManager();
        virtual ~ClientManager();

        void addClient(ClientPtr client);
        ClientPtr client(const QString& name);
        QStringList clientList();

        bool load();
        void save();

    protected:
        bool load_old();
        bool load_new();

    private:
        ClientPtr createClient(const QString& name);
        typedef QMap<QString, ClientPtr> ClientMap;
        ClientMap m_clients;
    };

    EXPORT ClientManager* getClientManager();
    void EXPORT createClientManager();
    void EXPORT destroyClientManager();
}

#endif

// vim: set expandtab:


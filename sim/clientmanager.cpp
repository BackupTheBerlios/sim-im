
#include <QDir>
#include <QDomElement>
#include "clientmanager.h"
#include "profilemanager.h"
#include "contacts/protocolmanager.h"
#include "standardclientmanager.h"
#include "log.h"

namespace SIM
{
    static ClientManager* g_clientManager = 0;

    ClientManager* getClientManager()
    {
        return g_clientManager;
    }

    void EXPORT setClientManager(ClientManager* manager)
    {
        if(g_clientManager)
            delete g_clientManager;
        g_clientManager = manager;
    }

    void createClientManager()
    {
        if(!g_clientManager)
            g_clientManager = new StandardClientManager();
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


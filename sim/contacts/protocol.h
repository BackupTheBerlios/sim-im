
#ifndef SIM_PROTOCOL_H
#define SIM_PROTOCOL_H

#include <QSharedPointer>
#include "simapi.h"
#include "plugins.h"
#include "event.h"
#include "cfg.h"
#include "contacts/imstatus.h"
#include "contacts/imcontact.h"
#include "contacts/imstatusinfo.h"

namespace SIM
{
    class Client;
    class EXPORT Protocol
    {
    public:
        Protocol(Plugin *plugin=NULL);
        virtual ~Protocol();
        Plugin  *plugin() { return m_plugin; }
        virtual QSharedPointer<Client> createClient(Buffer *cfg) = 0;
        virtual QSharedPointer<Client> createClient(const QString& name) = 0;
        virtual const CommandDef *description() = 0;
        virtual const CommandDef *statusList() = 0;
        virtual QStringList states() = 0;
        //virtual IMStatusInfo* statusInfo(const QString& id) = 0;
        virtual IMStatusPtr status(const QString& id) = 0;
        virtual const DataDef *userDataDef() = 0;
        virtual IMContact* createIMContact(const QSharedPointer<Client>& client) = 0;

    protected:
        Plugin *m_plugin;
    };

    typedef QSharedPointer<Protocol> ProtocolPtr;
}

#endif

// vim: set expandtab:



#ifndef SIM_PROTOCOL_H
#define SIM_PROTOCOL_H

#include <QSharedPointer>
#include <QBitArray>
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
        virtual QString name() = 0;
        virtual QString iconId() = 0;
        virtual QSharedPointer<Client> createClient(const QString& name) = 0;

        enum Flag
        {
            flNoAuth = 0,
            flMaxFlag
        };

        bool flag(Flag fl) const;
        void setFlag(Flag fl, bool value);

    private:
        Plugin *m_plugin;
        QBitArray m_flags;
    };

    typedef QSharedPointer<Protocol> ProtocolPtr;
}

#endif

// vim: set expandtab:


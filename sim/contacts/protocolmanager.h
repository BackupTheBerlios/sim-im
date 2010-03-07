
#ifndef SIM_PROTOCOLMANAGER_H
#define SIM_PROTOCOLMANAGER_H

#include "protocol.h"
#include "simapi.h"
#include <list>

namespace SIM
{
    class EXPORT ProtocolManager
    {
    public:
        ProtocolManager();
        virtual ~ProtocolManager();

        void addProtocol(ProtocolPtr protocol);
        ProtocolPtr protocol(int index);
        ProtocolPtr protocol(const QString& name);
        int protocolCount();
        void removeProtocol(ProtocolPtr protocol);

    private:
        QList<ProtocolPtr> m_protocols;
    };

    class EXPORT ProtocolIterator
    {
    public:
        ProtocolPtr operator++();
        ProtocolIterator();
        ~ProtocolIterator();
        void reset();

    private:
        int m_index;
        COPY_RESTRICTED(ProtocolIterator)
    };


    EXPORT ProtocolManager* getProtocolManager();
    void EXPORT createProtocolManager();
    void EXPORT destroyProtocolManager();
}

#endif

// vim: set expandtab:


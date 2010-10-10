
#include "protocolmanager.h"

namespace SIM
{
    static ProtocolManager* gs_protocolManager = 0;

    ProtocolManager::ProtocolManager()
    {
    }

    ProtocolManager::~ProtocolManager()
    {
    }

    void ProtocolManager::addProtocol(ProtocolPtr protocol)
    {
        m_protocols.push_back(protocol);
    }

    ProtocolPtr ProtocolManager::protocol(int index)
    {
        return m_protocols.at(index);
    }

    int ProtocolManager::protocolCount()
    {
        return m_protocols.size();
    }

    void ProtocolManager::removeProtocol(ProtocolPtr protocol)
    {
        int i = 0;
        foreach(ProtocolPtr p, m_protocols)
        {
            if(p.data() == protocol.data())
            {
                m_protocols.removeAt(i);
                break;
            }
            i++;
        }

    }

    ProtocolPtr ProtocolManager::protocol(const QString& name)
    {
        foreach(const ProtocolPtr& proto, m_protocols)
        {
            if(proto->name() == name)
                return proto;
        }
        return ProtocolPtr();
    }

    EXPORT ProtocolManager* getProtocolManager()
    {
        return gs_protocolManager;
    }

    void EXPORT createProtocolManager()
    {
        if(!gs_protocolManager)
            gs_protocolManager = new ProtocolManager();
    }

    void EXPORT destroyProtocolManager()
    {
        if(gs_protocolManager)
        {
            delete gs_protocolManager;
            gs_protocolManager = 0;
        }
    }

    ProtocolIterator::ProtocolIterator() : m_index(0)
    {
    }

    void ProtocolIterator::reset()
    {
        m_index = 0;
    }

    ProtocolIterator::~ProtocolIterator()
    {
    }

    ProtocolPtr ProtocolIterator::operator ++()
    {
        if(m_index < getProtocolManager()->protocolCount())
        {
            ProtocolPtr proto = getProtocolManager()->protocol(m_index);
            m_index++;
            return proto;
        }
        return ProtocolPtr();
    }
}

// vim: set expandtab:


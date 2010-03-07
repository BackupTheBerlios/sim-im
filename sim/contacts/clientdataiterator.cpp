
#include "clientdataiterator.h"
#include "clientuserdataprivate.h"
#include "contacts.h"

namespace SIM
{
    class ClientDataIteratorPrivate
    {
    public:
        ClientDataIteratorPrivate(ClientUserDataPrivate *p, Client *client);
        void *operator ++();
        void reset();
        Client *m_lastClient;
    protected:
        ClientUserDataPrivate *m_p;
        ClientUserDataPrivate::iterator m_it;
        Client *m_client;
    };

    ClientDataIteratorPrivate::ClientDataIteratorPrivate(ClientUserDataPrivate *p, Client *client)
    {
        m_p = p;
        m_client = client;
        reset();
    }

    void *ClientDataIteratorPrivate::operator ++()
    {
        for (; m_it != m_p->end(); ++m_it){
            if ((m_client == NULL) || ((*m_it).client == m_client)){
                void *res = (*m_it).data;
                m_lastClient = (*m_it).client;
                ++m_it;
                return res;
            }
        }
        return NULL;
    }

    void ClientDataIteratorPrivate::reset()
    {
        m_lastClient = NULL;
        m_it = m_p->begin();
    }

    ClientDataIterator::ClientDataIterator(ClientUserData &data, Client *client)
    {
        p = new ClientDataIteratorPrivate(data.p, client);
    }

    ClientDataIterator::~ClientDataIterator()
    {
        delete p;
    }

    clientData *ClientDataIterator::operator ++()
    {
        return (clientData*)(++(*p));
    }

    void ClientDataIterator::reset()
    {
        p->reset();
    }

    Client *ClientDataIterator::client()
    {
        return p->m_lastClient;
    }

}

// vim: set expandtab:


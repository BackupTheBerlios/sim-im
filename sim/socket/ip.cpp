
#include <list>
#include "ip.h"
#include "ipresolver.h"

namespace SIM
{
    extern IPResolver *pResolver; // FIXME hide it

    IP::IP() : m_ip(0)
    {
    }

    IP::~IP()
    {
        if (pResolver){
            pResolver->removeAddress( this );
        }
    }

    void IP::set(unsigned long ip, const QString &host)
    {
        bool bResolve = false;
        if (ip != m_ip){
            m_ip = ip;
            m_host = QString::null;
            bResolve = true;
        }
        m_host = host;
        if (bResolve && !m_host.isEmpty())
            resolve();
    }

    void IP::resolve()
    {
        if(!m_host.isEmpty())
            return;
        if (pResolver == NULL)
            pResolver = new IPResolver;

        pResolver->addAddress( this );
    }

}

// vim: set expandtab:


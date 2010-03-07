#include <QHostInfo>

#ifndef WIN32
    // name resolving
    #include <netdb.h>
    #include <arpa/inet.h>
#else
    #include <winsock.h>
#endif

#include "ipresolver.h"

#include "ip.h"
#include "log.h"

namespace SIM
{
    IPResolver *pResolver = NULL;

    void deleteResolver()
    {
        delete pResolver;
    }

    IPResolver::IPResolver()
    {
    }

    IPResolver::~IPResolver()
    {
    }

    void IPResolver::addAddress( IP* ip ) {
        queue.push_back( ip );
        if( queue.count() == 1 )
            QHostInfo::lookupHost(
                    QHostAddress( ip->ip() ).toString(),
                    this,
                    SLOT(resolve_ready(const QHostInfo &))
            );
    }

    void IPResolver::removeAddress( IP* ip ) {
        int i = queue.indexOf( ip );
        if( i != -1 ) {
            queue.removeAt( i );
        }
    }

#define iptoul(a,b,c,d) (unsigned long)(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

    static inline bool isPrivate(unsigned long ip)
    {
        ip = ntohl(ip);
        if ((ip >= iptoul(10,0,0,0) && ip <= iptoul(10,255,255,255)) ||
                (ip >= iptoul(172,16,0,0) && ip <= iptoul(172,31,255,255)) ||
                (ip >= iptoul(192,168,0,0) && ip <= iptoul(192,168,255,255)))
            return true;
        return false;
    }

    void IPResolver::resolve_ready( const QHostInfo &host )
    {
        if( queue.isEmpty() )
            return;

        IP *pIp = queue[0];
        queue.pop_front();
        pIp->set( pIp->ip(), host.hostName() );

        if( queue.count() > 0 )
            QHostInfo::lookupHost(
                    QHostAddress( queue[0]->ip() ).toString(),
                    this,
                    SLOT(resolve_ready(const QHostInfo &))
            );
    }
}

// vim: set expandtab:


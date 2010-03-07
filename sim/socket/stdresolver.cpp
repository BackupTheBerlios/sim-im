
#ifdef WIN32
	#include <winsock.h>
	#include <wininet.h>
#else
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <pwd.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
#endif

#include <QHostAddress>

#include <errno.h>

#include "stdresolver.h"

#include "log.h"

namespace SIM
{
    StdResolver::StdResolver(QObject* parent, const QString& host) : QThread(parent), m_done(false),
        m_timeout(false), m_addr(INADDR_NONE),
        m_host(host)
    {
        log(L_DEBUG, "StdResolver::StdResolver()");
        this->start();
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()),Qt::DirectConnection);
        m_timer->start(2000);
    }

    StdResolver::~StdResolver()
    {
        delete m_timer;
    }

    QHostAddress StdResolver::addr()
    {
        if( INADDR_NONE == m_addr ) {
            return QHostAddress();
        }
        return QHostAddress( m_addr );
    }

    QList<QHostAddress> StdResolver::addresses()
    {
        QList<QHostAddress> listAddresses;
        if( INADDR_NONE != m_addr ) {
            listAddresses << QHostAddress( m_addr );
        }
        return listAddresses;
    }

    QString StdResolver::host() const
    {
        return m_host;
    }

    void StdResolver::run()
    {
        struct hostent* server_entry = gethostbyname(m_host.toUtf8().constData());
        if(server_entry == NULL)
        {
            log(L_WARN, "gethostbyname failed");
            return;
        } 
        m_addr = inet_addr(inet_ntoa(*(struct in_addr*)server_entry->h_addr_list[0]));
        m_done = true;
        QTimer::singleShot(0, parent(), SLOT(resultsReady())); //FIXME parent() crash, because returned pointer is broken sometimes. :((
    }

    void StdResolver::timeout()
    {
        m_timeout = true;
        m_done = true;
        QTimer::singleShot(0, parent(), SLOT(resultsReady()));
    }

    bool StdResolver::isDone()
    {
        return m_done;
    }

    bool StdResolver::isTimeout()
    {
        return m_timeout;
    }

    IResolver* StdResolver::clone(const QString& host)
    {
        return new StdResolver(parent(), host);
    }
}

// vim: set expandtab:


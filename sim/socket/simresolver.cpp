#include "simresolver.h"

#include "socketfactory.h"

#include <QTimer>
#include <QHostInfo>

namespace SIM
{
    SIMResolver::SIMResolver( QObject *parent, const QString &host )
        : QObject(parent)
        , m_sHost( host )
        , m_bDone( false )
        , m_bTimeout( false )

    {
        QTimer::singleShot(20000, this, SLOT(resolveTimeout()));
        QHostInfo::lookupHost(m_sHost, this, SLOT(resolveReady(QHostInfo)));
    }

    SIMResolver::SIMResolver( QObject *parent, const QHostAddress &address )
        : QObject(parent)
        , m_bDone( false )
        , m_bTimeout( false )
    {
        m_listAddresses.push_back( address );
        QTimer::singleShot(20000, this, SLOT(resolveTimeout()));
        QHostInfo::lookupHost( address.toString(), this, SLOT(resolveReady(QHostInfo)) );
    }

    SIMResolver::~SIMResolver()
    {
    }

    void SIMResolver::resolveTimeout()
    {
        if( !m_bDone ) {
            m_bDone    = true;
            m_bTimeout = true;
            getSocketFactory()->setActive(false);
            QTimer::singleShot(0, parent(), SLOT(resultsReady()));
        }
    }

    void SIMResolver::resolveReady(const QHostInfo &host)
    {
        m_bDone = true;
        m_listAddresses = host.addresses();
        m_sHost = host.hostName();
        QTimer::singleShot(0, parent(), SLOT(resultsReady()));
    }

    QHostAddress SIMResolver::addr()
    {
        if (m_listAddresses.isEmpty())
            return QHostAddress();
        return m_listAddresses.first();
    }

    QString SIMResolver::host() const
    {
        return m_sHost;
    }

    bool SIMResolver::isDone()
    {
        return m_bDone;
    }

    bool SIMResolver::isTimeout()
    {
        return m_bTimeout;
    }

    IResolver* SIMResolver::clone(const QString& host)
    {
        return new SIMResolver(parent(), host);
    }
}

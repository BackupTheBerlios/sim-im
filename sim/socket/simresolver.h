
#ifndef SIM_SIMRESOLVER_H
#define SIM_SIMRESOLVER_H

#include "iresolver.h"

#include <QObject>
#include <QString>
#include <QHostAddress>

class QHostInfo;

namespace SIM
{
    class SIMResolver : public QObject, public IResolver
    {
        Q_OBJECT
    public:
        SIMResolver(QObject *parent, const QString &host);
        SIMResolver(QObject *parent, const QHostAddress &address );
        virtual ~SIMResolver();
        virtual QHostAddress addr();
        virtual QList<QHostAddress> addresses() { return m_listAddresses; }
        virtual QString host() const;
        virtual bool isDone();
        virtual bool isTimeout();
        virtual IResolver* clone(const QString& host);

    protected slots:
        void resolveTimeout();
        void resolveReady(const QHostInfo &host);

    private:
        bool   m_bDone;
        bool   m_bTimeout;
        QString m_sHost;
        QList<QHostAddress> m_listAddresses;
    };

}

#endif

// vim: set expandtab:


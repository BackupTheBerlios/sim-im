
#ifndef SIM_IPRESOLVER_H
#define SIM_IPRESOLVER_H

#include <QObject>
#include <QList>

class QHostInfo;

namespace SIM
{
    class IP;

    class IPResolver : public QObject
    {
        Q_OBJECT
    public:
        IPResolver();
        ~IPResolver();
        void addAddress( IP *ip );
        void removeAddress( IP *ip );

    protected slots:
        void resolve_ready( const QHostInfo &host );

    protected:
        QList<IP*> queue;
    };

}

#endif

// vim: set expandtab:


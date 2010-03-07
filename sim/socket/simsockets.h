#ifndef SIM_SIMSOCKETS_H
#define SIM_SIMSOCKETS_H

#include "socketfactory.h"

#include <QObject>

namespace SIM
{
    class EXPORT SIMSockets : public SocketFactory
    {
        Q_OBJECT
    public:
        SIMSockets(QObject *parent);
        virtual ~SIMSockets();
        virtual Socket *createSocket();
        virtual ServerSocket *createServerSocket();

    public slots:
        void idle();
        void checkState();
    };
}

#endif

// vim: set expandtab:


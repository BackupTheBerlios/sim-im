
#ifndef SIM_CLIENTSOCKETNOTIFY_H
#define SIM_CLIENTSOCKETNOTIFY_H

#include "simapi.h"

#include <QHostAddress>

namespace SIM
{
    class EXPORT ClientSocketNotify
    {
    public:
        ClientSocketNotify() {}
        virtual ~ClientSocketNotify() {}
        virtual bool error_state(const QString &err, unsigned code = 0) = 0;
        virtual void connect_ready() = 0;
        virtual void packet_ready() = 0;
        virtual void write_ready() {}
        virtual void resolve_ready(QHostAddress) {}
    };
}

#endif

// vim: set expandtab:

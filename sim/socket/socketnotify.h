#ifndef SIM_SOCKETNOTIFY_H
#define SIM_SOCKETNOTIFY_H

#include <QHostAddress>

namespace SIM
{
    class EXPORT SocketNotify
    {
    public:
        SocketNotify() {}
        virtual ~SocketNotify() {}
        virtual void connect_ready() = 0;
        virtual void read_ready() = 0;
        virtual void write_ready() = 0;
        virtual void error_state(const QString &err_text, unsigned code = 0) = 0;
        virtual void resolve_ready(QHostAddress /*ip*/) {}
	};
}

#endif


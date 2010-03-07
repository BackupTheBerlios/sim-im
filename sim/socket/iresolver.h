
#ifndef SIM_IRESOLVER_H
#define SIM_IRESOLVER_H

#include <QList>

class QString;
class QHostAddress;

namespace SIM
{
    class IResolver
    {
    public:
        virtual ~IResolver() {}
        virtual QHostAddress addr() = 0;
        virtual QList<QHostAddress> addresses() = 0;
        virtual QString host() const = 0;
        virtual bool isDone() = 0;
        virtual bool isTimeout() = 0;
        virtual IResolver* clone(const QString& host) = 0;
    };
}

#endif

// vim: set expandtab:


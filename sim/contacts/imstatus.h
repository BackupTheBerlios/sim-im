
#ifndef SIM_IMSTATUS_H
#define SIM_IMSTATUS_H

#include "simapi.h"
#include <QString>
#include <QIcon>
#include <QSharedPointer>

namespace SIM
{
    class EXPORT IMStatus;
    typedef QSharedPointer<IMStatus> IMStatusPtr;
    class EXPORT IMStatus
    {
    public:
        IMStatus();
        virtual ~IMStatus();

        virtual QString id() const = 0;
        virtual QString name() const = 0;
        virtual bool hasText() const = 0;
        virtual void setText(const QString& t) = 0;
        virtual QString text() const = 0;
        virtual QIcon icon() const = 0;

        virtual QStringList substatuses() = 0;
        virtual IMStatusPtr substatus(const QString& id) = 0;
        virtual IMStatusPtr clone() = 0;
    };
}

#endif

// vim: set expandtab:


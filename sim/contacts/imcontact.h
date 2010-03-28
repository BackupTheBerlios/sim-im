#ifndef IMCONTACT_H
#define IMCONTACT_H

#include <QSharedPointer>
#include "simapi.h"
#include "cfg.h"

namespace SIM
{
    class Client;
    typedef QSharedPointer<Client> ClientPtr;
    class EXPORT IMContact
    {
    public:
        IMContact();
        virtual ~IMContact();

        virtual unsigned long getSign() = 0;

        unsigned long getLastSend() { return m_lastSend; }
        void setLastSend(unsigned long ls) { m_lastSend = ls; }

        virtual ClientPtr client() = 0;

        virtual QByteArray serialize() = 0;
        virtual void deserialize(Buffer* cfg) = 0;
        virtual void deserializeLine(const QString& key, const QString& value) = 0;

        virtual void serialize(QDomElement& element) = 0;
        virtual void deserialize(QDomElement& element) = 0;

    private:
        unsigned long m_lastSend;
    };
}

#endif // IMCONTACT_H

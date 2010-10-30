#ifndef IMCONTACT_H
#define IMCONTACT_H

#include <QWeakPointer>
#include <QDomElement>
#include "simapi.h"
#include "contacts/imstatus.h"
#include "messaging/message.h"

namespace SIM
{
    class Client;
    typedef QWeakPointer<Client> ClientWeakPtr;
    class IMGroup;
    typedef QWeakPointer<IMGroup> IMGroupWeakPtr;

    class EXPORT IMContact
    {
    public:
        IMContact();
        virtual ~IMContact();

        virtual Client* client() = 0;
        virtual IMStatusPtr status() const = 0;

        virtual QString name() const = 0;

        virtual bool sendMessage(const MessagePtr& message) = 0;
        virtual bool hasUnreadMessages() = 0;

        virtual MessagePtr dequeueUnreadMessage() = 0;
        virtual void enqueueUnreadMessage(const MessagePtr& message) = 0;

        virtual IMGroupWeakPtr group() = 0;

        virtual QString makeToolTipText() = 0;

        virtual void serialize(QDomElement& element) = 0;
        virtual void deserialize(QDomElement& element) = 0;
        virtual bool deserialize(const QString& data) = 0;

    };
    typedef QSharedPointer<IMContact> IMContactPtr;
    typedef QWeakPointer<IMContact> IMContactWeakPtr;
}

#endif // IMCONTACT_H

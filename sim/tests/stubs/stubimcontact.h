#ifndef STUBIMCONTACT_H
#define STUBIMCONTACT_H

#include "contacts/imcontact.h"

namespace StubObjects
{
class StubIMContact : public SIM::IMContact
{
public:
    StubIMContact(const SIM::ClientWeakPtr& client) : m_client(client) {}

    virtual SIM::ClientWeakPtr client()
    {
        return m_client;
    }

    virtual SIM::IMStatusPtr status()
    {
        return SIM::IMStatusPtr();
    }

    virtual bool sendMessage(const SIM::MessagePtr& message)
    {
        Q_UNUSED(message);
        return true;
    }

    virtual bool hasUnreadMessages()
    {
        return false;
    }

    virtual SIM::MessagePtr dequeueUnreadMessage()
    {
        return SIM::MessagePtr();
    }

    virtual void enqueueUnreadMessage(const SIM::MessagePtr& message)
    {
        Q_UNUSED(message);
    }

    virtual SIM::IMGroupWeakPtr group()
    {
        return SIM::IMGroupWeakPtr();
    }

    virtual QString makeToolTipText()
    {
        return QString();
    }

    virtual void serialize(QDomElement& element)
    {
        Q_UNUSED(element);
    }

    virtual void deserialize(QDomElement& element)
    {
        Q_UNUSED(element);
    }

    virtual bool deserialize(const QString& data)
    {
        Q_UNUSED(data);
        return false;
    }

private:
    SIM::ClientWeakPtr m_client;
};
}

#endif // STUBIMCONTACT_H

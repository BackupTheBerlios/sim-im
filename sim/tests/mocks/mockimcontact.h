#ifndef MOCKIMCONTACT_H
#define MOCKIMCONTACT_H

#include "contacts/imcontact.h"

namespace MockObjects
{
    class MockIMContact : public SIM::IMContact
    {
    public:
        MOCK_METHOD0(client, SIM::ClientWeakPtr());
        MOCK_METHOD0(status, SIM::IMStatusPtr());
        MOCK_METHOD1(sendMessage, bool(const SIM::MessagePtr& message));
        MOCK_METHOD0(hasUnreadMessages, bool());
        MOCK_METHOD0(dequeueUnreadMessage, SIM::MessagePtr());
        MOCK_METHOD1(enqueueUnreadMessage, void(const SIM::MessagePtr& message));
        MOCK_METHOD0(group, SIM::IMGroupWeakPtr());
        MOCK_METHOD0(makeToolTipText, QString());
        MOCK_METHOD1(serialize, void(QDomElement& element));
        MOCK_METHOD1(deserialize, void(QDomElement& element));
        MOCK_METHOD1(deserialize, bool(const QString&));
    };
}

#endif // MOCKIMCONTACT_H

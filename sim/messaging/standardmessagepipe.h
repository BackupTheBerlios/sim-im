#ifndef STANDARDMESSAGEPIPE_H
#define STANDARDMESSAGEPIPE_H

#include "messagepipe.h"

namespace SIM {

class StandardMessagePipe : public MessagePipe
{
public:
    StandardMessagePipe();
    virtual ~StandardMessagePipe();
    virtual void pushMessage(const MessagePtr& message);
    virtual void addMessageProcessor(MessageProcessor* processor);
    virtual void removeMessageProcessor(const QString& id);
    virtual void setContactList(ContactList* contactList);

private:
    ContactList* m_contactList;
    QList<MessageProcessor*> m_processors;
};

} // namespace SIM

#endif // STANDARDMESSAGEPIPE_H

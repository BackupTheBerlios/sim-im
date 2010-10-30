#include "standardmessagepipe.h"

namespace SIM {

StandardMessagePipe::StandardMessagePipe() : m_contactList(NULL)
{
}

StandardMessagePipe::~StandardMessagePipe()
{
    qDeleteAll(m_processors);
}

void StandardMessagePipe::pushMessage(const MessagePtr& message)
{
    foreach(MessageProcessor* processor, m_processors)
    {
        if(processor->process(message) == MessageProcessor::Block)
            return;
    }
    if(m_contactList)
        m_contactList->incomingMessage(message);
}

void StandardMessagePipe::addMessageProcessor(MessageProcessor* processor)
{
    m_processors.append(processor);
}

void StandardMessagePipe::removeMessageProcessor(const QString& /*id*/)
{
}

void StandardMessagePipe::setContactList(ContactList* contactList)
{
    m_contactList = contactList;
}

} // namespace SIM

#include "messagepipe.h"
#include "standardmessagepipe.h"

namespace SIM {

static MessagePipe* gs_messagePipe = NULL;

EXPORT void createMessagePipe()
{
    Q_ASSERT(!gs_messagePipe);
    gs_messagePipe = new StandardMessagePipe();
}

EXPORT void destroyMessagePipe()
{
    Q_ASSERT(gs_messagePipe);
    delete gs_messagePipe;
    gs_messagePipe = NULL;
}

EXPORT void setMessagePipe(MessagePipe* pipe)
{
    if(gs_messagePipe)
        delete gs_messagePipe;
    gs_messagePipe = pipe;
}

EXPORT MessagePipe* getMessagePipe()
{
    return gs_messagePipe;
}

} // namespace SIM

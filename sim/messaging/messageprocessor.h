#ifndef MESSAGEPROCESSOR_H
#define MESSAGEPROCESSOR_H

#include <QString>
#include "simapi.h"

#include "message.h"

namespace SIM {

class EXPORT MessageProcessor
{
public:
    enum ProcessResult
    {
        Success,
        Block
    };

    virtual ~MessageProcessor() {}
    virtual QString id() const = 0;
    virtual ProcessResult process(const MessagePtr& message) = 0;
};

} // namespace SIM

#endif // MESSAGEPROCESSOR_H

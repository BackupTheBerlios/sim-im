#ifndef EVENTDATA_H
#define EVENTDATA_H

#include <QString>
#include <QSharedPointer>

namespace SIM {

class EventData
{
public:
    EventData();
    virtual ~EventData();
    virtual QString eventId() const = 0;
};

typedef QSharedPointer<EventData> EventDataPtr;

} // namespace SIM

#endif // EVENTDATA_H

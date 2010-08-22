#ifndef IEVENT_H
#define IEVENT_H

#include <QObject>
#include <QSharedPointer>
#include <QString>
#include "simapi.h"

#include "eventdata.h"

namespace SIM {

class EXPORT IEvent : public QObject
{
    Q_OBJECT
public:
    explicit IEvent();

    virtual QString id() = 0;
    virtual bool connectTo(QObject* receiver, const char* receiverSlot) = 0;

public slots:
    virtual void triggered(const EventDataPtr& data) = 0;

};

typedef QSharedPointer<IEvent> IEventPtr;

} // namespace SIM

#endif // IEVENT_H

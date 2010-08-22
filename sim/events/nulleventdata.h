#ifndef NULLEVENTDATA_H
#define NULLEVENTDATA_H

#include "eventdata.h"

namespace SIM {

class EXPORT NullEventData : public EventData
{
public:
    NullEventData();
    virtual QString eventId() const;
};

}

#endif // NULLEVENTDATA_H

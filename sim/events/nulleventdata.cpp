#include "nulleventdata.h"

namespace SIM {

NullEventData::NullEventData() : EventData()
{
}

QString NullEventData::eventId() const
{
    return "null";
}

}

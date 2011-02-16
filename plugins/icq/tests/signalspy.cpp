
#include "signalspy.h"

namespace Helper
{
    SignalSpy::SignalSpy() : connectedCalls(0), packetCalls(0), justSlotCalls(0)
    {

    }

    void SignalSpy::provokeSignal()
    {
        emit justSignal();
    }

    void SignalSpy::packet(int channel, const QByteArray& arr)
    {
        receivedPacket = arr;
        receivedChannel = channel;
        packetCalls++;
    }

    void SignalSpy::error(const QString& str)
    {
        errorString = str;
    }

    void SignalSpy::connected()
    {
        connectedCalls++;
    }

    void SignalSpy::justSlot()
    {
        justSlotCalls++;
    }
}

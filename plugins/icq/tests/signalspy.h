#ifndef SIGNALSPY_H
#define SIGNALSPY_H

#include <QObject>
#include <QByteArray>

namespace Helper
{
    class SignalSpy : public QObject
    {
        Q_OBJECT
    public:

        SignalSpy();

        int receivedChannel;
        QByteArray receivedPacket;
        QString errorString;

        int connectedCalls;
        int packetCalls;
        int justSlotCalls;

        void provokeSignal();

    signals:
        void justSignal();

    public slots:
        void packet(int channel, const QByteArray& arr);
        void error(const QString& str);
        void connected();

        void justSlot();
    };
}

#endif // SIGNALSPY_H

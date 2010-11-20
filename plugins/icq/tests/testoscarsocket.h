#ifndef TESTOSCARSOCKET_H
#define TESTOSCARSOCKET_H

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

        void provokeSignal();

    signals:
        void justSignal();

    public slots:
        void packet(int channel, const QByteArray& arr);
        void error(const QString& str);

        void connected();
    };
}

#endif // TESTOSCARSOCKET_H

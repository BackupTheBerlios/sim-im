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

        QByteArray receivedPacket;
        QString errorString;

        void provokeSignal();

    signals:
        void justSignal();

    public slots:
        void packet(const QByteArray& arr);
        void error(const QString& str);
    };
}

#endif // TESTOSCARSOCKET_H

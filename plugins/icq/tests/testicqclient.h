#ifndef TESTICQCLIENT_H
#define TESTICQCLIENT_H

#include <QObject>

namespace Helper
{
    class SignalEmitter : public QObject
    {
        Q_OBJECT
    public:

        void emitConnectedSignal()
        {
            emit connected();
        }

    signals:
        void connected();
    };
}

#endif // TESTICQCLIENT_H

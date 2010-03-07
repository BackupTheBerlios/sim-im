
#ifndef SIM_STDRESOLVER_H
#define SIM_STDRESOLVER_H

#include <QThread>
#include <QString>
#include <QTimer>
#include "iresolver.h"

namespace SIM
{
    class StdResolver : public QThread, public IResolver
    {
        Q_OBJECT
    public:
        StdResolver(QObject* parent, const QString& host);
        virtual ~StdResolver();
        virtual QHostAddress addr();
        virtual QList<QHostAddress> addresses();
        virtual QString host() const;
        virtual bool isDone();
        virtual bool isTimeout();
        virtual IResolver* clone(const QString& host);

    protected:
        virtual void run();

    protected slots:
        void timeout();

    private:
        bool m_done;
        bool m_timeout;
        unsigned long m_addr;
        QString m_host;
        QTimer* m_timer;
    };

}

#endif

// vim: set expandtab:


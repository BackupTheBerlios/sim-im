
#ifndef SIM_INTERFACECHECKER_H
#define SIM_INTERFACECHECKER_H

#include <QObject>
#include <QString>
#include <map>
#include <string>

#include "simapi.h"

namespace SIM
{
    class EXPORT InterfaceChecker : public QObject
    {
        Q_OBJECT
    public:
        InterfaceChecker(int polltime = 5000, bool raiseEvents = false);
        virtual ~InterfaceChecker();

        void setPollTime(int polltime);

    signals:
        void interfaceUp(QString ifname);
        void interfaceDown(QString ifname);

    protected:
        void timerEvent(QTimerEvent* e);

    private:
        typedef struct
        {
            bool present;
            bool state;
        } tIFState;

        int m_pollTime;
        int m_timerID;
        int m_testSocket;
        std::map<std::string, tIFState> m_states;
        bool m_raiseEvents;
    };
}

#endif

// vim: set expandtab:


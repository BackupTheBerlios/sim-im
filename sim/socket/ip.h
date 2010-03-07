
#ifndef SIM_IP_H
#define SIM_IP_H

#include <QString>

namespace SIM
{
    class IP
    {
    public:
        IP();
        ~IP();
        void set(unsigned long ip, const QString &host);
        void resolve();
        unsigned long ip() const { return m_ip; }
        const QString &host() const { return m_host; }

    protected:
        unsigned long m_ip;
        QString m_host;
    };
}

#endif

// vim: set expandtab:


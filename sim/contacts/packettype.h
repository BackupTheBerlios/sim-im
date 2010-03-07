
#ifndef SIM_PACKETTYPE_H
#define SIM_PACKETTYPE_H

#include <QString>
#include "simapi.h"

namespace SIM
{
    class EXPORT PacketType
    {
    public:
        PacketType(unsigned id, const QString &name, bool bText);
        ~PacketType();
        unsigned id() const { return m_id; }
        const QString &name() const { return m_name; }
        bool isText() const { return m_bText; }
    protected:
        unsigned m_id;
        QString  m_name;
        bool     m_bText;
    };
}

#endif

// vim: set expandtab:


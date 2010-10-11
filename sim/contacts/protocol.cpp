#include "protocol.h"

namespace SIM
{
    Protocol::Protocol(Plugin *plugin)
        :m_plugin(plugin)
    {
        m_flags.resize(flMaxFlag);
    }

    Protocol::~Protocol()
    {
    }

    bool Protocol::flag(Flag fl) const
    {
        return m_flags.at(fl);
    }
    void Protocol::setFlag(Flag fl, bool value)
    {
        m_flags.setBit(fl, value);
    }

}

// vim: set expandtab:


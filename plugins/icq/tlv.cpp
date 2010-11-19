#include "tlv.h"

Tlv::Tlv(int id, const QByteArray& data) : m_id(id), m_data(data)
{
}

int Tlv::id() const
{
    return m_id;
}

QByteArray Tlv::data() const
{
    return m_data;
}

quint16 Tlv::toUint16() const
{
    if(m_data.size() < 2)
        return 0;
    return m_data[1] + m_data[0] * 0x100;
}

quint32 Tlv::toUint32() const
{
    if(m_data.size() < 4)
        return 0;
    return m_data[3] + m_data[2] * 0x100 + m_data[1] * 0x10000 + m_data[0] * 0x1000000;
}

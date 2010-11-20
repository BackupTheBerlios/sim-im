#include "tlv.h"

Tlv::Tlv() : m_id(0), m_valid(false)
{

}

Tlv::Tlv(int id, const QByteArray& data) : m_id(id), m_data(data), m_valid(true)
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

bool Tlv::isValid() const
{
    return m_valid;
}

Tlv Tlv::fromUint16(int id, int value)
{
    QByteArray arr;
    arr.append((value >> 8) & 0xff);
    arr.append(value & 0xff);
    return Tlv(id, arr);
}

Tlv Tlv::fromUint32(int id, int value)
{
    QByteArray arr;
    arr.append((value >> 24) & 0xff);
    arr.append((value >> 16) & 0xff);
    arr.append((value >> 8) & 0xff);
    arr.append(value & 0xff);
    return Tlv(id, arr);
}

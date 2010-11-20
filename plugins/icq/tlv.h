#ifndef TLV_H
#define TLV_H

#include "icq_defines.h"
#include <QByteArray>

class ICQ_EXPORT Tlv
{
public:
    Tlv(int id, const QByteArray& data);

    int id() const;
    QByteArray data() const;

    quint16 toUint16() const;
    quint32 toUint32() const;

private:
    int m_id;
    QByteArray m_data;
};

#endif // TLV_H

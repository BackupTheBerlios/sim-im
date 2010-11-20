#ifndef TLVLIST_H
#define TLVLIST_H

#include "tlv.h"
#include "icq_defines.h"
#include <QList>

class ICQ_EXPORT TlvList
{
public:
    TlvList();

    void append(const Tlv& tlv);
    Tlv at(int index) const;

    int size() const;

    static TlvList fromByteArray(const QByteArray& data);
    QByteArray toByteArray();

private:
    QList<Tlv> m_tlvs;
};

#endif // TLVLIST_H

#include "tlvlist.h"
#include "bytearrayparser.h"
#include "bytearraybuilder.h"

TlvList::TlvList()
{
}

void TlvList::append(const Tlv& tlv)
{
    m_tlvs.append(tlv);
}

Tlv TlvList::at(int index) const
{
    return m_tlvs.at(index);
}

Tlv TlvList::firstTlv(int id) const
{
    foreach(const Tlv& tlv, m_tlvs)
    {
        if(tlv.id() == id)
            return tlv;
    }
    return Tlv();
}

int TlvList::size() const
{
    return m_tlvs.size();
}

bool TlvList::contains(int id) const
{
    foreach(const Tlv& tlv, m_tlvs)
    {
        if(tlv.id() == id)
            return true;
    }
    return false;
}

TlvList TlvList::fromByteArray(const QByteArray& data)
{
    ByteArrayParser parser(data);
    TlvList list;

    while(!parser.atEnd())
    {
        quint16 id = parser.readWord();
        int length = parser.readWord();
        QByteArray data = parser.readBytes(length);
        if(data.size() < length)
            break;
        list.append(Tlv(id, data));
    }

    return list;
}

QByteArray TlvList::toByteArray()
{
    ByteArrayBuilder builder;
    foreach(const Tlv& tlv, m_tlvs)
    {
        builder.appendWord(tlv.id());
        builder.appendWord(tlv.data().length());
        builder.appendBytes(tlv.data());
    }
    return builder.getArray();
}

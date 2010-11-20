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

int TlvList::size() const
{
    return m_tlvs.size();
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

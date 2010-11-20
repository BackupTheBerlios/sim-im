#ifndef BYTEARRAYBUILDER_H
#define BYTEARRAYBUILDER_H

#include <QByteArray>
#include "icq_defines.h"

class ICQ_EXPORT ByteArrayBuilder
{
public:
    enum Endianness
    {
        BigEndian,
        LittleEndian
    };

    // BigEndian by default - because icq uses it
    ByteArrayBuilder(Endianness endianness = BigEndian);

    void appendByte(quint8 value);
    void appendWord(quint16 value);
    void appendDword(quint32 value);
    void appendBytes(const QByteArray& bytes);

    QByteArray getArray() const;

private:
    QByteArray m_array;
    Endianness m_endianness;
};

#endif // BYTEARRAYBUILDER_H

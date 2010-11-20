#ifndef BYTEARRAYPARSER_H
#define BYTEARRAYPARSER_H

#include <QByteArray>
#include <QBuffer>
#include "icq_defines.h"

class ICQ_EXPORT ByteArrayParser
{
public:
    enum Endianness
    {
        BigEndian,
        LittleEndian
    };

    ByteArrayParser(const QByteArray& arr, Endianness endianness = BigEndian);

    quint8 readByte();
    quint16 readWord();
    quint32 readDword();
    QByteArray readBytes(int count);
    QByteArray readAll();

    bool atEnd();

private:
    QByteArray m_array;
    QBuffer m_buffer;
    Endianness m_endianness;
};

#endif // BYTEARRAYPARSER_H

#include "bytearraybuilder.h"

#ifdef WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <arpa/inet.h>
#endif
ByteArrayBuilder::ByteArrayBuilder(Endianness endianness) : m_endianness(endianness)
{
}

void ByteArrayBuilder::appendByte(quint8 value)
{
    m_array.append(value);
}

void ByteArrayBuilder::appendWord(quint16 value)
{
    int v = (m_endianness == BigEndian) ? value : htons(value);
    m_array.append((v >> 8) & 0xff);
    m_array.append(v & 0xff);
}

void ByteArrayBuilder::appendDword(quint32 value)
{
    int v = (m_endianness == BigEndian) ? value : htonl(value);
    m_array.append((v >> 24) & 0xff);
    m_array.append((v >> 16) & 0xff);
    m_array.append((v >> 8) & 0xff);
    m_array.append(v & 0xff);
}

void ByteArrayBuilder::appendBytes(const QByteArray& bytes)
{
    m_array.append(bytes);
}

QByteArray ByteArrayBuilder::getArray() const
{
    return m_array;
}

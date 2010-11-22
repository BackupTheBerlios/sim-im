#include "bytearrayparser.h"

#ifdef WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <arpa/inet.h>
#endif

ByteArrayParser::ByteArrayParser(const QByteArray& arr, Endianness endianness) : m_array(arr), m_buffer(&m_array), m_endianness(endianness)
{
    m_buffer.open(QIODevice::ReadOnly);
}

quint8 ByteArrayParser::readByte()
{
    quint8 b = 0;
    m_buffer.getChar((char*)&b);
    return b;
}

quint16 ByteArrayParser::readWord()
{
    quint8 b[2];
    m_buffer.getChar((char*)&b[0]);
    m_buffer.getChar((char*)&b[1]);

    quint16 result = b[1] | (b[0] << 8);
    if(m_endianness == LittleEndian)
        result = htons(result);
    return result;
}

quint32 ByteArrayParser::readDword()
{
    quint8 b[4];
    m_buffer.getChar((char*)&b[0]);
    m_buffer.getChar((char*)&b[1]);
    m_buffer.getChar((char*)&b[2]);
    m_buffer.getChar((char*)&b[3]);

    quint32 result = b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
    if(m_endianness == LittleEndian)
        result = htonl(result);
    return result;
}

QByteArray ByteArrayParser::readBytes(int count)
{
    return m_buffer.read(count);
}

QByteArray ByteArrayParser::readAll()
{
    return m_buffer.readAll();
}

bool ByteArrayParser::atEnd()
{
    return m_buffer.atEnd();
}

/***************************************************************************
                          buffer.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <netinet/in.h>
#endif

#include <vector>

#include "buffer.h"
#include "event.h"
#include "log.h"

using namespace std;
using namespace SIM;

#ifdef WORDS_BIGENDIAN
# define SWAP_S(s)  s = ((s&0xFF)<<8) + ((s&0xFF00)>>8);  
# define SWAP_L(s)  s = ((s&0xFF)<<24) + ((s&0xFF00)<<8) + ((s&0xFF0000)>>8) + ((s&0xFF000000)>>24); 
#else
# define SWAP_S(s)
# define SWAP_L(s)
#endif

// Buffer
Buffer::Buffer(unsigned size)
        : QByteArray(size, '\0')
{
    init(size);
}

Buffer::Buffer(const QByteArray &ba)
    : QByteArray(ba)
{
    init(ba.size());
    m_posWrite = ba.size();
}

/*Buffer::Buffer(const QByteArray &cstr)
    : QByteArray(cstr.copy())
{
    uint len = cstr.length();
    init(len);
    m_posWrite = len;
}*/

Buffer::~Buffer()
{
}

void Buffer::init(unsigned size)
{
    m_posRead = 0;
    m_posWrite = 0;
    m_packetStartPos = 0;
    m_startSection = 0;
    resize(size);
}

void Buffer::incReadPos(int n)
{
    m_posRead += n;
    if (m_posRead > m_posWrite) m_posRead = m_posWrite;
}

bool Buffer::add(uint addSize)
{
    return(resize(size()+addSize));
}

bool Buffer::resize(uint size)
{
    //bool bRet = QByteArray::resize(size);
    QByteArray::resize(size);
    if (m_posWrite > size)
        m_posWrite = size;
    if (m_posRead > size)
        m_posRead = size;
    return true;
}

void Buffer::setWritePos(unsigned n)
{
    m_posWrite = n;
    if (m_posRead > m_posWrite) m_posRead = m_posWrite;
    if (m_posWrite > (unsigned)size())
        resize(m_posWrite);
}

void Buffer::setReadPos(unsigned n)
{
    if (n > m_posWrite)
        n = m_posWrite;
    m_posRead = n;
}

void Buffer::pack(const char *d, unsigned s)
{
    if(s == 0)
        return;
    if(m_posWrite+s > (unsigned)size())
        resize(m_posWrite+s);
    if(d)
	{
        memcpy((char*)data() + m_posWrite, d, s);
    }
	else
	{
        memcpy((char*)data() + m_posWrite, "", 1);
    }
    m_posWrite += s;
}

unsigned Buffer::unpack(char *d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    memcpy(d, data() + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

Buffer& Buffer::operator << (char c)
{
    pack(&c, 1);
    return *this;
}

Buffer &Buffer::operator << (const char *str)
{
    if(!str)
        return *this;
    pack(str, strlen(str));
    return *this;
}

Buffer &Buffer::operator << (unsigned short c)
{
    c = htons(c);
    pack((char*)&c, 2);
    return *this;
}

Buffer &Buffer::operator << (long c)
{
    /* XXX:
       FIXME WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    c = htonl(c);
    pack((char*)&c, 4);
    return *this;
}

Buffer &Buffer::operator >> (char &c)
{
    if (unpack(&c, 1) != 1)
        c = 0;
    return *this;
}

Buffer &Buffer::operator >> (unsigned short &c)
{
    if (unpack((char*)&c, 2) != 2)
        c = 0;
    c = ntohs(c);
    return *this;
}

Buffer &Buffer::operator >> (long &c)
{
    /* XXX:
       FIXME WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    if (unpack((char*)&c, 4) != 4)
        c = 0;
    c = ntohl(c);
    return *this;
}

Buffer &Buffer::operator >> (unsigned long &c)
{
    return operator >> ((long&)c);
}


#ifndef WIN32

Buffer &Buffer::operator >> (uint32_t &c)
{
    if (unpack((char*)&c, 4) != 4)
        c = 0;
    c = ntohl(c);
    return *this;
}

#endif

bool Buffer::scan(const char *substr, QByteArray &res)
{
    char c = *substr;
    for (unsigned pos = readPos(); pos < writePos(); pos++){
        if (*data(pos) != c)
            continue;
        const char *sp = substr;
        for (unsigned pos1 = pos; *sp; pos1++, sp++){
            if (pos1 >= writePos())
                break;
            if (*data(pos1) != *sp)
                break;
        }
        if (*sp == 0){
            QByteArray resCStr = "";
            if (pos - readPos()){
                unpack(resCStr, pos - readPos());
            }
            res = resCStr;
            incReadPos(pos + strlen(substr) - readPos());
            return true;
        }
    }
    return false;
}

void Buffer::packetStart()
{
    m_packetStartPos = writePos();
}

static int findStartSection(const Buffer *pBuf, unsigned start)
{
    int idx = start == ~0U ? 0 : start;

    for ( ; ; ) {
        if(idx >= (int)pBuf->size())
            return -1;
        idx = pBuf->indexOf( '[', idx);
        if(idx == -1)
            return -1;
        if( idx == 0 || pBuf->at( idx - 1 ) == '\n' )
            return idx;
        idx++;
    }
}

static int findEndSection(const Buffer *pBuf, unsigned start)
{
    int idx = start == ~0U ? 0 : start;

	for ( ; ; ) {
        if(idx >= (int)pBuf->size())
            return -1;
        idx = pBuf->indexOf( ']', idx);
        if(idx == -1)
            return -1;
        if( idx == (int)pBuf->size() - 1 || pBuf->at( idx + 1 ) == '\n' )
            return idx;
        idx++;
    }
}

QByteArray Buffer::getSection(bool bSkip)
{
    QByteArray str;
    unsigned start = m_posRead;
    unsigned end = m_posRead;

    if( bSkip )
        start = findStartSection(this, m_posRead + 1);
    if( start == ~0U )
        return str;
    start = findStartSection( this, start );
    end   = findEndSection( this, start );
    if( start == ~0U || end == ~0U )
        return str;
    m_startSection = m_posRead = start;

    str = QByteArray( data() + start + 1, end - start - 1 );

    m_posRead = end + 1;
    if ( m_posRead < (unsigned)size() )
        if ( at(m_posRead) == '\n' )
            m_posRead++;
    if ( m_posRead >= (unsigned)size() )
        m_posRead = size() - 1;
    m_posWrite = findStartSection( this, end );
    if( m_posWrite == ~0U )
        m_posWrite = size();

    return str;
}

QByteArray Buffer::getLine()
{
    if (readPos() >= writePos())
        return QByteArray();
    unsigned start = m_posRead;
    int end = indexOf('\n', start);
    if(end == -1)
        end = size();
    QByteArray res = QByteArray(data() + start, end - start);
    m_posRead = end + 1; 
    if ( m_posRead < (unsigned)size() )
        if ( at(m_posRead) == '\n' )
            m_posRead++;

    return res;
}

// for Buffer::scan()
unsigned Buffer::unpack(QByteArray &d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    d = QByteArray(data() + m_posRead, readn + 1);
    m_posRead += readn;
    return readn;
}

// for msn
unsigned Buffer::unpack(QString &d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    d = QString::fromUtf8(data() + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

// vim: set expandtab:


/***************************************************************************
                          msnfiltetransfer.cpp  -  description
                             -------------------
    begin                : Fri Jan 05 2007
    copyright            : (C) 2007 Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#include "simapi.h"
#include "log.h"

#include "icqbuffer.h"
#include <QByteArray>

using namespace SIM;

// FIXME: move into own file
#ifdef WORDS_BIGENDIAN
# define SWAP_S(s)  s = ((s&0xFF)<<8) + ((s&0xFF00)>>8);  
# define SWAP_L(s)  s = ((s&0xFF)<<24) + ((s&0xFF00)<<8) + ((s&0xFF0000)>>8) + ((s&0xFF000000)>>24); 
#else
# define SWAP_S(s)
# define SWAP_L(s)
#endif
// Tlv
Tlv::Tlv(unsigned short num, unsigned short size, const char *data)
        : m_nNum(num), m_nSize(size)
{
    m_data.resize(m_nSize + 1);
    memcpy(m_data.data(), data, m_nSize);
    m_data[(int)m_nSize] = 0;
}
Tlv::Tlv(unsigned short num, QByteArray& array) : m_nNum(num), m_nSize(array.size())
{
    m_data = array;
}

Tlv::operator uint16_t () const
{
    return (m_nSize >= 2) ? htons(*((uint16_t*)m_data.data())) : 0;
}

Tlv::operator uint32_t () const
{
    return (m_nSize >= 4) ? htonl(*((uint32_t*)m_data.data())) : 0;
}

// TlvList
TlvList::TlvList()
{}

TlvList::TlvList(ICQBuffer &b, unsigned nTlvs)
{
    for(unsigned n = 0; (b.readPos() < (unsigned)b.size()) && (n < nTlvs); n++)
	{
        unsigned short num, size;
        b >> num >> size;
        if (b.readPos() + size > (unsigned)b.size())
            break;
        append(new Tlv(num, size, b.data(b.readPos())));
        b.incReadPos(size);
    }
}

TlvList::~TlvList()
{
    qDeleteAll(*this);
}

Tlv *TlvList::operator()(unsigned short num, int skip)
{
    for(uint i = 0; i < (unsigned)count(); i++) {
        if ((at(i))->Num() == num) {
            if(skip == 0)
                return at(i);
            --skip;
        }
    }
    return NULL;
}

ICQBuffer::ICQBuffer(unsigned size)
  : Buffer(size)
{}

ICQBuffer::ICQBuffer(const QByteArray &ba)
  : Buffer(ba)
{}

ICQBuffer::ICQBuffer(Tlv &tlv)
    : Buffer(tlv.Size())
{
    pack((char*)tlv, tlv.Size());
}


ICQBuffer::~ICQBuffer()
{}

void ICQBuffer::tlv(unsigned short n, const char *data, unsigned short len)
{
    *this << n << len;
    pack(data, len);
}

void ICQBuffer::tlvLE(unsigned short n, const char *data, unsigned short len)
{
    pack(n);
    pack(len);
    pack(data, len);
}

void ICQBuffer::tlv(unsigned short n, const char *data)
{
    if (data == NULL)
        data = "";
    tlv(n, data, (unsigned short)strlen(data));
}

void ICQBuffer::tlvLE(unsigned short n, const char *data)
{
    if (data == NULL)
        data = "";
    unsigned short len = strlen(data) + 1;
    pack(n);
    pack((unsigned short)(len + 2));
    pack(len);
    pack(data, len);
}

void ICQBuffer::tlv(unsigned short n, unsigned short c)
{
    c = htons(c);
    tlv(n, (char*)&c, 2);
}

void ICQBuffer::tlvLE(unsigned short n, unsigned short c)
{
    pack(n);
    pack((unsigned short)2);
    pack(c);
}

void ICQBuffer::tlv(unsigned short n, unsigned long c)
{
    /* XXX:
     * WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    c = htonl(c);
    tlv(n, (char*)&c, 4);
}

void ICQBuffer::tlvLE(unsigned short n, unsigned long c)
{
    /* XXX:
     * WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    pack(n);
    pack((unsigned short)4);
    pack(c);
}

ICQBuffer &ICQBuffer::operator << (const TlvList &tlvList)
{
    unsigned size = 0;
    for (uint i = 0; i < (unsigned)tlvList.count(); i++)
        size += tlvList[(int)i]->Size() + 4;
    *this << (unsigned short)size;
    for (uint i = 0; i < (unsigned)tlvList.count(); i++) {
        Tlv *tlv = tlvList[(int)i];
        *this << tlv->Num() << (int)tlv->Size();
        pack(*tlv, tlv->Size());
    }
    return *this;
}

ICQBuffer &ICQBuffer::operator << (const QString &s)
{
    QByteArray utf8 = s.toUtf8();
	unsigned short size = (unsigned short)(utf8.length() + 1);
    *this << (unsigned short)htons(size);
    pack(utf8, size);
    return *this;
}

/*ICQBuffer &ICQBuffer::operator << (const QByteArray &s)
{
    if(!s.length())
        return *this;
    unsigned short size = (unsigned short)(s.length() + 1);
    *this << (unsigned short)htons(size);
    pack(s, size);
    return *this;
}*/

ICQBuffer &ICQBuffer::operator << (const QByteArray &s)
{
    if(!s.size())
        return *this;
    unsigned short size = (unsigned short)(s.size());
    *this << (unsigned short)htons(size);
    pack(s, size);
    return *this;
}

ICQBuffer &ICQBuffer::operator << (const Buffer &b)
{
    unsigned short size = (unsigned short)(b.size() - b.readPos());
    *this << (unsigned short)htons(size);
    pack(b.data(b.readPos()), size);
    return *this;
}

ICQBuffer &ICQBuffer::operator << (char c)
{
    pack(&c, 1);
    return *this;
}

ICQBuffer &ICQBuffer::operator << (const char *str)
{
    if(!str)
        return *this;
    pack(str, strlen(str));
    return *this;
}

ICQBuffer &ICQBuffer::operator << (unsigned short c)
{
    c = htons(c);
    pack((char*)&c, 2);
    return *this;
}

ICQBuffer &ICQBuffer::operator << (unsigned long c)
{
    /* XXX:
     * WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    c = htonl(c);
    pack((char*)&c, 4);
    return *this;
}

ICQBuffer &ICQBuffer::operator << (bool b)
{
    char c = b ? (char)1 : (char)0;
    pack(&c, 1);
    return *this;
}

ICQBuffer &ICQBuffer::operator >> (std::string &s)
{
    unsigned short size;
    *this >> size;
    size = htons(size);
    s.erase();
    if (size){
        if (size > this->size() - m_posRead)
            size = (unsigned short)(this->size() - m_posRead);
        s.append((unsigned)size, '\x00');
        unpack((char*)s.c_str(), size);
    }
    return *this;
}

ICQBuffer &ICQBuffer::operator >> (QByteArray &str)
{
    unsigned short s;
    str = "";

    *this >> s;
    s = htons(s);
    if (s == 0)
        return *this;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    unpack(str, s);
    return *this;
}

ICQBuffer &ICQBuffer::operator >> (char &c)
{
    if (unpack(&c, 1) != 1)
        c = 0;
    return *this;
}

ICQBuffer &ICQBuffer::operator >> (unsigned short &c)
{
    if (unpack((char*)&c, 2) != 2)
        c = 0;
    c = ntohs(c);
    return *this;
}

ICQBuffer &ICQBuffer::operator >> (unsigned long &c)
{
    /* XXX:
     * WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    if (unpack((char*)&c, 4) != 4)
        c = 0;
    c = ntohl(c);
    return *this;
}

ICQBuffer &ICQBuffer::operator >> (int &c)
{
    if (unpack((char*)&c, 4) != 4)
        c = 0;
    c = ntohl(c);
    return *this;
}

void ICQBuffer::packScreen(const QString &screen)
{
    char len = screen.toUtf8().length();
    pack(&len, 1);
    pack(screen.toUtf8(), len);
}

void ICQBuffer::packStr32(const char *s)
{
    if (s) {
        unsigned long size = strlen(s);
        pack(size);
        pack(s, strlen(s));
    } else {
        pack((unsigned long)0);
        pack("", 0);
    }
}

void ICQBuffer::packStr32(const QByteArray &s)
{
    unsigned long size = s.length();
    pack(size);
    pack(s, size);
}

void ICQBuffer::pack32(const Buffer &b)
{
    /* XXX:
     * WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    unsigned long size = b.size() - b.readPos();
    *this << (unsigned long)htonl(size);
    pack(b.data(b.readPos()), size);
}

void ICQBuffer::pack(const QByteArray &s)
{
    unsigned short size = (unsigned short)(s.size());
    *this << size;
    pack(s, size);
}

void ICQBuffer::pack(const QString &s)
{
    QByteArray cstr = s.toUtf8();
	unsigned short size = (unsigned short)(s.length());
    *this << size;
    pack(cstr, size);
}

void ICQBuffer::pack(unsigned short s)
{
    SWAP_S(s);
    pack((char*)&s, 2);
}

void ICQBuffer::pack(unsigned long s)
{
    /* XXX:
     * WARNING! BUG HERE. sizeof(long) is not 4 on 64bit platform */
    unsigned int i = s;
    SWAP_L(i);
    pack((char*)&i, 4);
}
bool ICQBuffer::unpackStr(QString &str)
{
    unsigned short s;
    str = QString::null;
    *this >> s;
    if (s == 0)
        return false;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    unpack(str, s);
    return true;
}

bool ICQBuffer::unpackStr(QByteArray &str)
{
    unsigned short s;
    str = "";
    *this >> s;
    if (s == 0)
        return false;
    if (s > size() - m_posRead)
        s = (unsigned short)(size() - m_posRead);
    unpack(str, s);
    return true;
}

void ICQBuffer::unpackStr32(std::string &s)
{
    unsigned long size;
    *this >> size;
    size = htonl(size);
    s.erase();
    if (size == 0) return;
	if (size > this->size() - m_posRead)
        size = this->size() - m_posRead;
    s.append(size, '\x00');
    unpack((char*)s.c_str(), size);
}

bool ICQBuffer::unpackStr32(QByteArray &str)
{
    unsigned long s;
    *this >> s;
    s = ntohl(s);
    str = "";
    if (s == 0)
        return false;
    if (s > size() - m_posRead)
        s = size() - m_posRead;
    unpack(str, s);
    return true;
}

/*bool ICQBuffer::unpackStr32(QByteArray &str)
{
    unsigned long s;
    *this >> s;
    s = ntohl(s);
    str = QByteArray();
    if (s == 0)
        return false;
    if (s > size() - m_posRead)
        s = size() - m_posRead;
    unpack(str, s);
    return true;
}*/

QString ICQBuffer::unpackScreen()
{
    char len;
    QString res;

    *this >> len;
    /* 13 isn't right, AIM allows 16. But when we get a longer
    name, we *must* unpack them if we won't lose the TLVs
    behind the Screenname ... */
    if (len > 16)
        log(L_DEBUG,"Too long Screenname! Length: %d",len);
    unpack(res, len);
    return res;
}

unsigned ICQBuffer::unpack(char *d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    memcpy(d, data() + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

unsigned ICQBuffer::unpack(QString &d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    d = QString::fromUtf8(data() + m_posRead, readn);
    m_posRead += readn;
    return readn;
}

/*unsigned ICQBuffer::unpack(QByteArray &d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    d = QByteArray(data() + m_posRead, readn + 1);
    m_posRead += readn;
    return readn;
}*/

unsigned ICQBuffer::unpack(QByteArray &d, unsigned s)
{
    unsigned readn = size() - m_posRead;
    if (s < readn)
        readn = s;
    d = QByteArray( data() + m_posRead, readn );
    unsigned size = d.size();
    d.resize(size);
    m_posRead += readn;
	if(d.endsWith((char)0))
	{
		d.chop(1);
	}
    return readn;
}

void ICQBuffer::unpack(unsigned short &c)
{
    if (unpack((char*)&c, 2) != 2)
        c = 0;
    SWAP_S(c);
}

void ICQBuffer::unpack(unsigned long &c)
{
    // FIXME: This needs to be rewritten for 64-bit machines.
    // Kludge for now.
    unsigned int i;
    if (unpack((char*)&i, 4) != 4)
        i = 0;
    SWAP_L(i);
    c = i;
}

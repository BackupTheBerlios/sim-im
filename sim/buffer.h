/***************************************************************************
                          buffer.h  -  description
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

#ifndef _BUFFER_H
#define _BUFFER_H

#include <QByteArray>
#include <QString>

#include "simapi.h"

#ifdef HAVE_STDINT_H
	#include <stdint.h>
#else
	typedef signed		char	int8_t;
	typedef signed		short	int16_t;
	typedef signed		long	int32_t;
	typedef unsigned	char	uint8_t;
	typedef unsigned	short	uint16_t;
	typedef unsigned	long	uint32_t; 
#endif

class EXPORT Buffer : public QByteArray
{
public:
    Buffer(unsigned size = 0);
    Buffer(const QByteArray &ba);
    virtual ~Buffer();

    bool add(uint size);
    bool resize(uint size);
    unsigned readPos() const { return m_posRead; }
    void incReadPos(int size);
    void decReadPos(int size) { incReadPos(-size); }
    unsigned writePos() const { return m_posWrite; }
    void setWritePos(unsigned size);
    void setReadPos(unsigned size);

    const char* data(unsigned pos = 0) const { return QByteArray::data() + pos; }
    char* data(unsigned pos = 0) { return (char*)QByteArray::data() + pos; }

    void packetStart();
    unsigned long packetStartPos() const { return m_packetStartPos; }

    // just the normal pack/unpack routines
    void pack(char c)          { *this << c; }
    void pack(unsigned char c) { *this << c; }
    void pack(const char *d, unsigned size);

    void unpack(char &c)          { *this >> c; }
    void unpack(unsigned char &c) { *this >> c; }
    unsigned unpack(char *d, unsigned size);

    Buffer &operator << (char c);
    Buffer &operator << (unsigned char c) { return operator << ((char)c); }
    Buffer &operator << (const char *str);
    Buffer &operator << (unsigned short c);
    Buffer &operator << (long c);
    Buffer &operator << (unsigned long c) { return operator << ((long)c); }

    Buffer &operator >> (char &c);
    Buffer &operator >> (unsigned char &c) { return operator >> ((char&)c); }
    Buffer &operator >> (unsigned short &c);
    Buffer &operator >> (long &c);
    Buffer &operator >> (unsigned long &c);// { return operator >> ((long&)c); }
#ifndef WIN32
    Buffer &operator >> (uint32_t &c);
#endif

    bool scan(const char *substr, QByteArray &res);

    void init(unsigned size);

    QByteArray	getSection(bool bSkip=false);
    unsigned	startSection() { return m_startSection; }
    QByteArray	getLine();

    // still needed for msn until it has an ownbuffer too
    unsigned unpack(QString &d, unsigned size); // utf8
protected:
    unsigned unpack(QByteArray &d, unsigned size);

    unsigned m_packetStartPos;
    unsigned m_posRead;
    unsigned m_posWrite;
    unsigned m_startSection;
};

#endif


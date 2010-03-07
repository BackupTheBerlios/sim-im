/***************************************************************************
                          event.h  -  description
                             -------------------
    begin                : Sat Oct 28 2006
    copyright            : (C) 2006 by Christian Ehrlicher
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

// system includes
#include <stdio.h>
#include <QMultiMap>
#include <QDateTime>

// own header
#include "event.h"
// local includes
#include "log.h"
#include "misc.h"   // sprintf

namespace SIM
{

// this is all not thread-safe!
static QMultiMap<unsigned, EventReceiver*> receivers;
static bool g_bChanged = false;
static int g_iLevel = 0;

EventReceiver::EventReceiver(unsigned priority)
{
    m_priority = priority;
    receivers.insert(priority, this);
    g_bChanged = true;
}

EventReceiver::~EventReceiver()
{
    receivers.remove(m_priority, this);
    g_bChanged = true;
}

/*********************
 * class Event
 *********************/
Event::~Event()
{
    if(!m_bProcessed)
        log(L_ERROR, "Event::~Event() without call to Event::process(), Event: 0x%04x", m_type);
}

bool Event::process(EventReceiver *from)
{
    m_bProcessed = true;

    g_iLevel++;
    QMultiMap<unsigned, EventReceiver*>::ConstIterator it = receivers.begin();
    QMultiMap<unsigned, EventReceiver*>::ConstIterator end = receivers.constEnd();
    if (from){
        it = receivers.find(from->priority(), from);
        if(it == end)
            return false;
        ++it;
    }
    for (; it != end; ++it){
        EventReceiver *receiver = *it;
        if (receiver) {
            bool res = receiver->processEvent(this);
            if (res) {
                g_iLevel--;
                if(g_iLevel == 0 && g_bChanged)
                    g_bChanged = false;
                return res;
            }
            if(g_bChanged) {
                // adjust
                if(!receivers.contains(receiver->priority(), receiver))
                    return false;
            }
        }
    }
    g_iLevel--;
    if(g_iLevel == 0 && g_bChanged)
        g_bChanged = false;
    return false;
}

void EventReceiver::initList()
{
}

void EventReceiver::destroyList()
{
    receivers.clear();
}

// Some event functions
// ********************
// EventLog
// ********************
QString EventLog::make_packet_string(const EventLog &l)
{
    QString m;
    if (l.isPacketLog())
	{
        PacketType *type = getContacts()->getPacketType(l.packetID());
        if (type == NULL)
            return m;
        const Buffer &b = l.buffer();
        unsigned start = b.packetStartPos();
        QDateTime now(QDateTime::currentDateTime());
        QString name = type->name();
        if (!l.additionalInfo().isEmpty()){
            name += '.';
            name += l.additionalInfo();
        }
        m.sprintf("%s [%s] %s %u bytes\n",
               now.toString("yyyy-MM-dd hh:mm:ss").toLatin1().data(),
               name.toLatin1().data(),
               (l.logLevel() & L_PACKET_IN) ? "Read" : "Write",
               b.size() - start);
        if (type->isText()){
            m += QString::fromLatin1(b.data(start), b.size() - start);
        }else{
            char line[81];
            char *p1 = line;
            char *p2 = line;
            unsigned n = 20;
            unsigned offs = 0;
            for (unsigned i = start; i < (unsigned)b.size(); i++, n++){
                char buf[32];
                if (n == 16){
                    m += line;
                    m += '\n';
                }
                if (n >= 16){
                    memset(line, ' ', 80);
                    line[80] = 0;
                    snprintf(buf, sizeof(buf), "%04X: ", offs);
                    memcpy(line, buf, strlen(buf));
                    p1 = line + strlen(buf);
                    p2 = p1 + 52;
                    n = 0;
                    offs += 0x10;
                }
                if (n == 8)
                    p1++;
                unsigned char c = (unsigned char)*(b.data(i));
                *(p2++) = ((c >= ' ') && (c != 0x7F)) ? c : '.';
                snprintf(buf, sizeof(buf), "%02X ", c);
                memcpy(p1, buf, 3);
                p1 += 3;
            }
            if (n <= 16)
                m += line;
        }
    }
	else
	{
        m = l.logData();
    }
    return m;
}

/*
void EventLog::log_packet(const Buffer &packetBuf, bool bOut, unsigned packetID, const QString addInfo)
{
    EventLog e(packetBuf, bOut, packetID, addInfo);
    e.process();
}
*/

void EventLog::log_packet(const Buffer &packetBuf, bool bOut, unsigned packetID, const QString addInfo)
{
    EventLog e(packetBuf, bOut, packetID, addInfo);
    e.process();
}

}   // namespace SIM

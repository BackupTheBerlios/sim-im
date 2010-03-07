/***************************************************************************
                          history.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _HISTORY_H
#define _HISTORY_H

#include <map>
#include <list>
#include <QString>
#include <QByteArray>

#include "message.h"

class CorePlugin;
class QFile;

class HistoryFile;
class HistoryFileIterator;

using namespace std;

struct msg_save
{
    QByteArray	msg;
    QString     client;
    unsigned    contact;
};

typedef map<unsigned, msg_save>	MAP_MSG;

class History
{
public:
    History(unsigned contact_id);
    ~History();
    static void add(SIM::Message*, const QString &type);
    static void del(SIM::Message*);
    static void rewrite(SIM::Message*);
    static void cut(SIM::Message*, unsigned contact_id, unsigned date);
    static void del(unsigned msg_id);
    static void remove(SIM::Contact *contact);
    static bool save(unsigned id, const QString& file_name, bool bAppend = false);
    static SIM::Message *load(unsigned id, const QString &client, unsigned contact);
protected:
    static void del(const QString &name, unsigned contact, unsigned id, bool bCopy, SIM::Message *msg=NULL);
    static unsigned	s_tempId;
    static MAP_MSG	*s_tempMsg;
    unsigned m_contact;
    list<HistoryFile*> files;
    friend class HistoryIterator;
};

class HistoryIterator
{
public:
    HistoryIterator(unsigned contact_id);
    ~HistoryIterator();
    SIM::Message *operator++();
    SIM::Message *operator--();
    void begin();
    void end();
    QString state();
    void setState(const QString &);
    void setFilter(const QString &filter);
protected:
    bool m_bUp;
    bool m_bDown;
    unsigned m_temp_id;
    History m_history;
    HistoryFileIterator *m_it;
    list<HistoryFileIterator*> iters;
};

#endif


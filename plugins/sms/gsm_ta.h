/***************************************************************************
                          gsm_ta.h  -  description
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

#ifndef _GSM_TA_H
#define _GSM_TA_H

#include <QByteArray>
#include <QObject>
#include <string>
#include <list>
#include <vector>
#include "simapi.h"

using namespace std;

class SerialPort;
class QTimer;

struct OpInfo
{
    unsigned	oper;
    string	param;
};

class Phonebook
{
public:
    Phonebook();
    unsigned m_size;
    unsigned m_used;
    unsigned m_numberSize;
    unsigned m_nameSize;
    vector<bool> m_entries;
};

class GsmTA : public QObject
{
    Q_OBJECT
public:
    GsmTA(QObject *parent);
    ~GsmTA();
    bool open(const char *device, int baudrate, bool bXonXoff);
    QByteArray model() const;
    QByteArray oper() const;
    void getPhoneBook();
    void setPhoneBookEntry(unsigned index, const QString &phone, const QString &name);
signals:
    void init_done();
    void error();
    void phoneCall(const QString &phone);
    void phonebookEntry(int index, int type, const QString &phone, const QString &name);
    void quality(unsigned);
    void charge(bool, unsigned);
protected slots:
    void write_ready();
    void read_ready();
    void port_error();
    void ping();
protected:
    enum State
    {
        None,
        Open,
        Init,
        Init2,
        Init3,
        Init4,
        Init5,
        Init6,
        Init7,
        Info1,
        Info2,
        Info3,
        Info4,
        Charset1,
        Charset2,
        OpInfo1,
        OpInfo2,
        Connected,
        Ping,
        PhoneBook,
        PhoneBook1,
        PhoneBook2,
        PhoneBook3,
        PhoneBookStore
    };
    unsigned m_tries;
    void at(const QByteArray &str, unsigned timeout=10000);
    bool isOK(const QByteArray &answer);
    bool isError(const QByteArray &answer);
    bool isChatOK(const QByteArray &answer, const char *response = NULL,
                  bool bIgnoreErrors = false, bool bAcceptEmptyResponse = false);
    bool isChatResponse(const QByteArray &answer, const char *response = NULL,
                        bool bIgnoreErrors = false);
    bool isIncoming(const QByteArray &answer);
    bool matchResponse(QByteArray &answer, const char *responseToMatch);
    void processQueue();
    void parseEntriesList(const QByteArray &answ);
    void parseEntry(const QByteArray &answ);
    void getNextEntry();
    QByteArray normalize(const QByteArray &answ);
    QByteArray gsmToLatin1(const QByteArray &str);
    QByteArray latin1ToGsm(const QByteArray &str);
    State               m_state;
    QByteArray		m_cmd;
    QByteArray		m_manufacturer;
    QByteArray		m_model;
    QByteArray		m_revision;
    QByteArray		m_serialNumber;
    QByteArray		m_operator;
    QByteArray		m_response;
    QByteArray		m_charset;
    list<OpInfo>m_queue;
    Phonebook		m_books[2];
    Phonebook          *m_book;
    bool                m_bPing;
    unsigned		m_bookType;
    QTimer             *m_timer;
    SerialPort		*m_port;
};

#endif


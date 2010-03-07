/***************************************************************************
                          gsm_ta.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru

    Based on gsmlib by Peter Hofmann (software@pxh.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QTimer>
#include <QByteArray>

#include <ctype.h>

#include "log.h"

#include "gsm_ta.h"
#include "serial.h"
#include "sms.h"

using namespace std;
using namespace SIM;

const unsigned	PING_TIMEOUT	= 20000;
const unsigned	UNKNOWN			= (unsigned)(-1);

const unsigned	OP_PHONEBOOK		= 0;
const unsigned	OP_PHONEBOOK_ENTRY	= 1;

GsmTA::GsmTA(QObject *parent)
        : QObject(parent)
{
    m_state = None;
    m_bPing = false;
    m_port  = new SerialPort(this);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(ping()));
    connect(m_port, SIGNAL(write_ready()), this, SLOT(write_ready()));
    connect(m_port, SIGNAL(read_ready()), this, SLOT(read_ready()));
    connect(m_port, SIGNAL(error()), this, SLOT(port_error()));
}

GsmTA::~GsmTA()
{
}

bool GsmTA::open(const char *device, int baudrate, bool bXonXoff)
{
    m_state      = Open;
    return m_port->openPort(device, baudrate, bXonXoff, 100);
}

void GsmTA::write_ready()
{
    switch (m_state){
    case Open:
        at("Z");
        m_state = Init;
        break;
    default:
        break;
    }
}

void GsmTA::read_ready()
{
    QByteArray line = m_port->readLine();
    if (!line.isEmpty() && (line[(int)line.length() - 1] == '\r'))
        line = line.left(line.length() - 1);
    if (!line.isEmpty()){
        Buffer b(line);
        EventLog::log_packet(b, false, SMSPlugin::SerialPacket);
    }
    OpInfo	 opInfo;
    unsigned value1;
    unsigned value2;
    switch (m_state){
    case Init:
        if (!isOK(line))
            return;
        m_state = Init2;
        at("E0");
        break;
    case Init2:
        if (!isOK(line))
            return;
        m_state = Init3;
        at("+CMEE=1");
        break;
    case Init3:
        if (!isChatOK(line, "", true, true))
            return;
        m_state = Init4;
        at("+CMGF=0");
        break;
    case Init4:
        if (!isChatOK(line))
            return;
        m_state = Init5;
        at("+CLIP=1");
        break;
    case Init5:
        if (!isChatOK(line))
            return;
        m_state = Init6;
        at("+CBC");
        break;
    case Init6:
        if (!isChatResponse(line, "+CBC:", false))
            return;
        value1 = normalize(getToken(m_response, ',')).toUInt();
        value2 = normalize(m_response).toUInt();
        emit charge(value1 != 0, value2);
        m_state = Init7;
        at("+CSQ");
        break;
    case Init7:
        if (!isChatResponse(line, "+CSQ:", false))
            return;
        value1 = normalize(m_response).toUInt();
        if (value1 == 99)
            value1 = 0;
        emit quality(value1);
        m_state = Info1;
        at("+CGMI");
        break;
    case Info1:
        if (!isChatResponse(line, "+CGMI:", false))
            return;
        m_manufacturer = m_response;
        m_state = Info2;
        at("+CGMM");
        break;
    case Info2:
        if (!isChatResponse(line, "+CGMM:", false))
            return;
        m_model = m_response;
        m_state = Info3;
        at("+CGMR");
        break;
    case Info3:
        if (!isChatResponse(line, "+CGMR:", false))
            return;
        m_revision = m_response;
        m_state = Info4;
        at("+CGSN");
        break;
    case Info4:
        if (!isChatResponse(line, "+CGSN:", false))
            return;
        m_serialNumber = m_response;
        m_state = Charset1;
        at("+CSCS=\"UCS2\"");
        break;
    case Charset1:
        if (!isChatResponse(line, "+CSCS:", false))
            return;
        m_state = Charset2;
        at("+CSCS?");
        break;
    case Charset2:
        if (!isChatResponse(line, "+CSCS:", false))
            return;
        m_charset = normalize(m_response);
        if (m_charset.length() && (m_charset[0] == '\"')){
            getToken(m_charset, '\"');
            m_charset = getToken(m_charset, '\"');
        }
        m_state = OpInfo1;
        at("+COPS=3,0");
        break;
    case OpInfo1:
        if (!isChatOK(line, "", true, true))
            return;
        m_state = OpInfo2;
        at("+COPS?");
        break;
    case OpInfo2:
        if (!isChatResponse(line, "+COPS:", false))
            return;
        getToken(m_response, ',');
        getToken(m_response, ',');
        if (m_response.length() && (m_response[0] == '"')){
            getToken(m_response, '\"');
            m_operator = getToken(m_response, '\"');
        }
        m_port->setTimeout(~0U);
        m_state = Connected;
        emit init_done();
        processQueue();
        break;
    case Connected:
        if (isError(line))
            return;
        break;
    case Ping:
        if (m_bPing){
            if (!isChatResponse(line, "+CBC:", false))
                return;
            value1 = normalize(getToken(m_response, ',')).toUInt();
            value2 = normalize(m_response).toUInt();
            emit charge(value1 != 0, value2);
            m_bPing = false;
        }else{
            if (!isChatResponse(line, "+CSQ:", false))
                return;
            value1 = normalize(m_response).toUInt();
            if (value1 == 99)
                value1 = 0;
            emit quality(value1);
            m_bPing = true;
        }
        m_port->setTimeout((unsigned)(-1));
        m_state = Connected;
        processQueue();
        break;
    case PhoneBook:
        if (!isChatOK(line, "", true, true))
            return;
        m_state	= PhoneBook1;
        at("+CPBS?");
        break;
    case PhoneBook1:
        if (!isChatResponse(line, "+CPBS:"))
            return;
        m_state = PhoneBook2;
        getToken(m_response, ',');
        m_book->m_used = normalize(getToken(m_response, ',')).toUInt();
        m_book->m_size = normalize(getToken(m_response, ',')).toUInt();
        at("+CPBR=?");
        break;
    case PhoneBook2:
        if (!isChatResponse(line, "+CPBR:"))
            return;
        parseEntriesList(getToken(m_response, ','));
        if (m_book->m_entries.empty() && m_book->m_size){
            for (unsigned i = 0; i < m_book->m_size; i++)
                m_book->m_entries.push_back(true);
        }
        m_book->m_size = 0;
        m_book->m_numberSize = getToken(m_response, ',').toUInt();
        m_book->m_nameSize   = m_response.toUInt();
        if (m_queue.empty()){
            getNextEntry();
            break;
        }
        opInfo.oper = OP_PHONEBOOK_ENTRY;
        m_queue.push_back(opInfo);
        m_state = Connected;
        processQueue();
        break;
    case PhoneBook3:
        if (!isChatResponse(line, "+CPBR:"))
            return;
        parseEntry(m_response);
        if (m_queue.empty()){
            getNextEntry();
            break;
        }
        opInfo.oper = OP_PHONEBOOK_ENTRY;
        m_queue.push_back(opInfo);
        m_state = Connected;
        processQueue();
        break;
    default:
        break;
    }
}

void GsmTA::ping()
{
    if (m_state != Connected)
        return;
    m_timer->stop();
    m_state = Ping;
    if (m_bPing){
        at("+CBC");
    }else{
        at("+CSQ");
    }
}

void GsmTA::port_error()
{
    m_timer->stop();
    emit error();
}

void GsmTA::at(const QByteArray &str, unsigned timeout)
{
    QByteArray cmd = "AT";
    cmd += str;
    m_cmd = cmd;
    Buffer b(cmd);
    EventLog::log_packet(b, true, SMSPlugin::SerialPacket);
    cmd += "\r\n";
    m_tries = 5;
    m_response = "";
    m_port->writeLine(cmd.data(), timeout);
}

bool GsmTA::isOK(const QByteArray &answer)
{
    if (isIncoming(answer))
        return false;
    if (answer == "OK" || answer.contains("CABLE: GSM"))
        return true;
    if (--m_tries == 0)
        emit error();
    return false;
}

QByteArray GsmTA::normalize(const QByteArray &ans)
{
    QByteArray answer = ans;
    size_t start = 0, end = answer.length();
    bool changed = true;
    while (start < end && changed){
        changed = false;
        if (isspace(answer[(int)start])){
            ++start;
            changed = true;
        }else if (isspace(answer[(int)end - 1])){
            --end;
            changed = true;
        }
    }
    answer = answer.mid(start, end - start);
    return answer;
}

bool GsmTA::isError(const QByteArray &ans)
{
    if (isIncoming(ans))
        return false;
    QByteArray answer = normalize(ans);
    if (answer.isEmpty())
        return false;
    if (matchResponse(answer, "+CME ERROR:") ||
            matchResponse(answer, "+CMS ERROR:") ||
            matchResponse(answer, "ERROR")){
        emit error();
        return true;
    }
    return false;
}

bool GsmTA::isChatOK(const QByteArray &ans, const char *response,  bool bIgnoreErrors, bool bAcceptEmptyResponse)
{
    if (isIncoming(ans))
        return false;
    QByteArray answer = normalize(ans);
    if (answer.isEmpty() || (answer == m_cmd))
        return false;
    if (matchResponse(answer, "+CME ERROR:") ||
            matchResponse(answer, "+CMS ERROR:") ||
            matchResponse(answer, "ERROR")){
        if (bIgnoreErrors)
            return true;
        emit error();
        return false;
    }
    if (bAcceptEmptyResponse && (answer == "OK"))
        return true;
    if (response == NULL){
        if (answer == "OK")
            return true;
        log(L_DEBUG, "Unexpected answer %s", answer.data());
        emit error();
        return false;
    }
    if (matchResponse(answer, response))
        return true;
    log(L_DEBUG, "Unexpected answer %s", answer.data());
    emit error();
    return false;
}

bool GsmTA::isChatResponse(const QByteArray &ans, const char *response, bool bIgnoreErrors)
{
    if (isIncoming(ans))
        return false;
    QByteArray answer = normalize(ans);
    if (answer.isEmpty() || (answer == m_cmd))
        return false;
    if (matchResponse(answer, "+CME ERROR:") ||
            matchResponse(answer, "+CMS ERROR:") ||
            matchResponse(answer, "ERROR")){
        if (bIgnoreErrors)
            return true;
        emit error();
        return false;
    }
    if (answer == "OK")
        return true;
    if (answer.isEmpty())
        return false;
    matchResponse(answer, response);
    if (!m_response.isEmpty())
        m_response += "\n";
    m_response += answer;
    return false;
}

bool GsmTA::isIncoming(const QByteArray &ans)
{
    QByteArray answer = normalize(ans);
    if (matchResponse(answer, "+CLIP:")){
        QString number = getToken(answer, ',');
        if (!number.isEmpty() && (number[0] == '\"')){
            getToken(number, '\"');
            number = getToken(number, '\"');
        }
        unsigned type = answer.toUInt();
        if (type)
            emit phoneCall(number);
        return true;
    }
    return false;
}

bool GsmTA::matchResponse(QByteArray &answer, const char *responseToMatch)
{
    if (answer.left(strlen(responseToMatch)) == responseToMatch){
        answer = normalize(answer.data() + strlen(responseToMatch));
        return true;
    }
    return false;
}

QByteArray GsmTA::model() const
{
    QByteArray res = m_manufacturer;
    if (!m_model.isEmpty()){
        if (!res.isEmpty())
            res += " ";
        res += m_model;
    }
    return res;
}

QByteArray GsmTA::oper() const
{
    return m_operator;
}

void GsmTA::processQueue()
{
    if (m_queue.empty()){
        m_timer->setSingleShot( true );
        m_timer->start( PING_TIMEOUT );
        return;
    }
    m_timer->stop();
    list<OpInfo>::iterator it = m_queue.begin();
    OpInfo info = *it;
    m_queue.erase(it);
    switch (info.oper){
    case OP_PHONEBOOK:
        getPhoneBook();
        break;
    case OP_PHONEBOOK_ENTRY:
        getNextEntry();
        break;
    default:
        log(L_DEBUG, "Unknown oper");
        break;
    }
}

void GsmTA::getNextEntry()
{
    for (; m_book->m_size < m_book->m_entries.size(); m_book->m_size++){
        if (!m_book->m_entries[m_book->m_size])
            continue;
        m_state = PhoneBook3;
        QString cmd = "+CPBR=";
        cmd += QString::number(m_book->m_size);
        at(cmd.toLatin1(), 20000);
        m_book->m_size++;
        return;
    }
    if (m_bookType == 0){
        m_bookType = 1;
        m_state = PhoneBook;
        m_book  = m_books + 1;
        at("+CPBS=ME");
        return;
    }
    m_port->setTimeout((unsigned)(-1));
    m_state = Connected;
    processQueue();
}

void GsmTA::parseEntry(const QByteArray &answ)
{
    QByteArray answer = normalize(answ);
    unsigned index = getToken(answer, ',').toUInt();
    answer = normalize(answer);
    if (answer.isEmpty())
        return;
    QByteArray phone;
    if (answer[0] == '\"'){
        getToken(answer, '\"');
        phone = getToken(answer, '\"');
        getToken(answer, ',');
    }else{
        phone = getToken(answer, ',');
    }
    if (phone.isEmpty() || (phone == "EMPTY"))
        return;
    answer = normalize(answer);
    getToken(answer, ',');
    answer = normalize(answer);
    QByteArray name;
    if (answer[0] == '\"'){
        getToken(answer, '\"');
        name = getToken(answer, '\"');
    }else{
        name = getToken(name, ',');
    }
    QString nameString;
    if (m_charset == "UCS2"){
        for (; name.length() >= 4; ){
            unsigned short unicode = (fromHex(name[0]) << 12) +
                                     (fromHex(name[1]) << 8) +
                                     (fromHex(name[2]) << 4) +
                                     fromHex(name[3]);
            name = name.mid(4);
            nameString += QChar(unicode);
        }
    }else if (m_charset == "GSM"){
        nameString = gsmToLatin1(name);
    }else{
        nameString = name;
    }
    if (nameString.isEmpty())
        return;
    emit phonebookEntry(index, m_bookType, phone, nameString);
}

void GsmTA::getPhoneBook()
{
    if (m_state != Connected){
        OpInfo info;
        info.oper = OP_PHONEBOOK;
        m_queue.push_back(info);
        return;
    }
    m_bookType = 0;
    m_timer->stop();
    m_state = PhoneBook;
    m_book = m_books;
    at("+CPBS=SM");
}

void GsmTA::parseEntriesList(const QByteArray &str)
{
    for (int i = 0; i < str.length(); i++){
        char c = str[i];
        if ((c >= '0') && (c <= '9')){
            unsigned n = c - '0';
            unsigned n1 = 0;
            for (i++; str[i]; i++){
                c = str[i];
                if ((c < '0') || (c >= '9'))
                    break;
                n = (n * 10) + (c - '0');
            }
            if (str[i] == '-'){
                for (i++; str[i]; i++){
                    c = *str;
                    if ((c < '0') || (c >= '9'))
                        break;
                    n1 = (n1 * 10) + (c - '0');
                }
            }else{
                n1 = n;
            }
            if (n1 >= n){
                for (; n <= n1; n++){
                    while (m_book->m_entries.size() <= n)
                        m_book->m_entries.push_back(false);
                    m_book->m_entries[n] = true;
                }
            }
        }
    }
}

Phonebook::Phonebook()
{
    m_size			= 0;
    m_used			= 0;
    m_numberSize	= 0;
    m_nameSize		= 0;
}

// Latin-1 undefined character (code 172 (Latin-1 boolean not, "¬"))
const unsigned char NOP = 172;

// GSM undefined character (code 16 (GSM Delta))
const unsigned char GSM_NOP = 16;

static unsigned char gsmToLatin1Table[] =
    {
        '@', 163, '$', 165, 232, 233, 249, 236,
        242, 199,  10, 216, 248,  13, 197, 229,
        NOP, '_', NOP, NOP, NOP, NOP, NOP, NOP,
        NOP, NOP, NOP, NOP, 198, 230, 223, 201,
        ' ', '!', '"', '#', 164, '%', '&', '\'',
        '(', ')', '*', '+', ',', '-', '.', '/',
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', ':', ';', '<', '=', '>', '?',
        161, 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', 196, 214, 209, 220, 167,
        191, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
        'x', 'y', 'z', 228, 246, 241, 252, 224
    };

class GsmLatin1
{
public:
    GsmLatin1();
    unsigned char latin1ToGsmTable[256];
};

GsmLatin1::GsmLatin1()
{
    memset(latin1ToGsmTable, GSM_NOP, 256);
    for (unsigned char i = 0; i < 128; i++){
        if (gsmToLatin1Table[i] == NOP)
            continue;
        latin1ToGsmTable[gsmToLatin1Table[i]] = i;
    }
}

static GsmLatin1 gsmTable;

QByteArray GsmTA::gsmToLatin1(const QByteArray &str)
{
    QByteArray res;
    for (unsigned char *p = (unsigned char*)str.data(); *p; p++){
        if (*p >= 0x80)
            continue;
        unsigned char c = gsmToLatin1Table[*p];
        if (c == NOP)
            continue;
        res += (char)c;
    }
    return res;
}

QByteArray GsmTA::latin1ToGsm(const QByteArray &str)
{
    QByteArray res;
    for (unsigned char *p = (unsigned char*)str.data(); *p; p++){
        unsigned char c = gsmTable.latin1ToGsmTable[*p]; //warning C6385: Invalid data: accessing 'gsmTable.latin1ToGsmTable', the readable size is '256' bytes, but '1001' bytes might be read: Lines: 671, 672, 673
        if (c == GSM_NOP)
            continue;
        res += (char)c;
    }
    return res;
}


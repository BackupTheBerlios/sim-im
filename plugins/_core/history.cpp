/***************************************************************************
history.cpp  -  description
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

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegExp>
#include <QTextCodec>
#include <QTextStream>
#include <QByteArray>

#include <time.h>

#include "simapi.h"
#include "buffer.h"
#include "log.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#include "history.h"
#include "core.h"
#include "msgview.h"

using namespace std;
using namespace SIM;

static char HISTORY_PATH[] = "history/";
static char REMOVED[] = ".removed";

const unsigned CUT_BLOCK		= 0x4000;
const unsigned LOAD_BLOCK_SIZE	= 0x1000;
const unsigned BLOCK_SIZE		= 0x4000;
const unsigned TEMP_BASE		= 0x80000000;

unsigned History::s_tempId		= TEMP_BASE;
MAP_MSG  *History::s_tempMsg	= NULL;

class HistoryFile : public QFile
{
public:
    HistoryFile(const QString &name, unsigned contact);
    bool isOpen() { return handle() != -1; }
    QString     m_name;
    unsigned    m_contact;
    Message *load(unsigned id);
private:
    HistoryFile(const HistoryFile&);
    void operator = (const HistoryFile&);
};

class HistoryFileIterator
{
public:
    HistoryFileIterator(HistoryFile&, unsigned contact);
    ~HistoryFileIterator();
    void createMessage(unsigned id, const char *type, Buffer *cfg);
    void begin();
    void end();
    void clear();
    Message *operator++();
    Message *operator--();
    Message *message();
    HistoryFile     &file;
    list<Message*>  msgs;
    int             m_block;
    Message         *m_msg;
    bool            loadBlock(bool bUp);
    QString         m_filter;
private:
    unsigned        m_contact;
    QTextCodec      *m_codec;
    HistoryFileIterator(const HistoryFileIterator&);
    void operator = (const HistoryFileIterator&);
};

static Message *createMessage(unsigned id, const char *type, Buffer *cfg)
{
    if (type == NULL || *type == 0)
        return NULL;
    Message *msg = CorePlugin::instance()->createMessage(type, cfg);
    if (!msg)
        return NULL;

    msg->setId(id);
    return msg;
}

HistoryFile::HistoryFile(const QString &file_name, unsigned contact)
    : m_name    (file_name)
    , m_contact (contact)
{
    QString f_name = HISTORY_PATH;
    f_name += file_name.isEmpty() ? QString::number(contact) : file_name;
    f_name = user_file(f_name);
    log(L_DEBUG, "FNAME: %s", f_name.toUtf8().data());
    setFileName(f_name);
    QFileInfo fi(*this);
    if (!fi.exists()) 
        // make sure directory exists
        makedir(fi.absolutePath() + '/');
    else if (!fi.isFile()) 
        // FIXME!
        log(L_ERROR, "%s is not a file!", qPrintable(fi.filePath()));
    QFile bak(fileName().append(REMOVED));
    if (!exists() && bak.exists())
    {
        QFileInfo fInfo(fileName());
        fInfo.dir().rename(bak.fileName(), fileName());
    }
    open(QIODevice::ReadOnly);
}

Message *HistoryFile::load(unsigned id)
{
    if (!seek(id))
        return NULL;
    Buffer cfg;
    cfg = readAll();
    QByteArray type = cfg.getSection();
    Message *msg = CorePlugin::instance()->createMessage(type, &cfg);
    if (msg == NULL)
        return NULL;
    msg->setId(id);
    msg->setClient(m_name);
    msg->setContact(m_contact);
    return msg;
}

HistoryFileIterator::HistoryFileIterator(HistoryFile &f, unsigned contact)
: file(f)
, m_block(0)
, m_msg(NULL)
, m_contact(contact)
, m_codec(NULL)
{
}

HistoryFileIterator::~HistoryFileIterator()
{
    clear();
}

void HistoryFileIterator::createMessage(unsigned id, const char *type, Buffer *cfg)
{
    Message m(MessageGeneric, cfg);
    QString text = m.data.Text.str();
    if (!m_filter.isEmpty())
    {
        if (text.isEmpty())
        {
            QByteArray serverText = m.getServerText();
            if (serverText.isEmpty())
                return;
            if (m_codec == NULL)
                m_codec = getContacts()->getCodec(getContacts()->contact(m_contact));
            text = m_codec->toUnicode(serverText);
        }
        if (text.isEmpty())
            return;
        text = text.toLower();
        if (m.getFlags() & MESSAGE_RICHTEXT)
            text = text.replace(QRegExp("<[^>]+>"), " ");
        text = text.replace(QRegExp("  +"), " ");
        if (text.indexOf(m_filter) < 0)
            return;
    }
    Message *msg = ::createMessage(id, type, cfg);
    if (msg)
    {
        msg->setClient(file.m_name);
        msg->setContact(file.m_contact);
        msgs.push_back(msg);
    }
}

void HistoryFileIterator::begin()
{
    clear();
    m_block = 0;
}

void HistoryFileIterator::end()
{
    clear();
    m_block = file.size();
}

void HistoryFileIterator::clear()
{
    if (m_msg)
    {
        delete m_msg;
        m_msg = NULL;
    }
    for (list<Message*>::iterator it = msgs.begin(); it != msgs.end(); ++it)
        delete *it;
    msgs.clear();
}

Message *HistoryFileIterator::operator ++()
{
    if (m_msg)
    {
        delete m_msg;
        m_msg = NULL;
    }
    while (msgs.empty())
        if (loadBlock(true))
            break;
    if (!msgs.empty())
    {
        m_msg = msgs.front();
        msgs.pop_front();
        return m_msg;
    }
    return NULL;
}

Message *HistoryFileIterator::operator --()
{
    if (m_msg)
    {
        delete m_msg;
        m_msg = NULL;
    }
    while (msgs.empty())
        if (loadBlock(false))
            break;
    if (msgs.empty())
        return NULL;

    m_msg = msgs.back();
    msgs.pop_back();
    return m_msg;
}

bool HistoryFileIterator::loadBlock(bool bUp)
{
    unsigned blockEnd = m_block;
    if (bUp && !file.seek(m_block))
    {
        clear();
        return true;
    }
    Buffer config;
    for (;;)
    {
        int block = m_block;
        if (bUp)
        {
            if (blockEnd >= file.size())
                return true;
            blockEnd += BLOCK_SIZE;
            unsigned size = config.size();
            config.resize(BLOCK_SIZE);
            int readn = file.read((char*)config.data(size), BLOCK_SIZE);
            if (readn < 0)
            {
                log(L_WARN, "Can't read %s", qPrintable(file.fileName()));
                clear();
                return true;
            }
            config.resize(size + readn);
        }
        else
        {
            if (m_block == 0)
                return true;
            block -= BLOCK_SIZE;
            if (block < 0)
                block = 0;
            if (!file.seek(block))
            {
                m_block = 0;
                clear();
                return true;
            }
            unsigned size = m_block - block;
            m_block = block;
            config.resize(size);
            if ((unsigned)file.read(config.data(), size) != size)
            {
                log(L_WARN, "Can't read %s", qPrintable(file.fileName()));
                clear();
                return true;
            }
            config.setWritePos(0);
        }
        QByteArray type = config.getSection(!bUp && (m_block != 0));
        if (type.isEmpty() || 
                ((config.writePos() == (unsigned)config.size()) &&
                (file.pos() < file.size()))
           )
            continue;
        unsigned id = m_block;
        if (!bUp)
            m_block += config.startSection();
        createMessage(id + config.startSection(), type, &config);
        unsigned pos = config.writePos();
        for (;;)
        {
            type = config.getSection();
            if ((!bUp) && (id + config.writePos() > blockEnd) ||
                type.isEmpty() || 
                    (((config.writePos() == (unsigned)config.size()) &&
                    (file.pos() < file.size()))))
                break;
            createMessage(id + config.startSection(), type, &config);
            pos = config.writePos();
        }
        if (bUp)
            m_block += pos;
        break;
    }
    return false;
}

Message *HistoryFileIterator::message()
{
    return m_msg;
}

History::History(unsigned id)
    : m_contact (id)
{
    Contact *contact = getContacts()->contact(id);
    if (contact == NULL)
        return;
    HistoryFile *f = new HistoryFile(QString::number(id), id);
    if (f->isOpen())
        files.push_back(f);
    else
        delete f;
    void *data;
    ClientDataIterator it = contact->clientDataIterator();
    QStringList fnames;
    while ((data = ++it) != NULL)
    {
        QString name = it.client()->dataName(data);
        if(fnames.contains(name))
            continue;
        fnames.append(name);
        HistoryFile *f = new HistoryFile(name, id);
        f->m_name = name;
        if (f->isOpen())
            files.push_back(f);
        else delete f;
    }
}

History::~History()
{
    for (list<HistoryFile*>::iterator it = files.begin(); it != files.end(); ++it)
        delete *it;
}

HistoryIterator::HistoryIterator(unsigned contact_id)
        : m_bUp     (false)
        , m_bDown   (false)
        , m_temp_id (0)
        , m_history (contact_id)
        , m_it      (NULL)
{
    for (list<HistoryFile*>::iterator it = m_history.files.begin(); it != m_history.files.end(); ++it)
        iters.push_back(new HistoryFileIterator(**it, contact_id));
}

HistoryIterator::~HistoryIterator()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        delete *it;
}

void HistoryIterator::begin()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->begin();
    m_temp_id = 0;
    m_bUp = m_bDown = false;
}

void HistoryIterator::end()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->end();
    m_temp_id = 0xFFFFFFFF;
    m_bUp = m_bDown = false;
}

QString HistoryIterator::state()
{
    QString res;
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
    {
        if (!res.isEmpty())
            res += ';';
        Message *msg = (*it)->message();
        res += msg ? QString::number(msg->id()) : QString::number((*it)->m_block);
        res += ',';
        res += (*it)->file.m_name;
    }
    if (!res.isEmpty())
        res += ';';
    res += QString::number(m_temp_id);
    res += ",temp";
    return res;
}

void HistoryIterator::setState(const QString &str)
{
    QString s = str;
    while (!s.isEmpty())
    {
        QString item = getToken(s, ';');
        unsigned pos = getToken(item, ',').toUInt();
        if (item == "temp")
        {
            m_temp_id = item.toULong();
            continue;
        }
        for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        {
            if ((*it)->file.m_name == item)
            {
                (*it)->clear();
                (*it)->m_block = pos;
                break;
            }
        }
    }
    m_bUp = m_bDown = false;
}

Message *HistoryIterator::operator ++()
{
    if (!m_bUp)
    {
        m_bUp   = true;
        m_bDown = false;
        for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        {
            (*it)->clear();
            ++(**it);
        }
        m_it = NULL;
    }
    if (m_it)
        ++(*m_it);
    m_it = NULL;
    Message *msg = NULL;
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
    {
        Message *m = (**it).message();
        if (m == NULL ||
            (msg != NULL && msg->getTime() <= m->getTime()))
            continue;

        msg = m;
        m_it = *it;
    }
    if (msg)
        return msg;
    if (History::s_tempMsg)
    {
        MAP_MSG::iterator itm;
        for (itm = History::s_tempMsg->begin(); itm != History::s_tempMsg->end(); ++itm)
            if (itm->first > m_temp_id)
                break;
        for (; itm != History::s_tempMsg->end(); ++itm)
        {
            if (itm->second.contact != m_history.m_contact)
                continue;

            m_temp_id = (*itm).first;
            Message *msg = History::load(m_temp_id, QString::null, m_history.m_contact);
            if (msg)
                return msg;
        }
        m_temp_id = 0xFFFFFFFF;
    }
    return NULL;
}

Message *HistoryIterator::operator --()
{
    if (m_temp_id && History::s_tempMsg)
    {
        MAP_MSG::iterator itm = History::s_tempMsg->end();
        if (itm != History::s_tempMsg->begin())
        {
            for (--itm;;--itm)
            {
                if (itm->first < m_temp_id)
                    break;
                if (itm != History::s_tempMsg->begin())
                    continue;

                m_temp_id = 0;
                break;
            }
            if (m_temp_id)
            {
                for (;; --itm)
                {
                    if (itm->second.contact == m_history.m_contact)
                    {
                        m_temp_id = itm->first;
                        Message *msg = History::load(m_temp_id, QString::null, m_history.m_contact);
                        if (msg)
                            return msg;
                    }
                    if (itm == History::s_tempMsg->begin())
                        break;
                }
            }
        }
    }
    m_temp_id = 0;
    if (!m_bDown)
    {
        m_bDown = true;
        m_bUp   = false;
        for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        {
            (*it)->clear();
            --(**it);
        }
        m_it = NULL;
    }
    if (m_it)
        --(*m_it);
    m_it = NULL;
    Message *msg = NULL;
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
    {
        Message *m = (**it).message();
        if (m == NULL || 
            (msg != NULL && msg->getTime() > m->getTime()))
            continue;
        msg = m;
        m_it = *it;
    }
    return msg;
}

void HistoryIterator::setFilter(const QString &filter)
{
    QString f = filter.toLower();
    f = f.replace(QRegExp("  +"), " ");
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->m_filter = f;
}

Message *History::load(unsigned id, const QString &client, unsigned contact)
{
    if (id >= TEMP_BASE)
    {
        if (s_tempMsg == NULL)
            return NULL;
        MAP_MSG::iterator it = s_tempMsg->find(id);
        if (it == s_tempMsg->end())
            return NULL;
        msg_save &ms = it->second;
        Buffer config;
        config = ms.msg;
        config.setWritePos(0);
        QByteArray type = config.getSection();
        Message *msg = createMessage(id, type, &config);
        if (!msg)
            return NULL;
        msg->setClient(ms.client);
        msg->setContact(ms.contact);
        msg->setFlags(msg->getFlags() | MESSAGE_TEMP);
        return msg;
    }
    HistoryFile f(client, contact);
    if (!f.isOpen())
        return NULL;
    return f.load(id);
}

void History::add(Message *msg, const QString &type)
{
    QByteArray line = "[";
    line += type.toUtf8();
    line += "]\n";
    line += msg->save();
    line += '\n';

    if (msg->getFlags() & MESSAGE_TEMP)
    {
        if (s_tempMsg == NULL)
            s_tempMsg = new MAP_MSG;
        msg_save ms;
        ms.msg     = line;
        ms.contact = msg->contact();
        ms.client  = msg->client();
        s_tempMsg->insert(MAP_MSG::value_type(++s_tempId, ms));
        msg->setId(s_tempId);
        return;
    }

    if (!line.isEmpty() && line.at(line.length() - 1) != '\n')
        line += '\n';

    QString name = msg->client();
    if (name.isEmpty())
        name = QString::number(msg->contact());
    QString f_name = QString(HISTORY_PATH).append(name);

    f_name = user_file(f_name);

    Contact *contact = getContacts()->contact(msg->contact());
	SIM::PropertyHubPtr data;
    if (contact)
         data = contact->getUserData("history");
    if (!data.isNull() && data->value("CutSize").toBool())
    {
        QFileInfo fInfo(f_name);
        if (fInfo.exists() && (fInfo.size() >= data->value("MaxSize").toInt() * 0x100000 + CUT_BLOCK))
        {
            int pos = fInfo.size() - data->value("MaxSize").toUInt() * 0x100000 + line.size();
            if (pos < 0)
                pos = 0;
            del(f_name, msg->contact(), pos, false);
        }
    }

    QFile f(f_name);
    if (!f.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        log(L_ERROR, "Can't open %s", qPrintable(f_name));
        return;
    }
    qint64 id = f.pos();
    f.write(line);

    msg->setId(id);
}

void History::del(Message *msg)
{
    QString name = msg->client();
    if (name.isEmpty())
        name = QString::number(msg->contact());
    del(name, msg->contact(), msg->id(), true);
}

void History::rewrite(Message *msg)
{
    QString name = msg->client();
    if (name.isEmpty())
        name = QString::number(msg->contact());
    del(name, msg->contact(), msg->id(), true, msg);
}

typedef map<my_string, unsigned> CLIENTS_MAP;

void History::cut(Message *msg, unsigned contact_id, unsigned date)
{
    QString client;
    if (msg)
        client = msg->client();
    CLIENTS_MAP clients;
    {
        HistoryIterator it(msg ? msg->contact() : contact_id);
        Message *m;
        while ((m = ++it) != NULL)
        {
            if (date && m->getTime() > date)
                break;
            CLIENTS_MAP::iterator itm = clients.find(m->client());
            if (itm == clients.end())
                clients.insert(CLIENTS_MAP::value_type(m->client(), m->id()));
            else
                itm->second = m->id();
            if (!msg || 
                client != m->client() || 
                m->id() < msg->id())
                continue;

            break;
        }
    }
    for (CLIENTS_MAP::iterator it = clients.begin(); it != clients.end(); ++it)
        del(it->first.str(), msg ? msg->contact() : contact_id, it->second + 1, false);
}

void History::del(const QString &name, unsigned contact, unsigned id, bool bCopy, Message *msg)
{
    QFile f(user_file(QString(HISTORY_PATH).append(name)));
    if (!f.open(QIODevice::ReadOnly))
    {
        log(L_ERROR, "Can't open %s", qPrintable(f.fileName()));
        return;
    }
    QFile t(f.fileName() + '~');
    if (!t.open(QIODevice::ReadWrite | QIODevice::Truncate))
    {
        log(L_ERROR, "Can't open %s", qPrintable(t.fileName()));
        return;
    }
    unsigned tail = id;
    for (; tail > 0; )
    {
        char b[LOAD_BLOCK_SIZE];
        int size = sizeof(b);
        if (tail < (unsigned)size)
            size = tail;
        size = f.read(b, size);
        if (size == -1)
        {
            log(L_ERROR, "Read history error");
            return;
        }
        if (bCopy && t.write(b, size) != size)
        {
            log(L_ERROR, "Write history error");
            return;
        }
        tail -= size;
    }
    Buffer config;
    unsigned skip_start = id;
    for (;;)
    {
        unsigned size = config.size();
        config.resize(LOAD_BLOCK_SIZE);
        int readn = f.read(config.data(size), LOAD_BLOCK_SIZE);
        if (readn < 0)
        {
            log(L_ERROR, "Read history error");
            return;
        }
        config.resize(size + readn);
        QByteArray section = config.getSection();
        if (section.isEmpty())
        {
            if (readn == 0)
                return;
            continue;
        }
        if (config.writePos() == (unsigned)config.size() && 
            readn != 0)
            continue;

        break;
    }
    if (config.startSection())
    {
        skip_start += config.startSection();
        if ((unsigned)t.write(config.data(), config.startSection()) != config.startSection())
        {
            log(L_ERROR, "Write history error");
            return;
        }
    }
    unsigned skip_size = config.writePos() - config.startSection();
    QByteArray line = "\n";
    if (msg)
    {
        line += msg->save();
        line += '\n';
        skip_start++;
    }
    int size = line.length();
    int written = t.write(line, size);
    if (written != size)
    {
        log(L_DEBUG, "Write history error");
        return;
    }
    skip_size -= line.length();
    if (config.writePos() < (unsigned)config.size())
    {
        size = config.size() - config.writePos();
        written = t.write(config.data(config.writePos()), size);
        if (written != size)
        {
            log(L_DEBUG, "Write history error");
            return;
        }
    }
    tail = f.size() - f.pos();
    for (; tail > 0; )
    {
        char b[2048];
        size = f.read(b, sizeof(b));
        if (size == -1)
        {
            log(L_ERROR, "Read history error");
            return;
        }
        written = t.write(b, size);
        if (written != size)
        {
            log(L_DEBUG, "Write history error");
            return;
        }
        tail -= size;
    }
    f.close();
    t.close();
    QFileInfo fInfo(f.fileName());
    QFileInfo tInfo(t.fileName());
    fInfo.dir().remove(fInfo.fileName());
    if (!tInfo.dir().rename(tInfo.fileName(), fInfo.fileName())) 
    {
        log(L_ERROR, "Can't rename file %s to %s", qPrintable(fInfo.fileName()), qPrintable(tInfo.fileName()));
        return;
    }
    CutHistory ch;
    ch.contact = contact;
    ch.client  = name;
    if (bCopy)
    {
        ch.from    = skip_start;
        ch.size    = skip_size;
    }
    else
    {
        ch.from    = skip_start;
        ch.size	   = skip_start + skip_size;
    }
    EventCutHistory(&ch).process();
}

void History::del(unsigned msg_id)
{
    if (s_tempMsg == NULL)
        return;
    MAP_MSG::iterator it = s_tempMsg->find(msg_id);
    if (it == s_tempMsg->end())
    {
        log(L_WARN, "Message %X for remove not found", msg_id);
        return;
    }
    s_tempMsg->erase(it);
}

void History::remove(Contact *contact)
{
    bool bRename = (contact->getFlags() & CONTACT_NOREMOVE_HISTORY);
    QString name = QString::number(contact->id());
    QString f_name = HISTORY_PATH;
    f_name += name;
    name = user_file(f_name);
    QFile f(name);
    f.remove();

    void *data;
    ClientDataIterator it = contact->clientDataIterator();
    while ((data = ++it) != NULL)
    {
        name = it.client()->dataName(data);
        f_name = HISTORY_PATH;
        f_name += name;
        name = user_file(f_name);
        QFile f(name);
        if (!f.exists())
            continue;
        if (bRename)
        {
            QFileInfo fInfo(f.fileName());
            fInfo.dir().rename(fInfo.fileName(), QString(fInfo.fileName()).append(REMOVED));
            continue;
        }
        f.remove();
    }
}

bool History::save(unsigned id, const QString& file_name, bool bAppend)
{
    QFile f(file_name);
	QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    if (bAppend)
        mode |= QIODevice::Append;
    if(f.open(mode))
	{
        QTextStream stream(&f);
        HistoryIterator it(id);
        it.begin();
        const QString owner = getContacts()->owner()->getName(),
                      contact = getContacts()->contact(id)->getName();
        for (;;) 
        {
            Message *msg = ++it;
            if (msg == NULL)
                break;
            QDateTime t = QDateTime::fromTime_t(msg->getTime());
            QString time;
            time = t.toString("hh:mm:ss");
            stream << (msg->getFlags() & MESSAGE_RECEIVED ? contact : owner)
                << " (" << formatDate(t.date()) << " " << time << "):\n"
                << msg->getPlainText()
                << "\n\n";
        }
        const QFile::FileError status = f.error();
        const QString errorMessage = f.errorString();
        f.close();
        if (status != QFile::NoError) 
        {
            log(L_ERROR, "I/O error during write to file %s : %s", qPrintable(file_name), qPrintable(errorMessage));
            return false;
        }
        return true;
    }
	//else deleted: unreachable Code
    log(L_ERROR, "Can't open %s for writing", qPrintable(file_name));
    return false;
    
}

/***************************************************************************
                          message.h  -  description
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

#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QDateTime>

#include "simapi.h"
#include "event.h"
#include "cfg.h"

class QFile;

namespace SIM {

const unsigned MessageGeneric       = 1;
const unsigned MessageSMS           = 2;
const unsigned MessageFile          = 3;
const unsigned MessageAuthRequest   = 4;
const unsigned MessageAuthGranted   = 5;
const unsigned MessageAuthRefused   = 6;
const unsigned MessageAdded         = 7;
const unsigned MessageRemoved       = 8;
const unsigned MessageStatus        = 9;
const unsigned MessageTypingStart   = 10;
const unsigned MessageTypingStop    = 11;
const unsigned MessageUrl           = 12;
const unsigned MessageContacts      = 13;

// Message flags:

const unsigned  MESSAGE_RECEIVED    = 0x00000001;
const unsigned  MESSAGE_RICHTEXT    = 0x00000002;
const unsigned  MESSAGE_SECURE      = 0x00000004;
const unsigned  MESSAGE_URGENT      = 0x00000008;
const unsigned  MESSAGE_LIST        = 0x00000010;
const unsigned  MESSAGE_NOVIEW      = 0x00000020;

const unsigned  MESSAGE_SAVEMASK    = 0x0000FFFF;

const unsigned  MESSAGE_TRANSLIT    = 0x00010000;
const unsigned  MESSAGE_1ST_PART    = 0x00020000;
const unsigned  MESSAGE_NOHISTORY   = 0x00040000;
const unsigned  MESSAGE_LAST        = 0x00080000;
const unsigned  MESSAGE_MULTIPLY    = 0x00100000;
const unsigned  MESSAGE_FORWARD     = 0x00200000;
const unsigned  MESSAGE_INSERT      = 0x00400000;
const unsigned  MESSAGE_OPEN        = 0x00800000;
const unsigned  MESSAGE_NORAISE     = 0x01000000;
const unsigned  MESSAGE_TEMP        = 0x10000000;

struct MessageData
{
    Data        Text;           // Message text (locale independent)
    Data        ServerText;     // Message text (locale dependent 8bit endcoding!)
    Data        Flags;          // Message flags from above, should be QFlags in Qt4
    Data        Background;
    Data        Foreground;
    Data        Time;
    Data        Font;
    Data        Error;
    Data        RetryCode;
    Data        Resource;
};

class EXPORT Message
{
public:
    Message(unsigned type = MessageGeneric, Buffer *cfg = NULL);
    virtual ~Message();
    unsigned type() const { return m_type; }
    unsigned id() const { return m_id; }
    void setId(unsigned id) { m_id = id; }
    unsigned contact() const { return m_contact; }
    void setContact(unsigned contact) { m_contact = contact; }
    virtual QByteArray save();
    virtual unsigned baseType() { return m_type; }
    QString getPlainText();
    QString getRichText();
    virtual QString getText() const;
    void setText(const QString &text);
    PROP_CSTR(ServerText)
    PROP_ULONG(Flags)
    PROP_ULONG(Background)
    PROP_ULONG(Foreground)
    PROP_ULONG(Time)
    PROP_STR(Error);
    PROP_STR(Font);
    PROP_ULONG(RetryCode);
    PROP_UTF8(Resource);
    const QString &client() const { return m_client; }
    void setClient(const QString &client);
    virtual QString presentation();
    MessageData data;
protected:
    unsigned    m_id;
    unsigned    m_contact;
    unsigned    m_type;
    QString     m_client;
};

struct MessageSMSData
{
    Data    Phone;
    Data    Network;
};

class EXPORT SMSMessage : public Message
{
public:
    SMSMessage(Buffer *cfg=NULL);
    ~SMSMessage();
    PROP_UTF8(Phone);
    PROP_UTF8(Network);
    virtual QByteArray save();
    virtual QString presentation();
protected:
    MessageSMSData data;
};

struct MessageFileData
{
    Data        File;
    Data        Description;
    Data        Size;
};

class FileMessage;

class FileTransferNotify
{
public:
    FileTransferNotify() {}
    virtual ~FileTransferNotify() {}
    virtual void process() = 0;
    virtual void transfer(bool) = 0;
    virtual void createFile(const QString &name, unsigned size, bool bCanResume) = 0;
};

const unsigned NO_FILE  = (unsigned)(-1);

class EXPORT FileTransfer
{
public:
    FileTransfer(FileMessage *msg);
    virtual ~FileTransfer();
    void setNotify(FileTransferNotify*);
    FileTransferNotify          *notify() { return m_notify; }
    unsigned file()             { return m_nFile; }
    unsigned files()            { return m_nFiles; }
    unsigned bytes()            { return m_bytes; }
    unsigned totalBytes()       { return m_totalBytes; }
    unsigned fileSize()         { return m_fileSize; }
    unsigned totalSize()        { return m_totalSize; }
    unsigned speed()            { return m_speed; }
    unsigned transferBytes()    { return m_transferBytes; }
    virtual void setSpeed(unsigned speed);
    QString  dir()              { return m_dir; }
    void setDir(const QString &dir) { m_dir = dir; }
    enum OverwriteMode overwrite()   { return m_overwrite; }
    void setOverwrite(OverwriteMode overwrite) { m_overwrite = overwrite; }
    enum State
    {
        Unknown,
        Listen,
        Connect,
        Negotiation,
        Read,
        Write,
        Done,
        Wait,
        Error
    };
    State state()   { return m_state; }
    QFile           *m_file;
    virtual void    startReceive(unsigned pos) = 0;
    virtual void    setError();
    void    addFile(const QString &file, unsigned size);
    QString filename()      { return m_name; }
    bool    isDirectory()   { return m_bDir; }
protected:
    bool    openFile();
    FileMessage         *m_msg;
    FileTransferNotify  *m_notify;
    unsigned m_nFile;
    unsigned m_nFiles;
    unsigned m_bytes;
    unsigned m_totalBytes;
    unsigned m_fileSize;
    unsigned m_totalSize;
    unsigned m_speed;
    unsigned m_transferBytes;

    QDateTime m_sendTime;
    unsigned m_sendSize;
    unsigned m_transfer;

    OverwriteMode m_overwrite;
    QString  m_dir;
    QString  m_base;
    QString  m_name;
    State    m_state;
    bool     m_bDir;
    friend class FileMessage;
};

class EXPORT FileMessage : public Message
{
public:
    FileMessage(unsigned type=MessageFile, Buffer *cfg=NULL);
    ~FileMessage();
    PROP_UTF8(File);
    unsigned getSize();
    void     setSize(unsigned);
    virtual QByteArray save();
    virtual QString presentation();
    virtual QString getDescription();
    bool    setDescription(const QString&);
    void    addFile(const QString&);
    void    addFile(const QString&, unsigned size);
    class EXPORT Iterator
    {
    public:
        Iterator(const FileMessage&);
        ~Iterator();
        const QString *operator++();
        const QString *operator[](unsigned);
        void reset();
        unsigned count();
        unsigned dirs();
        unsigned size();
    protected:
        class FileMessageIteratorPrivate *p;
        friend class FileMessage;

        COPY_RESTRICTED(Iterator)
    };
    FileTransfer    *m_transfer;
protected:
    MessageFileData data;
    friend class FileTransfer;
    friend class Iterator;
};

class EXPORT AuthMessage : public Message
{
public:
    AuthMessage(unsigned type, Buffer *cfg=NULL)
: Message(type, cfg) {}
    virtual QString presentation();
};

struct MessageUrlData
{
    Data    Url;
};

class EXPORT UrlMessage : public Message
{
public:
    UrlMessage(unsigned type=MessageUrl, Buffer *cfg=NULL);
    ~UrlMessage();
    virtual QByteArray save();
    virtual QString presentation();
    VPROP_UTF8(Url)
protected:
    MessageUrlData data;
};

struct MessageContactsData
{
    Data    Contacts;
};

class EXPORT ContactsMessage : public Message
{
public:
    ContactsMessage(unsigned type=MessageContacts, Buffer *cfg=NULL);
    ~ContactsMessage();
    virtual QByteArray save();
    virtual QString presentation();
    VPROP_UTF8(Contacts);
protected:
    MessageContactsData data;
};

struct MessageStatusData
{
    Data    Status;
};

class EXPORT StatusMessage : public Message
{
public:
    StatusMessage(Buffer *cfg=NULL);
    PROP_ULONG(Status);
    virtual QByteArray save();
    virtual QString presentation();
protected:
    MessageStatusData data;
};

} // namespace SIM

#endif

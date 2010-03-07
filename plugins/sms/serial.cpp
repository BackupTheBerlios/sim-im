/***************************************************************************
                          serial.cpp  -  description
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

#include <QApplication>
#include <QEvent>
#include <QStringList>
#include <QTimer>
#include <QDir>
#include <QSocketNotifier>
#include <QByteArray>

//#ifdef HAVE_UNISTD_H
#include <unistd.h>
//#endif

#include "buffer.h"
#include "log.h"
#include "serial.h"

using namespace SIM;

#ifdef WIN32

#include <windows.h>

const unsigned SERIAL_TIMEOUT	= 1000;

class SerialEvent : public QEvent
{
public:
    SerialEvent(unsigned reason);
    unsigned reason() { return m_reason; }
protected:
    unsigned m_reason;
};

SerialEvent::SerialEvent(unsigned reason)
        : QEvent(User)
{
    m_reason = reason;
}

const unsigned SerialEventComplete	= 0;
const unsigned SerialEventError		= 1;
const unsigned SerialEventTimeout   = 2;

enum PortState
{
    None,
    Read,
    Write,
    StartRead,
    StartWrite,
    Setup,
    Close
};

class SerialPortPrivate
{
public:
    SerialPortPrivate(SerialPort *port);
    ~SerialPortPrivate();
    void close();
    Qt::HANDLE		hPort;
    Qt::HANDLE		hEvent;
    Qt::HANDLE		hThread;
    OVERLAPPED	over;
    QTimer		*m_timer;
    SerialPort	*m_port;
    int			m_baudrate;
    bool		m_bXonXoff;
    QByteArray	m_line;
    PortState	m_state;
    Buffer		m_buff;
    int			m_time;
    int			m_read_time;
    char		m_char;
};

static DWORD __stdcall SerialThread(LPVOID lpParameter)
{
    log(L_DEBUG, "SerialThread: %X", (unsigned int)GetCurrentThreadId());
    SerialPortPrivate *p = (SerialPortPrivate*)lpParameter;
    DWORD timeout = INFINITE;
    for (;;){
        DWORD res = WaitForSingleObject(p->hEvent, timeout);
        if ((res == WAIT_ABANDONED) || (p->m_state == Close))
            break;
        timeout = INFINITE;
        switch (p->m_state){
        case StartRead:{
                DWORD bytesReadn = 0;
                memset(&p->over, 0, sizeof(p->over));
                p->over.hEvent = p->hEvent;
                p->m_state = Read;
                if (ReadFile(p->hPort, &p->m_char, 1, &bytesReadn, &p->over))
                    break;
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING){
                    p->m_state = None;
                    QApplication::postEvent(p->m_port, new SerialEvent(SerialEventError));
                }else{
                    timeout = p->m_read_time;
                }
                break;
            }
        case StartWrite:{
                DWORD bytesWritten = 0;
                memset(&p->over, 0, sizeof(p->over));
                p->over.hEvent = p->hEvent;
                p->m_state = Write;
                if (WriteFile(p->hPort, p->m_line.data(), p->m_line.length(), &bytesWritten, &p->over))
                    break;
                DWORD err = GetLastError();
                if (err != ERROR_IO_PENDING){
                    p->m_state = None;
                    QApplication::postEvent(p->m_port, new SerialEvent(SerialEventError));
                }else{
                    timeout = SERIAL_TIMEOUT;
                }
                break;
            }
        case Read:
        case Write:
            if (res == WAIT_TIMEOUT){
                QApplication::postEvent(p->m_port, new SerialEvent(SerialEventTimeout));
            }else{
                QApplication::postEvent(p->m_port, new SerialEvent(SerialEventComplete));
            }
            break;
        default:
            break;
        }
    }
    return 0;
}

SerialPort::SerialPort(QObject *parent)
        : QObject(parent)
{
    d = new SerialPortPrivate(this);
    connect(d->m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

SerialPortPrivate::SerialPortPrivate(SerialPort *port)
{
    hPort = INVALID_HANDLE_VALUE;
    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_timer    = new QTimer(NULL);
    m_port     = port;
    m_state	   = None;
    DWORD threadId;
    hThread = CreateThread(NULL, 0, SerialThread, this, 0, &threadId);
}

SerialPortPrivate::~SerialPortPrivate()
{
    close();
    delete m_timer;
    m_state = Close;
    SetEvent(hEvent);
    CloseHandle(hEvent);
    WaitForSingleObject(hThread, INFINITE);
}

void SerialPortPrivate::close()
{
    if (hPort != INVALID_HANDLE_VALUE){
        CloseHandle(hPort);
        hPort = INVALID_HANDLE_VALUE;
        m_buff.init(0);
    }
    m_state = None;
}

SerialPort::~SerialPort()
{
    delete d;
}

void SerialPort::close()
{
    d->close();
}

void SerialPort::readReady(int)
{
}

bool SerialPort::openPort(const char *device, int baudrate, bool bXonXoff, int DTRtime)
{
    close();
    d->m_time	  = DTRtime;
    d->m_baudrate = baudrate;
    d->m_bXonXoff = bXonXoff;
    QByteArray port; // = "\\\\.\\";
    port += device;
    port += ":";
    d->hPort = CreateFileA(port.data(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
    if (d->hPort == INVALID_HANDLE_VALUE){
        close();
        log(L_WARN, "Can' open %s", port.data());
        return false;
    }
    FlushFileBuffers(d->hPort);
    if (!EscapeCommFunction(d->hPort, CLRDTR)){
        close();
        log(L_WARN, "Clear DTR error");
        return false;
    }
    d->m_timer->setSingleShot(true);
    d->m_timer->start(d->m_time);
    return true;
}

void SerialPort::timeout()
{
    if (d->m_state == Setup){
        if (!SetupComm(d->hPort, 1024, 1024)){
            log(L_WARN, "SetupComm error");
            close();
            emit error();
            return;
        }
        PurgeComm(d->hPort, PURGE_RXABORT | PURGE_RXCLEAR);
        d->m_state = None;
        emit write_ready();
        return;
    }
    if (!EscapeCommFunction(d->hPort, SETDTR)){
        log(L_WARN, "Set DTR error");
        close();
        emit error();
        return;
    }
    DCB dcb;
    memset(&dcb, 0, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);
    if (GetCommState(d->hPort, &dcb) == 0){
        log(L_WARN, "Get status error");
        close();
        emit error();
        return;
    }
    dcb.fBinary  = TRUE;
    dcb.BaudRate = d->m_baudrate;
    dcb.fParity  = FALSE;
    dcb.Parity   = 0;
    dcb.ByteSize = 8;
    dcb.StopBits = 0;
    if (!d->m_bXonXoff)
    {
        dcb.fInX  = FALSE;
        dcb.fOutX = FALSE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutxCtsFlow = FALSE;
    }
    else
    {
        dcb.fInX  = TRUE;
        dcb.fOutX = TRUE;
        dcb.fOutxDsrFlow = FALSE;
        dcb.fOutxCtsFlow = FALSE;
    }
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;
    if (SetCommState(d->hPort, &dcb) == 0){
        log(L_WARN, "Set status error");
        close();
        emit error();
        return;
    }
    d->m_state = Setup;
    d->m_timer->setSingleShot(true);
    d->m_timer->start(d->m_time);
}

void SerialPort::writeLine(const char *data, unsigned read_time)
{
    if (d->hPort == INVALID_HANDLE_VALUE){
        emit error();
        return;
    }
    switch (d->m_state){
    case Read:
    case Write:
        CancelIo(d->hPort);
        break;
    default:
        break;
    }
    d->m_state		= StartWrite;
    d->m_line		= data;
    d->m_read_time	= read_time;
    FlushFileBuffers(d->hPort);
    SetEvent(d->hEvent);
}

void SerialPort::setTimeout(unsigned read_time)
{
    switch (d->m_state){
    case Read:
    case Write:
        CancelIo(d->hPort);
        break;
    default:
        break;
    }
    d->m_state		= StartRead;
    d->m_read_time	= read_time;
    SetEvent(d->hEvent);
}

QByteArray SerialPort::readLine()
{
    QByteArray res;
    if (d->hPort == INVALID_HANDLE_VALUE)
        return res;
    if (d->m_buff.scan("\n", res)){
        if (d->m_buff.readPos() == d->m_buff.writePos())
            d->m_buff.init(0);
    }
    return res;
}

bool SerialPort::event(QEvent *e)
{
    if (e->type() != QEvent::User)
        return QObject::event(e);
    switch (static_cast<SerialEvent*>(e)->reason()){
    case SerialEventComplete:{
            DWORD bytes;
            if (GetOverlappedResult(d->hPort, &d->over, &bytes, true)){
                if (d->m_state == Read){
                    d->m_buff.pack(&d->m_char, 1);
                    if (d->m_char == '\n')
                        emit read_ready();
                }
                if (d->m_state == Write){
                    emit write_ready();
                    d->m_state = Read;
                }
                if (d->m_state == Read){
                    d->m_state = StartRead;
                    SetEvent(d->hEvent);
                }
                break;
            }
            close();
            emit error();
            break;
        }
    case SerialEventTimeout:{
            log(L_WARN, "IO timeout");
            CancelIo(d->hPort);
            close();
            emit error();
            break;
        }
    case SerialEventError:{
            log(L_WARN, "IO error");
            close();
            emit error();
        }
    }
    return true;
}

void SerialPort::readTimeout()
{
}

QStringList SerialPort::devices()
{
    QStringList res;
    for (unsigned i = 1; i <= 8; i++){
        QString port = "COM" + QString::number(i);
        QString fullPort = "\\\\.\\" + port;
        Qt::HANDLE hPort = CreateFile((LPCWSTR)fullPort.utf16(),GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
        if (hPort == INVALID_HANDLE_VALUE)
            continue;
        res.append(port);
        CloseHandle(hPort);
    }
    return res;
}

#else

#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

enum PortState
{
    None,
    Setup
};

class SerialPortPrivate
{
public:
    SerialPortPrivate(SerialPort*);
    ~SerialPortPrivate();
    void close();
    QTimer			*m_timer;
    QTimer			*m_readTimer;
    QSocketNotifier	*m_notify;
    int fd;
    int m_time;
    int m_timeout;
    int m_baudrate;
    bool m_bXonXoff;
    Buffer m_buf;
    PortState m_state;
};

SerialPortPrivate::SerialPortPrivate(SerialPort *port)
{
    fd = -1;
    m_timer     = new QTimer(port);
    m_readTimer = new QTimer(port);
    m_timeout   = 0;
    m_notify = NULL;
    m_state  = None;
}

SerialPortPrivate::~SerialPortPrivate()
{
    close();
}

void SerialPortPrivate::close()
{
    if (m_notify){
        delete m_notify;
        m_notify = NULL;
    }
    if (fd == -1)
        return;
    ::close(fd);
    fd = -1;
}

SerialPort::SerialPort(QObject *parent)
        : QObject(parent)
{
    d = new SerialPortPrivate(this);
    connect(d->m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
    connect(d->m_readTimer, SIGNAL(timeout()), this, SLOT(readTimeout()));
}

SerialPort::~SerialPort()
{
    delete d;
}

bool SerialPort::openPort(const char *device, int baudrate, bool bXonXoff, int DTRtime)
{
    close();
    QByteArray fname = "/dev/";
    fname += device;
    d->m_time = DTRtime;
    d->m_baudrate = baudrate;
    d->m_bXonXoff = bXonXoff;
    d->fd = open(fname.data(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (d->fd == -1){
        log(L_WARN, "Can't open %s: %s", fname.data(), strerror(errno));
        return false;
    }
    int fdFlags;
    if ((fdFlags = fcntl(d->fd, F_GETFL)) == -1){
        log(L_WARN, "Can't get flags %s: %s", fname.data(), strerror(errno));
        close();
        return false;
    }
    fdFlags &= ~O_NONBLOCK;
    if (fcntl(d->fd, F_SETFL, fdFlags) == -1){
        log(L_WARN, "Can't set flags %s: %s", fname.data(), strerror(errno));
        close();
        return false;
    }
    int mctl = TIOCM_DTR;
    if (ioctl(d->fd, TIOCMBIC, &mctl) < 0){
        log(L_WARN, "Clear failed %s: %s", fname.data(), strerror(errno));
        close();
        return false;
    }
    d->m_timer->setSingleShot( true );
    d->m_timer->start( d->m_time );
    return true;
}

void SerialPort::readReady(int)
{
    d->m_readTimer->stop();
    for (;;){
        char c;
        int res = read(d->fd, &c, 1);
        if ((res < 0) && (errno == EAGAIN))
            return;
        if (res <= 0){
            log(L_DEBUG, "Read serial error: %s", (res < 0) ? strerror(errno) : "connection closed");
            close();
            emit error();
            return;
        }
        d->m_readTimer->setSingleShot( true );
        d->m_readTimer->start( d->m_timeout );
        d->m_buf.pack(&c, 1);
        if (c == '\n')
            emit read_ready();
    }
}

void SerialPort::close()
{
    d->close();
}

void SerialPort::writeLine(const char *data, unsigned timeRead)
{
    d->m_readTimer->stop();
    int res = write(d->fd, data, strlen(data));
    if (res < 0){
        log(L_DEBUG, "Write serial error: %s", strerror(errno));
        close();
        emit error();
        return;
    }
    d->m_timeout = timeRead;
    d->m_readTimer->setSingleShot( true );
    d->m_readTimer->start( d->m_timeout );
}

void SerialPort::setTimeout(unsigned timeRead)
{
    d->m_readTimer->stop();
    d->m_timeout = timeRead;
    d->m_readTimer->setSingleShot( true );
    d->m_readTimer->start( d->m_timeout );
}

QByteArray SerialPort::readLine()
{
    QByteArray res;
    if (d->fd == -1)
        return res;
    if (d->m_buf.scan("\n", res)){
        if (d->m_buf.readPos() == d->m_buf.writePos())
            d->m_buf.init(0);
    }
    return res;
}

void SerialPort::readTimeout()
{
    close();
    emit error();
}

void SerialPort::timeout()
{
    if (d->m_state == Setup){
        tcflush(d->fd, TCIFLUSH);
        d->m_state = None;
        d->m_notify = new QSocketNotifier(d->fd, QSocketNotifier::Read, this);
        connect(d->m_notify, SIGNAL(activated(int)), this, SLOT(readReady(int)));
        emit write_ready();
        return;
    }
    int mctl = TIOCM_DTR;
    if (ioctl(d->fd, TIOCMBIS, &mctl) < 0){
        log(L_WARN, "setting DTR failed: %s", strerror(errno));
        close();
        return;
    }

    struct termios t;
    if (tcgetattr(d->fd, &t) < 0){
        log(L_WARN, "Getattr failed: %s", strerror(errno));
        close();
        return;
    }
    cfsetispeed(&t, d->m_baudrate);
    cfsetospeed(&t, d->m_baudrate);

    t.c_iflag |= IGNPAR | (d->m_bXonXoff ? IXON | IXOFF : 0);
    t.c_iflag &= ~(INPCK | ISTRIP | IMAXBEL |
                   (d->m_bXonXoff ? 0 : IXON |  IXOFF)
                   | IXANY | IGNCR | ICRNL | IMAXBEL | INLCR | IGNBRK);
    t.c_oflag &= ~(OPOST);
    t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD |
                   (d->m_bXonXoff ? CRTSCTS : 0 ));
    t.c_cflag |= CS8 | CREAD | HUPCL | (d->m_bXonXoff ? 0 : CRTSCTS) | CLOCAL;
    t.c_lflag &= ~(ECHO | ECHOE | ECHOPRT | ECHOK | ECHOKE | ECHONL |
                   ECHOCTL | ISIG | IEXTEN | TOSTOP | FLUSHO | ICANON);
    t.c_lflag |= NOFLSH;
    t.c_cc[VMIN] = 1;
    t.c_cc[VTIME] = 0;
    t.c_cc[VSUSP] = 0;

    if(tcsetattr (d->fd, TCSANOW, &t) < 0){
        log(L_WARN, "Setattr failed: %s", strerror(errno));
        close();
        return;
    }
    d->m_state = Setup;
    d->m_timer->setSingleShot( true );
    d->m_timer->start( d->m_time );
}

bool SerialPort::event(QEvent *e)
{
    return QObject::event(e);
}

QStringList SerialPort::devices()
{
    QStringList res;
    QDir dev("/dev");
    QStringList entries = dev.entryList( QStringList( "cuaa*" ), QDir::System);
    for (QStringList::Iterator it = entries.begin(); it != entries.end(); ++it)
        res.append(*it);
    return res;
}

#endif


/***************************************************************************
                          serial.h  -  description
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

#ifndef _SERIAL_H
#define _SERIAL_H

#include <QObject>

class SerialPortPrivate;
class QEvent;

class SerialPort : public QObject
{
    Q_OBJECT
public:
    SerialPort(QObject *parent);
    ~SerialPort();
    bool openPort(const char *device, int baudrate, bool bXonXoff, int DTRtime);
    void close();
    static QStringList devices();
    void writeLine(const char *data, unsigned timeRead);
    void setTimeout(unsigned timeRead);
    QByteArray readLine();
signals:
    void write_ready();
    void read_ready();
    void error();
protected slots:
    void timeout();
    void readReady(int);
    void readTimeout();
protected:
    bool event(QEvent *e);
    SerialPortPrivate  *d;
};

#endif


/***************************************************************************
                          logger.h  -  description
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

#ifndef _LOGGER_H
#define _LOGGER_H

#include <QObject>
#include <QSet>

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

class QFile;
const unsigned short L_PACKETS = 0x08;
// const unsigned short L_EVENTS  = 0x10;

class QFile;

class LoggerPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    LoggerPlugin(unsigned, Buffer*);
    virtual ~LoggerPlugin();
    bool isLogType(unsigned id);
    void setLogType(unsigned id, bool bLog);

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

public slots:
    void logEvent(const QString& str, int level);

protected:
    void setDefaultConfigValues();
    QSet<unsigned> m_packets;
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QByteArray getConfig();
    virtual bool processEvent(SIM::Event *e);
    void openFile();
    QFile *m_file;
    bool m_bFilter;
    friend class LogConfig;

	SIM::PropertyHubPtr m_propertyHub;
};

#endif


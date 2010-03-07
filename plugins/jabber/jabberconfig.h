/***************************************************************************
                          jabberconfig.h  -  description
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

#ifndef _JABBERCONFIG_H
#define _JABBERCONFIG_H

#include "ui_jabberconfigbase.h"
#include "event.h"

class JabberClient;

class JabberConfig : public QWidget, public Ui::JabberConfigBase
{
    Q_OBJECT
public:
    JabberConfig(QWidget *parent, JabberClient *client, bool bConfig);
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected slots:
    void changed();
    void changed(const QString&);
    void toggledSSL(bool);
    void toggledVHost(bool);
protected:
    bool m_bConfig;
    JabberClient *m_client;
};

#endif


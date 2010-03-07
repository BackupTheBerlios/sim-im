/***************************************************************************
                          livejournalcfg.h  -  description
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

#ifndef _LIVEJOURNALCFG_H
#define _LIVEJOURNALCFG_H

#include "contacts.h"

#include "ui_livejournalcfgbase.h"

class LiveJournalClient;

class LiveJournalCfg : public QWidget, public Ui::LiveJournalCfgBase
{
    Q_OBJECT
public:
    LiveJournalCfg(QWidget*, LiveJournalClient*, bool bConfig);
signals:
    void okEnabled(bool);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
    void changed(const QString&);
    void changed();
    void useSigToggled(bool);
protected:
    LiveJournalClient *m_client;
    bool m_bConfig;
};

#endif


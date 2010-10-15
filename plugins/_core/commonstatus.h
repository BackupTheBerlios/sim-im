/***************************************************************************
                          status.h  -  description
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

#ifndef _STATUS_H
#define _STATUS_H

#include "clientmanager.h"
#include "core.h"

#include <QObject>
#include <QString>
#include <QStringList>

//struct BalloonItem
//{
//    QString		text;
//    QStringList	buttons;
//    unsigned	id;
//    SIM::Client	*client;
//};

class CORE_EXPORT CommonStatus : public QObject
{
    Q_OBJECT
public:
    CommonStatus(SIM::ClientManager* manager);
    virtual ~CommonStatus();

    void rebuildStatusList();
    int stateCount() const;

//protected slots:
//    void timeout();
//    void setBarStatus();
//    void yes_action(void*);
//    void finished();
//    void showBalloon();
//protected:
//    QTimer *m_timer;
//    virtual bool processEvent(SIM::Event*);
//    void rebuildStatus();
//    void checkInvisible();
//    bool m_bInitialized;
//    bool m_bBlink;
//    bool m_bConnected;
//    BalloonMsg		  *m_balloon;
//    list<BalloonItem> m_queue;

private slots:
    void eventInit();

private:
    SIM::ClientManager* m_clientManager;
};

#endif


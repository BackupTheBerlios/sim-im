/***************************************************************************
                          icqpicture.h  -  description
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

#ifndef _ICQPICTURE_H
#define _ICQPICTURE_H

#include "event.h"

#include "ui_icqpicturebase.h"

class ICQClient;
struct ICQUserData;

class QImage;

class ICQPicture : public QWidget, public Ui::ICQPictureBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ICQPicture(QWidget *parent, ICQUserData *data, ICQClient *client);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected slots:
    void clearPicture();
    void pictSelected(const QString&);
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill();
    void setPict(const QImage &img);
    ICQUserData	*m_data;
    ICQClient	*m_client;
};

#endif


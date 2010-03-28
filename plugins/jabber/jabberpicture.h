/***************************************************************************
                          jabberpicture.h  -  description
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

#ifndef _JABBERPICTURE_H
#define _JABBERPICTURE_H

#include "ui_jabberpicturebase.h"
#include "event.h"

class JabberUserData;
class JabberClient;

class QImage;

class JabberPicture : public QWidget, public Ui::JabberPictureBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    JabberPicture(QWidget *parent, JabberUserData *data, JabberClient *client, bool bPhoto);
public slots:
    void apply();
    void apply(SIM::Client*, void*);
protected slots:
    void clearPicture();
    void pictSelected(const QString&);
protected:
    virtual bool processEvent(SIM::Event *e);
    void fill();
    void setPict(QImage &img);
    bool m_bPhoto;
    JabberUserData	*m_data;
    JabberClient			*m_client;
};

#endif


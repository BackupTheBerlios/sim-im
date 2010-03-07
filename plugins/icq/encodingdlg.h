/***************************************************************************
                          encodingdlg.h  -  description
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

#ifndef _ENCODINGDLG_H
#define _ENCODINGDLG_H

#include "ui_encodingdlgbase.h"

class ICQClient;

class EncodingDlg : public QDialog, public Ui::EncodingDlgBase
{
    Q_OBJECT
public:
    EncodingDlg(QWidget *parent, ICQClient *m_client);
public slots:
    void apply();
    void changed(int);
protected:
    ICQClient *m_client;
};

#endif


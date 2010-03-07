/***************************************************************************
                          msnsearch.h  -  description
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

#ifndef _MSNSEARCH_H
#define _MSNSEARCH_H

#include "contacts.h"

#include "ui_msnsearchbase.h"

#include <QShowEvent>

class MSNClient;
class GroupRadioButton;

class MSNSearch : public QWidget, public Ui::MSNSearch
{
    Q_OBJECT
public:
    MSNSearch(MSNClient *client, QWidget *parent);
signals:
    void setAdd(bool);
protected slots:
    void createContact(unsigned tmpFlags, SIM::Contact *&contact);
protected:
    MSNClient *m_client;
    void showEvent(QShowEvent*);
};

#endif


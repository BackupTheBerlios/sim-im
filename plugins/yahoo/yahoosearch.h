/***************************************************************************
                          yahoosearch.h  -  description
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

#ifndef _YAHOOSEARCH_H
#define _YAHOOSEARCH_H

#include "contacts.h"
#include "fetch.h"

#include "ui_yahoosearchbase.h"

#include <QShowEvent>

class YahooClient;
class GroupRadioButton;

class YahooSearch : public QWidget, public Ui::YahooSearchBase, public FetchClient
{
    Q_OBJECT
public:
    YahooSearch(YahooClient *client, QWidget *parent);
signals:
    void setAdd(bool);
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
protected slots:
    void search();
    void searchStop();
    void searchMail(const QString&);
    void searchName(const QString&, const QString&, const QString&);
	void createContact(unsigned tmpFlags, SIM::Contact *&contact);
    void createContact(const QString&, unsigned tmpFlags, SIM::Contact *&contact);
protected:
    YahooClient *m_client;
    void showEvent(QShowEvent*);
    void search(const QString&, int type);
    bool done(unsigned code, Buffer &data, const QString &headers);
};

#endif


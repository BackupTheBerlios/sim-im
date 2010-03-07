/***************************************************************************
                          searchall.h  -  description
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

#ifndef _SEARCHALL_H
#define _SEARCHALL_H

#ifdef __OS2__  // to make it compileable under OS/2 (gcc 3.3.5)
#include "simapi.h"
#endif
#include "ui_searchallbase.h"

#include <map>

typedef std::map<QWidget*, QStringList>	WND_MAP;

class SearchAll : public QWidget, public Ui::SearchAll
{
    Q_OBJECT
public:
    SearchAll(QWidget *parent);
signals:
    void setAdd(bool);
    void searchName(const QString &first, const QString &last, const QString &nick);
    void searchMail(const QString &mail);
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
    void sSearchStop();
protected slots:
    void search();
    void searchStop();
    void wndDestroyed();
    void refresh();
    void slotSearchDone(QWidget*);
    void slotSetColumns(const QStringList&, int, QWidget*);
    void slotAddItem(const QStringList&, QWidget*);
protected:
    void showEvent(QShowEvent*);
    bool makeSearches();
    WND_MAP		m_searches;
    QStringList	m_attrs;
};

#endif


/***************************************************************************
                          icqsearch.h  -  description
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

#ifndef _ICQSEARCH_H
#define _ICQSEARCH_H

#include "ui_icqsearchbase.h"
#include <QList>
#include <QShowEvent>
#include "event.h"

class ICQClient;
class AdvSearch;
class AIMSearch;
class GroupRadioButton;

class ICQSearch : public QWidget, public Ui::ICQSearchBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ICQSearch(ICQClient *client, QWidget *parent);
    ~ICQSearch();
signals:
    void setAdd(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
protected slots:
    void advDestroyed();
    void radioToggled(bool);
    void advClick();
    void search();
    void searchStop();
    void searchMail(const QString&);
    void searchName(const QString&, const QString&, const QString&);
    void createContact(const QString&, unsigned tmpFlags, SIM::Contact *&contact);
    void createContact(unsigned tmpFlags, SIM::Contact *&contact);
protected:
    enum SearchType
    {
        None,
        UIN,
        Mail,
        Name,
        Full
    };
    static inline const QString extractUIN(const QString& str);
    virtual bool processEvent(SIM::Event *e);
    void showEvent(QShowEvent*);
    void setAdv(bool);
    void icq_search();
    void addColumns();
    void add(const QString &screen, unsigned tmpFlags, SIM::Contact *&contact);
    QList<unsigned>     m_uins;
    ICQClient			*m_client;
    QWidget				*m_adv;
    bool				m_bAdv;
    bool				m_bAdd;
    SearchType			m_type;
    unsigned short		m_id_icq;
    unsigned short		m_id_aim;
    unsigned long		m_uin;
    QString			    m_first;
    QString				m_last;
    QString				m_nick;
    QString				m_mail;
    unsigned short		m_age;
    char				m_gender;
    unsigned short		m_lang;
    QString				m_city;
    QString				m_state;
    unsigned short		m_country;
    QString				m_company;
    QString				m_depart;
    QString				m_position;
    unsigned short		m_occupation;
    unsigned short		m_past;
    QString				m_past_text;
    unsigned short		m_interests;
    QString				m_interests_text;
    unsigned short		m_affilations;
    QString				m_affilations_text;
    QString				m_keywords;
    bool				m_bOnline;
};

#endif


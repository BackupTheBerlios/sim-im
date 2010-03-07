/***************************************************************************
                          jabberadd.h  -  description
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

#ifndef _JABBERADD_H
#define _JABBERADD_H

#include "jabberclient.h"
#include "ui_jabberaddbase.h"

#include <QShowEvent>

class JabberClient;
class JabberBrowser;
class GroupRadioButton;

struct ItemInfo
{
    QString	jid;
    QString	node;
    QString	id;
};

struct AgentSearch
{
    QString		jid;
    QString		node;
    QString		id_info;
    QString		id_search;
    QString			condition;
    unsigned		fill;
    std::vector<QString>	fields;
    QString		type;
};

class JabberAdd : public QWidget, public Ui::JabberAdd, public SIM::EventReceiver
{
    Q_OBJECT
public:
    JabberAdd(JabberClient *client, QWidget *parent);
    ~JabberAdd();
signals:
    void setAdd(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget*);
    void searchDone(QWidget*);
protected slots:
    void radioToggled(bool);
    void browserDestroyed();
    void browserClick();
    void search();
    void searchStop();
    void searchMail(const QString&);
    void searchName(const QString&, const QString&, const QString&);
    void createContact(const QString&, unsigned tmpFlags, SIM::Contact *&contact);
    void createContact(unsigned tmpFlags, SIM::Contact *&contact);
protected:
    virtual bool processEvent(SIM::Event *e);
    void setBrowser(bool bBrowser);
    void showEvent(QShowEvent*);
    void startSearch();
    void checkDone();
    void addAttr(const QString &name, const QString &label);
    void addAttrs();
    void addSearch(const QString &jid, const QString &node, const QString &features, const QString &type);
    JabberClient	*m_client;
    JabberBrowser	*m_browser;
    bool			m_bBrowser;
    QString			m_first;
    QString			m_last;
    QString			m_nick;
    QString			m_mail;
    QString		m_id_browse;
    QString		m_id_disco;
    std::list<ItemInfo>		m_disco_items;
    std::list<AgentSearch>	m_agents;
    std::vector<QString>        m_fields;
    std::vector<QString>	m_labels;
    unsigned			m_nFields;
};

#endif


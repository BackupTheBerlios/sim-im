/***************************************************************************
                          jidsearch.cpp  -  description
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

#include <QPushButton>
#include <QShowEvent>

#include "icons.h"
#include "misc.h"

#include "jabberclient.h"
#include "jidsearch.h"
#include "jabbersearch.h"
#include "jidadvsearch.h"
#include "jabber.h"
#include "contacts/contact.h"

using namespace SIM;

JIDSearch::JIDSearch(QWidget *parent, JabberClient *client, const QString &jid,
                     const QString &node, const QString &type) : QWidget(parent)
        //: JIDSearchBase(parent)
{
    setupUi(this);
    m_client = client;
    m_jid    = jid;
    m_node	 = node;
    m_type	 = type;
    connect(btnBrowser, SIGNAL(clicked()), this, SLOT(browserClicked()));
    connect(btnAdvanced, SIGNAL(clicked()), this, SLOT(advancedClicked()));
    QIcon is = Icon("1rightarrow");
    btnBrowser->setIcon(is);
    btnAdvanced->setIcon(is);
    m_bInit = false;
    m_adv = new JIDAdvSearch(this);
    jidSearch->setAdvanced(m_adv);
    m_bAdv = false;
}

void JIDSearch::browserClicked()
{
    connect(this, SIGNAL(showClient(SIM::Client*)), topLevelWidget(), SLOT(showClient(SIM::Client*)));
    emit showClient(m_client);
    disconnect(this, SIGNAL(showClient(SIM::Client*)), topLevelWidget(), SLOT(showClient(SIM::Client*)));
}

void JIDSearch::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    if (!m_bInit){
        m_bInit = true;
        connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
        connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
        connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
        if(!m_adv->grpSearch->children().empty())
		{
            emit addResult(m_adv);
        }
		else
		{
            btnAdvanced->hide();
            m_adv->hide();
        }
    }
    if (m_bAdv)
	{
        m_bAdv = false;
        advancedClicked();
    }
    emit setAdd(false);
}

void JIDSearch::advancedClicked()
{
    if (m_bAdv){
        m_bAdv = false;
        QIcon is = Icon("1rightarrow");
        btnAdvanced->setIcon(is);
        emit showResult(NULL);
    }else{
        m_bAdv = true;
        QIcon is = Icon("1leftarrow");
        btnAdvanced->setIcon(is);
        emit showResult(m_adv);
    }
}

void JIDSearch::search()
{
    QString condition = jidSearch->condition(NULL);
    if (m_bAdv){
        if (!condition.isEmpty())
            condition += ';';
        condition += jidSearch->condition(m_adv);
        advancedClicked();
    }
    m_search_id = m_client->search(m_jid, m_node, condition);
}

void JIDSearch::searchStop()
{
    m_search_id = QString::null;
}

bool JIDSearch::processEvent(Event *e)
{
    if (e->type() == eEventJabberSearch){
        EventSearch *es = static_cast<EventSearch*>(e);
        JabberSearchData *data = es->searchData();
        if (m_search_id != data->ID.str())
            return false;
        if (data->JID.str().isEmpty()){
            QStringList l;
            l.append(QString::null);
            l.append(i18n("JID"));
            for (unsigned i = 0; i < data->nFields.toULong(); i++){
                l.append(get_str(data->Fields, i * 2));
                l.append(i18n(get_str(data->Fields, i * 2 + 1)));
            }
            emit setColumns(l, 0, this);
            return true;
        }
        QString icon = "Jabber";
        if (m_type == "icq"){
            icon = "ICQ";
        }else if (m_type == "aim"){
            icon = "AIM";
        }else if (m_type == "msn"){
            icon = "MSN";
        }else if (m_type == "yahoo"){
            icon = "Yahoo!";
        }else if (m_type == "sms"){
            icon = "sms";
        }else if ((m_type == "x-gadugadu") || (m_type == "gg")){
            icon = "GG";
        }
        if (!data->Status.str().isEmpty()){
            if (data->Status.str() == "online"){
                icon += "_online";
            }else{
                icon += "_offline";
            }
        }
        QStringList l;
        l.append(icon);
        l.append(data->JID.str());
        l.append(data->JID.str());
        for (unsigned n = 0; n < data->nFields.toULong(); n++)
            l.append(get_str(data->Fields, n));
        emit addItem(l, this);
    }
    if (e->type() == eEventJabberSearchDone){
        EventSearchDone *esd = static_cast<EventSearchDone*>(e);
        QString id = esd->userID();
        if (m_search_id == id){
            m_search_id = QString::null;
            emit searchDone(this);
        }
    }
    return false;
}

void JIDSearch::createContact(const QString &name, unsigned tmpFlags, Contact *&contact)
{
    QString resource;
    if (m_client->findContact(name, QString::null, false, contact, resource))
        return;
    if (m_client->findContact(name, QString::null, true, contact, resource, false) == NULL)
        return;
    contact->setFlags(contact->getFlags() | tmpFlags);
}

#if 0
i18n("User")
i18n("Full Name")
i18n("Middle Name")
i18n("Family Name")
i18n("email")
i18n("Birthday")
i18n("Organization Name")
i18n("Organization Unit")
#endif

/*
#ifndef NO_MOC_INCLUDES
#include "jidsearch.moc"
#endif
*/


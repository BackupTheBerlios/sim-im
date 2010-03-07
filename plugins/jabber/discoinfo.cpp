/***************************************************************************
                          discoinfo.cpp  -  description
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

#include <QPixmap>
#include <QLineEdit>
#include <QTabWidget>
#include <QPushButton>
#include <QResizeEvent>

#include "icons.h"
#include "simgui/listview.h"
#include "misc.h"

#include "jabberclient.h"
#include "discoinfo.h"
#include "jabberbrowser.h"
#include "jabber.h"
#include "jabberaboutinfo.h"

using namespace SIM;

extern DataDef jabberUserData[];

DiscoInfo::DiscoInfo(JabberBrowser *browser, const QString &features,
                     const QString &name, const QString &type, const QString &category) : QDialog(browser)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose,true);
    m_browser = browser;
    SET_WNDPROC("jbrowser")
    setWindowIcon(Icon("Jabber_online"));
    setTitle();
    setButtonsPict(this);
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
    m_bTime    = true;
    m_bLast	   = true;
    m_bStat	   = true;
    m_bVCard   = true;
    m_about    = NULL;
    m_features = features;
    m_name	   = name;
    m_type	   = type;
    m_category = category;
    load_data(jabberUserData, &m_data, NULL);
    disableWidget(edtJName);
    disableWidget(edtType);
    disableWidget(edtCategory);
    edtNameSpace->setReadOnly(true);
    disableWidget(edtName);
    disableWidget(edtVersion);
    disableWidget(edtSystem);
    disableWidget(edtTime);
    disableWidget(edtLast);
    lstStat->addColumn(i18n("Name"));
    lstStat->addColumn(i18n("Units"));
    lstStat->addColumn(i18n("Value"));
    lstStat->setExpandingColumn(2);
    btnUrl->setIcon(Icon("home"));
    connect(btnUrl, SIGNAL(clicked()), this, SLOT(goUrl()));
    connect(edtUrl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
}

DiscoInfo::~DiscoInfo()
{
    free_data(jabberUserData, &m_data);
    m_browser->m_info = NULL;
}

void DiscoInfo::setTitle()
{
    setWindowTitle(m_url);
}

void DiscoInfo::reset()
{
    if (m_about){
        tabInfo->removeTab(tabInfo->indexOf(m_about));
        delete m_about;
        m_about = NULL;
    }
    if (m_browser->m_list->currentItem()){
        m_url  = m_browser->m_list->currentItem()->text(COL_JID);
        m_node = m_browser->m_list->currentItem()->text(COL_NODE);
    }
    free_data(jabberUserData, &m_data);
    load_data(jabberUserData, &m_data, NULL);
    m_data.ID.str() = m_url;
    m_data.Node.str() = m_node;
    setTitle();
    edtJName->setText(m_name);
    edtType->setText(m_type);
    edtCategory->setText(m_category);
    edtNameSpace->setPlainText(m_features);
    bool bTime    = false;
    bool bLast	  = false;
    bool bStat	  = false;
    bool bVCard	  = false;
    QString mf = m_features;
    while (!mf.isEmpty()){
        QString f = getToken(mf, '\n');
        if (f == "jabber:iq:time")
            bTime = true;
        if (f == "jabber:iq:last")
            bLast = true;
        if (f == "http://jabber.org/protocol/stats")
            bStat = true;
        if (f == "vcard-temp")
            bVCard = true;
    }
    int pos = 2;
    edtName->setText(QString::null);
    edtVersion->setText(QString::null);
    edtSystem->setText(QString::null);
    m_browser->m_client->versionInfo(m_url, m_node);
    if ((bTime || bLast) != (m_bTime || m_bLast)){
        m_bTime = bTime;
        m_bLast = bLast;
        if (m_bTime || m_bLast){
            tabInfo->insertTab(pos++, tabTime, i18n("&Time"));
        }else{
            tabInfo->removeTab(tabInfo->indexOf(tabTime));
        }
    }else if (m_bTime || m_bLast){
        pos++;
    }
    edtTime->setText(QString::null);
    edtLast->setText(QString::null);
    if (m_bTime){
        edtTime->show();
        m_browser->m_client->timeInfo(m_url, m_node);
    }else{
        edtTime->hide();
    }
    if (m_bLast){
        edtLast->show();
        m_browser->m_client->lastInfo(m_url, m_node);
    }else{
        edtLast->hide();
    }
    lstStat->clear();
    if (bStat != m_bStat){
        m_bStat = bStat;
        if (m_bStat){
            tabInfo->insertTab(pos++, tabStat, i18n("&Stat"));
        }else{
            tabInfo->removeTab(tabInfo->indexOf(tabStat));
        }
    }else if (m_bStat){
        pos++;
    }
    m_statId = m_bStat ? m_browser->m_client->statInfo(m_url, m_node) : QString::null;
    if (bVCard != m_bVCard){
        m_bVCard = bVCard;
        if (m_bVCard || m_bVCard){
            tabInfo->insertTab(pos++, tabVCard, i18n("&Info"));
        }else{
            tabInfo->removeTab(tabInfo->indexOf(tabVCard));
        }
    }else if (m_bVCard){
        pos++;
    }
    edtFirstName->setText(QString::null);
    edtNick->setText(QString::null);
    edtBirthday->setText(QString::null);
    edtUrl->setText(QString::null);
    edtEMail->setText(QString::null);
    edtPhone->setText(QString::null);
    if (bVCard){
        m_about = new JabberAboutInfo(tabInfo, &m_data, m_browser->m_client);
        tabInfo->insertTab(pos++, m_about, i18n("About info"));
        m_browser->m_client->info_request(&m_data, true);
    }
}

bool DiscoInfo::processEvent(Event *e)
{
    if (e->type() == eEventVCard){
        EventVCard *evc = static_cast<EventVCard*>(e);
        JabberUserData *data = evc->data();
        if (m_data.ID.str() == data->ID.str() && m_data.Node.str() == data->Node.str()){
            edtFirstName->setText(data->FirstName.str());
            edtNick->setText(data->Nick.str());
            edtBirthday->setText(data->Bday.str());
            edtUrl->setText(data->Url.str());
            urlChanged(edtUrl->text());
            edtEMail->setText(data->EMail.str());
            edtPhone->setText(data->Phone.str());
        }
    } else
    if (e->type() == eEventDiscoItem){
        EventDiscoItem *edi = static_cast<EventDiscoItem*>(e);
        DiscoItem *item = edi->item();
        if (m_statId == item->id){
            if (item->jid.isEmpty()){
                m_statId = QString::null;
                return true;
            }
            ListViewItem *i = new ListViewItem(lstStat);
            i->setText(0, item->jid);
            i->setText(1, item->name);
            i->setText(2, item->node);
            return true;
        }
    } else
    if (e->type() == eEventClientVersion){
        EventClientVersion *ecv = static_cast<EventClientVersion*>(e);
        ClientVersionInfo* info = ecv->info();
        if (m_data.ID.str() == info->jid && m_data.Node.str() == info->node){
            edtName->setText(info->name);
            edtVersion->setText(info->version);
            edtSystem->setText(info->os);
        }
    } else
    if (e->type() == eEventClientLastInfo){
        EventClientLastInfo *ecli = static_cast<EventClientLastInfo*>(e);
        ClientLastInfo* info = ecli->info();
        if (m_data.ID.str() == info->jid){
            unsigned ss = info->seconds;
            unsigned mm = ss / 60;
            ss -= mm * 60;
            unsigned hh = mm / 60;
            mm -= hh * 60;
            unsigned dd = hh / 24;
            hh -= dd * 24;
            QString date;
            if (dd){
                date  = i18n("%n day", "%n days", dd);
                date += ' ';
            }
            QString time;
            time.sprintf("%02u:%02u:%02u", hh, mm, ss);
            date += time;
            edtLast->setText(date);
        }
    } else
    if (e->type() == eEventClientTimeInfo){
        EventClientTimeInfo *ecti = static_cast<EventClientTimeInfo*>(e);
        ClientTimeInfo* info = ecti->info();
        if (m_data.ID.str() == info->jid){
          /*
            if (!info->display.isEmpty())
                edtTime->setText(info->display);
            else
          */
                edtTime->setText(info->utc);
        }
    }
    return false;
}

void DiscoInfo::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    lstStat->adjustColumn();
}

void DiscoInfo::accept()
{
    apply();
    QDialog::accept();
}

void DiscoInfo::apply()
{
    if (m_bVCard && m_about){
        m_about->apply(m_browser->m_client, &m_data);
        m_data.FirstName.str()  = edtFirstName->text();
        m_data.Nick.str()       = edtNick->text();
        m_data.Bday.str()       = edtBirthday->text();
        m_data.Url.str()        = edtUrl->text();
        m_data.EMail.str()      = edtEMail->text();
        m_data.Phone.str()      = edtPhone->text();
        m_browser->m_client->setClientInfo(&m_data);
    }
}

void DiscoInfo::goUrl()
{
    QString url = edtUrl->text();
    if (url.isEmpty())
        return;
    EventGoURL e(url);
    e.process();
}

void DiscoInfo::urlChanged(const QString &text)
{
    btnUrl->setEnabled(!text.isEmpty());
}

/*
#ifndef NO_MOC_INCLUDES
#include "discoinfo.moc"
#endif
*/


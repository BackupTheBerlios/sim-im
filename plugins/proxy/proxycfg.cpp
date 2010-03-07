/***************************************************************************
                          proxycfg.cpp  -  description
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

#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>
#include <QPixmap>

#include "icons.h"
#include "misc.h"
#include "socket/socket.h"
#include "socket/tcpclient.h"
#include "fetch.h"

#include "proxycfg.h"
#include "proxy.h"

using namespace SIM;

ProxyConfig::ProxyConfig(QWidget *parent, ProxyPlugin *plugin, QTabWidget *tab, ClientPtr client)
  : QWidget(parent)
  , m_client(client)
  , m_plugin(plugin)
  , m_current(~0U)
{
    setupUi(this);
    cmbType->addItem(i18n("None"));
    cmbType->addItem("SOCKS4");
    cmbType->addItem("SOCKS5");
    cmbType->addItem("HTTP/HTTPS");
    if (tab){
        tab->addTab(this, i18n("&Proxy"));
        for (QWidget *p = this; p; p = p->parentWidget()){
            QSize s  = p->sizeHint();
            QSize s1 = QSize(p->width(), p->height());
            p->setMinimumSize(s);
            p->resize(qMax(s.width(), s1.width()), qMax(s.height(), s1.height()));
            if (p->layout())
                p->layout()->invalidate();
            if (p == topLevelWidget())
                break;
        }
    }
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    connect(chkAuth, SIGNAL(toggled(bool)), this, SLOT(authToggled(bool)));
    if (m_client){
        lblClient->hide();
        cmbClient->hide();
        ProxyData data;
        plugin->clientData(static_cast<TCPClient*>(m_client.data()), data);
        fill(&data);
    }else{
        fillClients();
        connect(cmbClient, SIGNAL(activated(int)), this, SLOT(clientChanged(int)));
        clientChanged(0);
    }
}

void ProxyConfig::apply()
{
    if (m_client){
        ProxyData nd(NULL);
        get(&nd);
        nd.Client.str() = QString::null;
        if (getContacts()->nClients() <= 1){
            m_plugin->clearClients();
            m_plugin->data = nd;
            return;
        }
        ProxyData d;
        m_plugin->clientData(static_cast<TCPClient*>(m_client.data()), d);
        m_data.clear();
        if (d.Default.toBool()){
            d = nd;
        }else{
            d = m_plugin->data;
        }
        m_data.push_back(d);
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (client == m_client){
                nd.Client.str() = m_client->name();
                m_data.push_back(nd);
            }else{
                ProxyData d;
                m_plugin->clientData(static_cast<TCPClient*>(client), d);
                m_data.push_back(d);
            }
        }
    }else{
        clientChanged(0);
    }
    m_plugin->data = m_data[0];
    m_plugin->clearClients();
    unsigned nClient = 1;
    for (unsigned i = 1; i < m_data.size(); i++){
        if (m_data[i] == m_data[0])
            continue;
        m_plugin->setClients(nClient++, save_data(ProxyPlugin::proxyData, &m_data[i]));
    }
}

void ProxyConfig::typeChanged(int type)
{
    if (type){
        edtHost->show();
        edtPort->show();
        lblHost->show();
        lblPort->show();
    }else{
        edtHost->hide();
        edtPort->hide();
        lblHost->hide();
        lblPort->hide();
    }
    if (type > 1){
        chkAuth->show();
        edtUser->show();
        edtPswd->show();
        lblUser->show();
        lblPswd->show();
    }else{
        chkAuth->hide();
        edtUser->hide();
        edtPswd->hide();
        lblUser->hide();
        lblPswd->hide();
    }
    authToggled(chkAuth->isChecked());
}

void ProxyConfig::authToggled(bool bState)
{
    edtUser->setEnabled(bState);
    edtPswd->setEnabled(bState);
    lblUser->setEnabled(bState);
    lblPswd->setEnabled(bState);
}

void ProxyConfig::clientChanged(int)
{
    if (m_current < m_data.size()){
        get(&m_data[m_current]);
        if (m_current == 0){
            for (unsigned i = 1; i < m_data.size(); i++){
                if (m_data[i].Default.toBool()){
                    QString client = m_data[i].Client.str();
                    m_data[i] = m_data[0];
                    m_data[i].Default.asBool() = true;
                    m_data[i].Client.str() = client;
                }else{
                    if (m_data[i] == m_data[0])
                        m_data[i].Default.asBool() = true;
                }
            }
        }else{
            ProxyData &d = m_data[m_current];
            d.Default.asBool() = (d == m_data[0]);
        }
    }
    m_current = cmbClient->currentIndex();
    if (m_current < m_data.size())
        fill(&m_data[m_current]);
}

bool ProxyConfig::processEvent(Event *e)
{
    if ((m_client == NULL) && (e->type() == eEventClientsChanged))
        fillClients();
    return false;
}

void ProxyConfig::fillClients()
{
    m_current = (unsigned)(-1);
    m_data.clear();
    cmbClient->clear();
    cmbClient->insertItem(INT_MAX,i18n("Default"));
    ProxyData d(m_plugin->data);
    d.Clients.clear();
    m_data.push_back(d);
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (client->protocol()->description()->flags & PROTOCOL_NOPROXY)
            continue;
        QString name = client->name();
        int pos = name.indexOf(".");
        if (pos > 0)
            name = name.replace(pos, 1, " ");
        cmbClient->addItem(Icon(client->protocol()->description()->icon), name);
        ProxyData d;
        m_plugin->clientData(static_cast<TCPClient*>(client), d);
        m_data.push_back(d);
    }
    bool bState;
    if (!get_connection_state(bState)){
        cmbClient->insertItem(INT_MAX,i18n("HTTP requests"));;
        ProxyData d;
        m_plugin->clientData((TCPClient*)(-1), d);
        m_data.push_back(d);
    }
    clientChanged(0);
}

void ProxyConfig::fill(ProxyData *data)
{
    cmbType->setCurrentIndex(data->Type.toULong());
    edtHost->setText(data->Host.str());
    edtPort->setValue(data->Port.toULong());
    chkAuth->setChecked(data->Auth.toBool());
    edtUser->setText(data->User.str());
    edtPswd->setText(data->Password.str());
    typeChanged(data->Type.toULong());
    chkNoShow->setChecked(data->NoShow.toBool());
}

void ProxyConfig::get(ProxyData *data)
{
    data->Type.asULong() = cmbType->currentIndex();
    data->Host.str()     = edtHost->text();
    data->Port.asULong() = edtPort->text().toULong();
    data->Auth.asBool()  = chkAuth->isChecked();
    data->User.str()     = edtUser->text();
    data->Password.str() = edtPswd->text();
    data->NoShow.asBool()= chkNoShow->isChecked();
    data->bInit = true;
}

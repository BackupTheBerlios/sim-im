/***************************************************************************
                          newprotocol.cpp  -  description
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

#include "newprotocol.h"

#include "icons.h"

#include "profilemanager.h"
#include "newprotocol.h"
#include "connectwnd.h"
#include "core.h"
#include "contacts/client.h"
#include "contacts/protocolmanager.h"
#include "log.h"

#include <QPixmap>
#include <QPushButton>
#include <QComboBox>
#include <QTimer>

using namespace std;
using namespace SIM;

//static bool cmp_protocol(Protocol *p1, Protocol *p2)
//{
//    const CommandDef *cmd1 = p1->description();
//    const CommandDef *cmd2 = p2->description();
//    QString s1 = i18n(cmd1->text);
//    QString s2 = i18n(cmd2->text);
//    return s1 < s2;
//}

NewProtocol::NewProtocol(QWidget *parent, int default_protocol, bool bConnect) : QWizard(parent)
{
    setupUi(this);
    m_setup  = NULL;
    m_last   = NULL;
    m_bConnected = false;
    m_bConnect = false;
    m_bStart   = (parent == NULL);
    setWindowIcon(Icon("configure"));
    setButtonsPict(this);

    m_setupPage = new QWizardPage( this );
    m_setupLayout = new QHBoxLayout(m_setupPage);
    addPage(m_setupPage);

    m_connectWnd = new ConnectWnd(m_bStart);
    addPage(m_connectWnd); //, i18n(protocol->description()->text));
    if (m_bStart){
        m_last = new QWizardPage(this);
        addPage(m_last);//, i18n(protocol->description()->text));
    }

//    helpButton()->hide();

    QStringList plugins = getPluginManager()->enumPlugins();
    foreach(QString pluginname, plugins)
    {
        if(getPluginManager()->isPluginProtocol(pluginname))
            m_protocolPlugins.append(getPluginManager()->plugin(pluginname));
    }

    ProtocolPtr protocol;
    ProtocolIterator it;
    while ((protocol = ++it) != NULL){
        const CommandDef *cmd = protocol->description();
        if (cmd == NULL)
            continue;
        m_protocols.push_back(protocol);
    }
    //sort(m_protocols.begin(), m_protocols.end(), cmp_protocol);
    for (unsigned i = 0; i < m_protocols.size(); i++){
        const CommandDef *cmd = m_protocols[i]->description();
        cmbProtocol->addItem(Icon(cmd->icon), i18n(cmd->text));
    }
    connect(cmbProtocol, SIGNAL(activated(int)), this, SLOT(protocolChanged(int)));
    cmbProtocol->setCurrentIndex(default_protocol);
    protocolChanged(default_protocol);
    if (bConnect){
        next();
//        showPage(m_connectWnd);
//        pageChanged(NULL);
    }
    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
    log(L_DEBUG, "NewProtocol::NewProtocol()");
}

NewProtocol::~NewProtocol()
{
    if (m_connectWnd)
        delete m_connectWnd;
    if (m_setup)
        delete m_setup;
    
    // Protocol::plugin() returns raw Plugin pointer, we need smart
    SIM::ProfileManager::instance()->currentProfile()->enablePlugin(m_protocol->plugin()->name());
    log(L_DEBUG, "NewProtocol::~NewProtocol()");
}

void NewProtocol::protocolChanged(int n)
{
    if (m_setup){
        delete m_setup;
        m_setup = NULL;
    }
    if ((n < 0) || (n >= (int)(m_protocols.size())))
        return;
    ProtocolPtr protocol = m_protocols[n];
    m_protocol = protocol;
    m_client = protocol->createClient(NULL);
    if (m_client == NULL)
        return;
    m_setup = m_client->setupWnd();
    m_setup->setParent(m_setupPage);
    m_setupLayout->addWidget(m_setup);
    if (m_setup == NULL){
        m_client.clear();
        return;
    }
    connect(m_setup, SIGNAL(okEnabled(bool)), this, SLOT(okEnabled(bool)));
    connect(this, SIGNAL(apply()), m_setup, SLOT(apply()));
    m_setupPage->setTitle(i18n(protocol->description()->text));
    m_connectWnd->setTitle(i18n(protocol->description()->text));
    if(m_last)
    {
        m_last->setTitle(i18n(protocol->description()->text));
    }
//    setNextEnabled(currentPage(), true);
    setWindowIcon(Icon(protocol->description()->icon));
    EventRaiseWindow e(this);
    e.process();
}

void NewProtocol::okEnabled(bool bEnable)
{
//    setNextEnabled(m_setup, bEnable);
}

void NewProtocol::pageChanged(int id)
{
    if (currentPage() == m_connectWnd){
        emit apply();
        m_bConnect = true;
        unsigned status = CorePlugin::instance()->getManualStatus();
        if (status == STATUS_OFFLINE)
            status = STATUS_ONLINE;
        m_client->setStatus(status, false);
        m_connectWnd->setConnecting(true);
//        setBackEnabled(m_connectWnd, false);
//        setNextEnabled(currentPage(), false);
//        setFinishEnabled(m_connectWnd, false);
    }
    if (m_last && (currentPage() == m_last)){
//        setFinishEnabled(m_connectWnd, false);
//        cancelButton()->show();
//        backButton()->show();
//        finishButton()->hide();
//        showPage(protocolPage);
        protocolChanged(0);
    }
}

void NewProtocol::reject()
{
    if (m_bConnect){
        m_client->setStatus(STATUS_OFFLINE, false);
//        setBackEnabled(m_connectWnd, true);
        m_bConnect = false;
        back();
        return;
    }
    QWizard::reject();
}

void NewProtocol::loginComplete()
{
    if (m_client == NULL)
        return;
    m_bConnect = false;
    m_bConnected = true;
    m_client->setStatus(CorePlugin::instance()->getManualStatus(), true);
    m_connectWnd->setConnecting(false);
//    setNextEnabled(currentPage(), true);
//    setFinishEnabled(m_connectWnd, true);
    getContacts()->addClient(m_client.data());
    m_client.clear();
//    cancelButton()->hide();
//    backButton()->hide();
    EventSaveState e;
    e.process();
    accept();
}

bool NewProtocol::processEvent(Event *e)
{
    if (m_client == NULL || !m_bConnect)
        return false;

    switch (e->type()){
        case eEventClientChanged:
            if (m_client->getState() == Client::Connected){
                QTimer::singleShot(0, this, SLOT(loginComplete()));
                return false;
            }
            break;
        case eEventClientNotification: {
            EventClientNotification *ee = static_cast<EventClientNotification*>(e);
            const EventNotification::ClientNotificationData &d = ee->data();
            if (d.client == m_client){
                m_connectWnd->setErr(i18n(d.text),
                    (d.code == AuthError) ? m_client->protocol()->description()->accel : QString());
                m_bConnect = false;
                m_client->setStatus(STATUS_OFFLINE, false);
//                setBackEnabled(m_connectWnd, true);
//                setFinishEnabled(m_connectWnd, false);
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}
// vim: set expandtab:



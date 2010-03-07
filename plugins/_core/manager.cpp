/***************************************************************************
                          manager.cpp  -  description
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

#include "icons.h"
#include "manager.h"
#include "newprotocol.h"
#include "connectionsettings.h"
#include "core.h"
#include "contacts/client.h"

#include <QListWidget>
#include <QScrollBar>
#include <QPushButton>
#include <QResizeEvent>
#include <QCloseEvent>

using namespace SIM;

ConnectionManager::ConnectionManager(bool bModal)
  : QDialog(NULL)
{
    setupUi(this);
    setObjectName("manager");
    SET_WNDPROC("manager")
    setWindowIcon(Icon("configure"));
    setButtonsPict(this);
    lstConnection->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fill();
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClient()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeClient()));
    connect(btnUp, SIGNAL(clicked()), this, SLOT(upClient()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(downClient()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateClient()));
    connect(lstConnection, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    m_bModal = bModal;
}

void ConnectionManager::fill(Client *current)
{
    lstConnection->clear();
    QListWidgetItem *curItem = NULL;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        Protocol *protocol = client->protocol();
        const CommandDef *descr = protocol->description();
        QString text = CorePlugin::instance()->clientName(client);
        QListWidgetItem *item = new QListWidgetItem(text, lstConnection);
        if (descr)
            item->setIcon(Pict(descr->icon));
        if (current == client)
            curItem = item;
    }
    if (curItem)
        lstConnection->setCurrentItem(curItem);
    selectionChanged();
}

void ConnectionManager::selectionChanged()
{
    QListWidgetItem *item = lstConnection->currentItem();
    if (item == NULL){
        btnUp->setEnabled(false);
        btnDown->setEnabled(false);
        btnRemove->setEnabled(false);
        btnUpdate->setEnabled(false);
        return;
    }
    btnUpdate->setEnabled(true);
    btnRemove->setEnabled(true);
    int index = lstConnection->row(item);
    btnUp->setEnabled(index > 0);
    btnDown->setEnabled(index < lstConnection->count());
}

void ConnectionManager::closeEvent(QCloseEvent *e)
{
    QDialog::closeEvent(e);
    emit finished();
}

void ConnectionManager::addClient()
{
    NewProtocol pDlg(this);
    if (pDlg.exec())
        fill();
}

Client *ConnectionManager::currentClient()
{
    QListWidgetItem *item = lstConnection->currentItem();
    if (item == NULL)
        return NULL;
    int index = lstConnection->row(item);
    if (index == -1)
        return NULL;
    return getContacts()->getClient(index);
}

void ConnectionManager::removeClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    delete client;
    fill();
}

void ConnectionManager::upClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    getContacts()->moveClient(client, true);
    fill(client);
}

void ConnectionManager::downClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    getContacts()->moveClient(client, false);
    fill(client);
}

void ConnectionManager::updateClient()
{
    Client *client = currentClient();
    if (client == NULL)
        return;
    unsigned n;
    for (n = 0; n < getContacts()->nClients(); n++){
        if (getContacts()->getClient(n) == client)
            break;
    }
    if (n >= getContacts()->nClients())
        return;
    Command cmd;
    cmd->id = CmdSetup;
    cmd->menu_id = CmdClient + n;
    EventCommandExec(cmd).process();
}

/*
#ifndef NO_MOC_INCLUDES
#include "manager.moc"
#endif
*/


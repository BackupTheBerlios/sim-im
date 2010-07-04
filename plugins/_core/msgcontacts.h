/***************************************************************************
                          msgcontacts.h  -  description
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

#ifndef _MSGCONTACTS_H
#define _MSGCONTACTS_H

#include <QObject>
#include <QString>
#include <QTreeWidget>

#include "event.h"

class MsgEdit;
class UserList;
namespace SIM
{
    class ContactList;
}

class MsgContacts : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgContacts(MsgEdit *btn, SIM::Message *msg, SIM::ContactList* cl);
    ~MsgContacts();
protected slots:
    void init();
    void changed();
    void editFinished();
    void listFinished();
protected:
    virtual bool processEvent(SIM::Event*);
    void fillContactList(QTreeWidget* tree);
    void setCheckedState(QTreeWidget* tree, int contact_id, bool checked);
    QList<unsigned int> checkedContacts(QTreeWidget* tree);
    QTreeWidget* m_list;
    MsgEdit    *m_edit;
    QString     m_client;
    static const int ContactIdRole = Qt::UserRole + 1;
    SIM::ContactList* m_contactList;
};

#endif


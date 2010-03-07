/***************************************************************************
                          userlist.h  -  description
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

#ifndef _USERLIST_H
#define _USERLIST_H

#include <list>
#include "simgui/listview.h"
#include <QMouseEvent>

using namespace std;

class UserListBase;
class QTimer;
class UserViewDelegate;

const unsigned DIV_ITEM = 0;
const unsigned GRP_ITEM = 1;
const unsigned USR_ITEM = 2;

const unsigned DIV_ONLINE  = 0;
const unsigned DIV_OFFLINE = 1;

const unsigned CONTACT_TEXT		= 1;
const unsigned CONTACT_ICONS	= 2;
const unsigned CONTACT_ACTIVE	= 3;
const unsigned CONTACT_STATUS	= 4;

namespace SIM {
    enum ItemDataRole {
        ExtraIconsRole = Qt::UserRole + 0,
    };
}

class UserViewItemBase : public ListViewItem
{
public:
    UserViewItemBase(UserListBase *view);
    UserViewItemBase(UserViewItemBase *parent);
    virtual unsigned type() = 0;
    virtual void setup();
protected:
    void setCheckable( bool bCheckable );
};

class DivItem : public UserViewItemBase
{
public:
    DivItem(UserListBase *view, unsigned type);
    unsigned type() { return DIV_ITEM; }
    unsigned state() { return m_type; }
protected:
    unsigned m_type;
    friend class UserListBase;
    friend class UserViewDelegate;
    virtual QVariant data( int column, int role ) const;
};

class GroupItem : public UserViewItemBase
{
public:
    GroupItem(UserListBase *view, SIM::Group *grp, bool bOffline, bool bCheckable );
    GroupItem(UserViewItemBase *view, SIM::Group *grp, bool bOffline, bool bCheckable );
    unsigned type() { return GRP_ITEM; }
    unsigned long id() const { return m_id; }
    void update(SIM::Group *grp, bool bInit=false);
    unsigned m_nContacts;
    unsigned m_nContactsOnline;
    unsigned m_unread;
    virtual QVariant data( int column, int role ) const;
    virtual void setData( int column, int role, const QVariant &value );
protected:
    virtual void setOpen(bool bOpen);
    void init(SIM::Group *grp);
    unsigned long m_id;
    bool m_bOffline;
};

class ContactItem : public UserViewItemBase
{
public:
    ContactItem(UserViewItemBase *view, SIM::Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread, bool bCheckable );
    unsigned type() { return USR_ITEM; }
    unsigned long id() { return m_id; }
    unsigned style() { return m_style; }
    unsigned status() { return m_status; }
    bool update(SIM::Contact *grp, unsigned status, unsigned style, const QString &icons, unsigned unread);
    bool m_bOnline;
    bool m_bBlink;
    unsigned m_unread;
    virtual QVariant data( int column, int role ) const;
protected:
    virtual QString key(int column/*, bool ascending*/) const;
    void init(SIM::Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread);
    unsigned long m_id;
    unsigned m_style;
    unsigned m_status;
    QIcon m_Icon;
    QString m_sExtraIcons;
    SIM::Contact *contact;
};

class UserListBase : public ListView
{
    Q_OBJECT
public:
    UserListBase(QWidget *parent);
    ~UserListBase();
    virtual void fill();
protected slots:
    void drawUpdates();
protected:
    unsigned m_groupMode;
    unsigned m_bShowOnline;
    unsigned m_bShowEmpty;
    virtual bool processEvent(SIM::Event*);
    unsigned getUserStatus(SIM::Contact *contact, unsigned &style, QString &icons);
    virtual unsigned getUnread(unsigned contact_id);
    GroupItem *findGroupItem(unsigned id, ListViewItem *p = NULL);
    ContactItem *findContactItem(unsigned id, ListViewItem *p = NULL);
    void addSortItem(ListViewItem *item);
    void addUpdatedItem(ListViewItem *item);
    void addGroupForUpdate(unsigned long id);
    void addContactForUpdate(unsigned long id);
    virtual void deleteItem(ListViewItem *item);
    std::list<ListViewItem*> sortItems;
    std::list<ListViewItem*> updatedItems;
    std::list<unsigned long>	updGroups;
    std::list<unsigned long>	updContacts;
    bool m_bDirty;
    bool m_bInit;
    QTimer *updTimer;
    friend class UserViewItemBase;
    bool m_bCheckable;
	ContactItem *m_contactItem;
};

class UserList
    : public UserListBase
{
    Q_OBJECT
public:
    UserList( QWidget *parent );
    virtual ~UserList();

    void select( unsigned int id );
    bool isHaveSelected();
    QList< unsigned int > selected();

signals:
    void selectChanged();

protected:
    QList< unsigned int > selected( QTreeWidgetItem *pItem );
};

#endif

// vim: set expandtab:

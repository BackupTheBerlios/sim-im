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
#include "contacts/contact.h"
#include "contacts/group.h"
#include "core.h"
#include <QMouseEvent>
#include <QTimer>
#include <QSharedPointer>
#include <QMimeData>
#include <QTreeWidget>
using namespace std;
using namespace SIM;


const unsigned long MenuListView    = 0x100;
const unsigned long CmdListDelete   = 0x100;

class QTimer;

class CORE_EXPORT ListView : public QTreeWidget
{
    Q_OBJECT
public:
    ListView(QWidget *parent);
    virtual ~ListView();

//    void setAcceptDrop(bool bAccept);
//    void setMenu(unsigned long menuId);
//    QTreeWidgetItem* currentItem();
//    QTreeWidgetItem* itemAt(const QPoint& p);
//    QTreeWidgetItem* firstChild();
//    void addColumn(const QString& name);

//    void repaint(QTreeWidgetItem* item);

//signals:
//    void clickItem(QTreeWidgetItem*);
//    void deleteItem(QTreeWidgetItem*);

//protected:
//    virtual bool getMenu(QTreeWidgetItem *item, unsigned long &id, void *&param);
//    virtual bool processEvent(SIM::Event *e);
//    virtual bool eventFilter(QObject*, QEvent*);
//    virtual void resizeEvent(QResizeEvent*);
//    virtual QMimeData *dragObject();
//    void viewportContextMenuEvent( QContextMenuEvent *e);
//    void viewportMousePressEvent(QMouseEvent *e);
//    void mousePressEvent(QMouseEvent *e);
//    void mouseMoveEvent(QMouseEvent *e);
//    void mouseReleaseEvent(QMouseEvent *e);
//    void dragEnterEvent(QDragEnterEvent *e);
//    void dragMoveEvent(QDragMoveEvent *e);
//    void dropEvent(QDropEvent *e);
//    void keyPressEvent(QKeyEvent *e);
//    void showPopup(QTreeWidgetItem *item, QPoint p);
//    void contextMenuEvent(QContextMenuEvent* e);

//    unsigned long m_menuId;
//    QTimer	 *m_resizeTimer;
//    bool m_bAcceptDrop;
//    static bool s_bInit;
//    QTreeWidgetItem *m_pressedItem;
};

//class EXPORT ContactDragObject : public QMimeData
//{
//    Q_OBJECT
//public:
//    ContactDragObject( SIM::Contact *contact );
//    ~ContactDragObject();
//    static bool canDecode(QMimeSource*);
//    static SIM::Contact *decode(QMimeSource*);
//protected:
//    unsigned long m_id;
//};


class UserListBase;
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

//class UserViewItemBase : public QTreeWidgetItem
//{
//public:
//    UserViewItemBase(UserListBase *view, int type);
//    UserViewItemBase(UserViewItemBase *parent, int type);
//    virtual void setup();
//protected:
//    void setCheckable( bool bCheckable );
//};

//class DivItem : public UserViewItemBase
//{
//public:
//    DivItem(UserListBase *view, unsigned type);
//    virtual int type() { return DIV_ITEM; }
//    unsigned state() { return m_type; }
//protected:
//    unsigned m_type;
//    friend class UserListBase;
//    friend class UserViewDelegate;
//    virtual QVariant data( int column, int role ) const;
//};

//typedef QSharedPointer<DivItem> DivItemPtr;

//class GroupItem : public UserViewItemBase
//{
//public:
//    GroupItem(UserListBase *view, SIM::Group *grp, bool bOffline, bool bCheckable );
//    GroupItem(UserViewItemBase *view, SIM::Group *grp, bool bOffline, bool bCheckable );
//    virtual int type() { return GRP_ITEM; }
//    unsigned long id() const { return m_id; }
//    void update(SIM::Group *grp, bool bInit=false);
//    unsigned m_nContacts;
//    unsigned m_nContactsOnline;
//    unsigned m_unread;
//    virtual QVariant data( int column, int role ) const;
//    virtual void setData( int column, int role, const QVariant &value );
//protected:
//    virtual void setOpen(bool bOpen);
//    void init(SIM::Group *grp);
//    unsigned long m_id;
//    bool m_bOffline;
//};
//typedef QSharedPointer<GroupItem> GroupItemPtr;

//class ContactItem : public UserViewItemBase
//{
//public:
//    ContactItem(UserViewItemBase *view, SIM::Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread, bool bCheckable );
//    virtual int type() { return USR_ITEM; }
//    unsigned long id() { return m_id; }
//    unsigned style() { return m_style; }
//    unsigned status() { return m_status; }
//    bool update(SIM::Contact *grp, unsigned status, unsigned style, const QString &icons, unsigned unread);
//    bool m_bOnline;
//    bool m_bBlink;
//    unsigned m_unread;
//    virtual QVariant data( int column, int role ) const;
//protected:
//    virtual QString key(int column/*, bool ascending*/) const;
//    void init(SIM::Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread);
//    unsigned long m_id;
//    unsigned m_style;
//    unsigned m_status;
//    QIcon m_Icon;
//    QString m_sExtraIcons;
//    SIM::Contact *contact;
//    friend class UserViewDelegate;
//};
//typedef QSharedPointer<ContactItem> ContactItemPtr;

//class UserListBase : public ListView
//{
//    Q_OBJECT
//public:
//    UserListBase(QWidget *parent);
//    ~UserListBase();
//    virtual void fill();
//    void select(unsigned int id);
//    QList<unsigned int> selected();
//    bool isHaveSelected();

//protected slots:
//    void drawUpdates();
//    bool updateGroups();
//    bool updateContacts();
//    bool updateContactNoGroups(SIM::Contact* contact);
//    bool updateContactGroupMode1(SIM::Contact* contact);
//    bool updateContactGroupMode2(SIM::Contact* contact);

//    bool removeContactFromItem(unsigned long contactId, DivItem* item);
//    void refreshOnlineOfflineGroups();

//    void updateUnread();

//protected:
//    unsigned m_groupMode;
//    unsigned m_bShowOnline;
//    unsigned m_bShowEmpty;

//    virtual bool processEvent(SIM::Event*);
//    unsigned getUserStatus(SIM::Contact *contact, unsigned &style, QString &icons);
//    virtual unsigned getUnread(unsigned contact_id);
//    virtual unsigned getUnread(SIM::Contact *contact);
//    GroupItem *findGroupItem(unsigned id, QTreeWidgetItem *p = NULL);
//    ContactItem *findContactItem(unsigned id, QTreeWidgetItem *p = NULL);
//    ContactItem* getContactItem(Contact *contact);
//    GroupItem* getGrpItem(Group *g);
//    GroupItem* getGrpItem(Group *group, DivItem *divItem);
//    void addSortItem(QTreeWidgetItem *item);
//    void addUpdatedItem(QTreeWidgetItem *item);
//    void addGroupForUpdate(unsigned long id);
//    void addContactForUpdate(unsigned long id);
//    virtual void deleteItem(QTreeWidgetItem *item);

//    QList< unsigned int > selected( QTreeWidgetItem *pItem );

//    std::list<QTreeWidgetItem*> sortItems;
//    std::list<QTreeWidgetItem*> updatedItems;
//    std::list<ContactItem*> m_unreadItems;
//    std::list<unsigned long>	updGroups;
//    std::list<unsigned long>	updContacts;

//    bool m_bDirty;
//    bool m_bInit;
//    QTimer *updTimer;
//    QTimer m_unreadTimer;
//    friend class UserViewItemBase;
//    bool m_bCheckable;
//	ContactItem *m_contactItem;
//    DivItem* m_itemOnline;
//    DivItem* m_itemOffline;
//};

#endif

// vim: set expandtab:

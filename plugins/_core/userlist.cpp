/***************************************************************************
                          userlist.cpp  -  description
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

#include "userlist.h"
#include "core.h"
#include "icons.h"
#include "userview.h"

#include <QScrollBar>
#include <QTimer>
#include <QBitmap>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QAbstractButton>
#include <QScrollBar>

#include "log.h"
#include "contacts/contact.h"
#include "contacts/group.h"

using namespace std;
using namespace SIM;

UserViewItemBase::UserViewItemBase(UserListBase *parent)
    : ListViewItem(parent)
{
}

UserViewItemBase::UserViewItemBase(UserViewItemBase *parent)
    : ListViewItem(parent)
{
}

void UserViewItemBase::setup()
{
}

void UserViewItemBase::setCheckable( bool bCheckable ) {
    if( bCheckable ) {
        setFlags( flags() | Qt::ItemIsUserCheckable );
        setCheckState( 0, Qt::Unchecked );
    }
    else {
        setFlags( flags() & ~Qt::ItemIsUserCheckable );
    }
}

DivItem::DivItem(UserListBase *view, unsigned type)
    : UserViewItemBase(view)
{
    m_type = type;
    setText(0, QString::number(m_type));
    setExpandable(true);
    //setSelectable(false);
}

QVariant DivItem::data( int column, int role ) const
{
    QVariant result;

    switch( role )
    {
        case Qt::DisplayRole : {
            QString text;
            switch( m_type )
            {
                case DIV_ONLINE:
                    text = i18n("Online");
                    break;
                case DIV_OFFLINE:
                    text = i18n("Offline");
                    break;
            }
            result = QVariant( text );
            break;
        }
        default :
            return UserViewItemBase::data( column, role );
    }

    return result;
}

GroupItem::GroupItem(UserListBase *view, Group *grp, bool bOffline, bool bCheckable )
    : UserViewItemBase(view)
{
    m_id = grp->id();
    m_bOffline = bOffline;
    init(grp);
    setCheckable( bCheckable );
}

GroupItem::GroupItem( UserViewItemBase *view, Group *grp, bool bOffline, bool bCheckable )
    : UserViewItemBase(view)
{
    m_id = grp->id();
    m_bOffline = bOffline;
    init(grp);
    setCheckable( bCheckable );
}

void GroupItem::init(Group *grp)
{
    m_unread = 0;
    m_nContacts = 0;
    m_nContactsOnline = 0;
    setExpandable(true);
    //setSelectable(true);
    SIM::PropertyHubPtr data = grp->getUserData("list");
    if (data.isNull()){
        setOpen(true);
    }else{
        if (m_bOffline){
            setOpen(data->value("OfflineOpen").toBool());
        }else{
            setOpen(data->value("OnlineOpen").toBool());
        }
    }
    update(grp, true);
}

void GroupItem::update(Group *grp, bool bInit)
{
    QString s;
    s = "A";
    if (grp->id()){
        s = QString::number(getContacts()->groupIndex(grp->id()));
        while (s.length() < 12){
            s = QString("0") + s;
        }
    }
    if (s == text(0))
        return;
    setText(0, s);
    if (bInit)
        return;
    ListViewItem *p = static_cast<ListViewItem*>(parent());
    if (p){
        //p->sort();
        return;
    }
    //listView()->sort();
}

void GroupItem::setOpen(bool bOpen)
{
    //UserViewItemBase::setOpen(bOpen);
    Group *grp = getContacts()->group(m_id);
    if (grp){
        SIM::PropertyHubPtr data = grp->getUserData("list", !bOpen);
        if (!data.isNull()){
            if (m_bOffline){
                data->setValue("OfflineOpen", bOpen);
            }else{
                data->setValue("OnlineOpen", bOpen);
            }
        }
    }
}

QVariant GroupItem::data( int column, int role ) const {
    QVariant result;

    switch( role )
    {
        case Qt::DisplayRole : {
            QString text;
            if (id()){
                Group *grp = getContacts()->group(id());
                if (grp){
                    text = grp->getName();
                }else{
                    text = "???";
                }
            }else{
                text = i18n("Not in list");
            }
            if (m_nContacts){
                text += " (";
                if (m_nContactsOnline){
                    text += QString::number(m_nContactsOnline);
                    text += '/';
                }
                text += QString::number(m_nContacts);
                text += ')';
            }
            result = QVariant( text );
            break;
        }
        default :
            return UserViewItemBase::data( column, role );
    }

    return result;
}

void GroupItem::setData( int column, int role, const QVariant &value ) {
    if( Qt::CheckStateRole == role ) {
        Qt::CheckState cs = (Qt::CheckState)value.toInt();
        for( int i = 0 ; i < childCount() ; i++ ) {
            child( i )->setCheckState( 0, cs );
        }
    }

    UserViewItemBase::setData( column, role, value );
}

ContactItem::ContactItem( UserViewItemBase *view, Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread, bool bCheckable )
    : UserViewItemBase(view)
{
    m_id = contact->id();
    init(contact, status, style, icons, unread);
    setExpandable(false);
    setCheckable( bCheckable );
    setFlags( flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable );
}

void ContactItem::init(Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread)
{
    m_bOnline    = false;
    m_bBlink	 = false;
    update(contact, status, style, icons, unread);
}

bool ContactItem::update(Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread)
{
    m_unread = unread;
    m_style  = style;
    m_status = status;
    QString active;
    active.sprintf("%08lX", (long unsigned int)(0xFFFFFFFF - contact->getLastActive()));
    m_sExtraIcons = icons;
    QString icon = getToken(m_sExtraIcons, ',');
    m_Icon = Icon(icon);
    setText(CONTACT_ICONS, icons);
    setText(CONTACT_ACTIVE, active);
    setText(CONTACT_STATUS, QString::number(9 - status));
    setup();
    return true;
}

QString ContactItem::key(int column/*, bool ascending */) const //Fixme?
{
    if (column == 0){
        unsigned mode = CorePlugin::instance()->value("SortMode").toUInt();
        QString res;
        for (;;){
            int n = 0;
            switch (mode & 0xFF){
            case SORT_STATUS:
                n = CONTACT_STATUS;
                break;
            case SORT_ACTIVE:
                n = CONTACT_ACTIVE;
                break;
            case SORT_NAME:
                n = CONTACT_TEXT;
                break;
            }
            if (n == 0)
                break;
            res += text(n).toLower();
            mode = mode >> 8;
        }
        return res;
    }
    return QString::null; //UserViewItemBase::key(column, ascending);
}

QVariant ContactItem::data( int column, int role ) const
{
    Contact *contact = getContacts()->contact( m_id );
    if( NULL == contact )
        return QVariant();

    QVariant result;

    switch( role )
    {
        case Qt::DisplayRole : {
            result = QVariant( contact->getName() );
            break;
        }
        case Qt::DecorationRole : {
            QIcon icon = m_Icon;
            UserView* uv = dynamic_cast<UserView*>( treeWidget() );
            if( m_unread && uv->m_bUnreadBlink ) {
                CommandDef *def = CorePlugin::instance()->messageTypes.find( m_unread );
                if (def)
                    icon = Icon( def->icon );
            }
            result = QVariant( icon );
            break;
        }
        case Qt::ToolTipRole : {
            result = QVariant( contact->tipText() );
            break;
        }
        case SIM::ExtraIconsRole : {
            result = QVariant( m_sExtraIcons );
            break;
        }
        default :
            return UserViewItemBase::data( column, role );
    }

    return result;
}

UserListBase::UserListBase(QWidget *parent)
    : ListView(parent)
	, m_bInit		(false)
    , m_bDirty		(false)
    , m_groupMode	(1)
    , m_bShowOnline (false)
    , m_bShowEmpty  (false)
    , m_bCheckable	(false)
	, updTimer		(new QTimer(this))
	, m_contactItem	(NULL)
{
    //header()->hide();
    addColumn("");
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //setSorting(0); //Fixme
    connect(updTimer, SIGNAL(timeout()), this, SLOT(drawUpdates()));

    setExpandingColumn(0);
}

UserListBase::~UserListBase()
{
}

void UserListBase::drawUpdates()
{
    m_bDirty = false;
    updTimer->stop();
    ListViewItem *item;
//    int x = viewport()->x();
//    int y = viewport()->y();
    viewport()->setUpdatesEnabled(false);
    bool bChanged = false;
    list<unsigned long>::iterator it;
    for (it = updGroups.begin(); it != updGroups.end(); ++it){
        Group *group = getContacts()->group(*it);
        if (group == NULL)
            continue;
        switch (m_groupMode){
        case 1:
            item = findGroupItem(group->id());
            if (item){
                if (!m_bShowEmpty && (item->child(0) == NULL)){
                    delete item;
                    bChanged = true;
                }else{
                    static_cast<GroupItem*>(item)->update(group);
                    addUpdatedItem(item);
                }
            }else{
                if (m_bShowEmpty){
                    new GroupItem( this, group, true, m_bCheckable );
                    bChanged = true;
                }
            }
            break;
        case 2:
            for(int c = 0; c < topLevelItemCount(); c++)
            {
                item = static_cast<ListViewItem*>(topLevelItem(c));
                UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
                if (i->type() != DIV_ITEM) continue;
                DivItem *divItem = static_cast<DivItem*>(i);
                GroupItem *grpItem = findGroupItem(group->id(), divItem);
                if (grpItem){
                    if (!m_bShowEmpty && (item->child(0) == NULL)){
                        delete grpItem;
                        bChanged = true;
                    }else{
                        grpItem->update(group);
                        addUpdatedItem(grpItem);
                    }
                }else{
                    if (m_bShowEmpty){
                        new GroupItem( divItem, group, divItem->state() == DIV_OFFLINE, m_bCheckable );
                        bChanged = true;
                    }
                }
            }
            break;
        }
    }
    updGroups.clear();
    DivItem *itemOnline  = NULL;
    DivItem *itemOffline = NULL;
    if (updContacts.size()){
        if (m_groupMode != 1){
            for(int c = 0; c < topLevelItemCount(); c++)
            {
                item = static_cast<ListViewItem*>(topLevelItem(c));
                UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
                if (i->type() != DIV_ITEM) continue;
                DivItem *divItem = static_cast<DivItem*>(i);
                if (divItem->state() == DIV_ONLINE)
                    itemOnline = divItem;
                if (divItem->state() == DIV_OFFLINE)
                    itemOffline = divItem;
            }
        }
    }
    for (it = updContacts.begin(); it != updContacts.end(); ++it){ //Fixme, got crash: list operator not incremenable!
        Contact *contact = getContacts()->contact(*it);
        if (contact == NULL)
            continue;
        GroupItem *grpItem;
        unsigned style;
        QString icons;
        unsigned status = getUserStatus(contact, style, icons);
        unsigned unread = getUnread(contact->id());
        bool bShow = false;
        SIM::PropertyHubPtr data = contact->getUserData("list");
        if (!data.isNull() && data->value("ShowAlways").toBool())
            bShow = true;
        switch (m_groupMode){
        case 0:
            if (status <= STATUS_OFFLINE){
                if (itemOnline){
                    m_contactItem = findContactItem(contact->id(), itemOnline);
                    if (m_contactItem){
                        deleteItem(m_contactItem); //<== crash
                        bChanged = true;
                        if (itemOnline->child(0) == NULL){
                            deleteItem(itemOnline);
                            itemOnline = NULL;
                        }
                    }
                }
                if ((unread == 0) && !bShow && m_bShowOnline){
                    if (itemOffline){
                        m_contactItem = findContactItem(contact->id(), itemOffline);
                        if (m_contactItem){
                            deleteItem(m_contactItem);
                            bChanged = true;
                            if (itemOffline->child(0) == NULL){
                                deleteItem(itemOffline);
                                itemOffline = NULL;
                            }
                        }
                    }
                    break;
                }
                if (itemOffline == NULL){
                    itemOffline = new DivItem(this, DIV_OFFLINE);
                    setOpen(itemOffline, true);
                    bChanged = true;
                }
                m_contactItem = findContactItem(contact->id(), itemOffline);
                if (m_contactItem)
				{
                    if (m_contactItem->update(contact, status, style, icons, unread))
                        addSortItem(itemOffline);
                    addUpdatedItem(m_contactItem);
                }
				else
				{
                    m_contactItem = new ContactItem( itemOffline, contact, status, style, icons, unread, m_bCheckable );
                    bChanged = true;
                }
            }
			else
			{
                if (itemOffline)
				{
                    m_contactItem = findContactItem(contact->id(), itemOffline);
                    if (m_contactItem){
                        deleteItem(m_contactItem);
                        bChanged = true;
                        if (itemOffline->child(0) == NULL){
                            deleteItem(itemOffline);
                            itemOffline = NULL;
                        }
                    }
                }
                if (itemOnline == NULL){
                    itemOnline = new DivItem(this, DIV_ONLINE);
                    setOpen(itemOnline, true);
                    bChanged = true;
                }
                m_contactItem = findContactItem(contact->id(), itemOnline);
                if (m_contactItem)
				{
                    if (m_contactItem->update(contact, status, style, icons, unread))
                        addSortItem(itemOnline);
                    addUpdatedItem(m_contactItem);
                }
				else
				{
                    m_contactItem = new ContactItem( itemOnline, contact, status, style, icons, unread, m_bCheckable );
                    bChanged = true;
                }
            }
            break;
        case 1:
            m_contactItem = findContactItem(contact->id());
            grpItem = NULL;
            if (m_contactItem){
                grpItem = static_cast<GroupItem*>(m_contactItem->parent());
                if (((status <= STATUS_OFFLINE) && (unread == 0) && !bShow && m_bShowOnline) ||
                        (contact->getGroup() != (int)grpItem->id())){
                    grpItem->m_nContacts--;
                    if (m_contactItem->m_bOnline)
                        grpItem->m_nContactsOnline--;
                    addGroupForUpdate(grpItem->id());
                    deleteItem(m_contactItem);
                    bChanged = true;
                    if (!m_bShowEmpty && (grpItem->child(0) == NULL))
                        delete grpItem;
                    m_contactItem = NULL;
                    grpItem = NULL;
                }
            }
            if ((status > STATUS_OFFLINE) || unread || bShow || !m_bShowOnline){
                if (grpItem == NULL){
                    grpItem = findGroupItem(contact->getGroup());
                    if (grpItem == NULL){
                        Group *grp = getContacts()->group(contact->getGroup());
                        if (grp){
                            grpItem = new GroupItem( this, grp, true, m_bCheckable );
                            bChanged = true;
                        }
                    }
                }
                if (grpItem){
                    if (m_contactItem){
                        if (m_contactItem->update(contact, status, style, icons, unread))
                            addSortItem(grpItem);
                        addUpdatedItem(m_contactItem);
                        if (!m_bShowOnline &&
                                (m_contactItem->m_bOnline != (status > STATUS_OFFLINE))){
                            if (status <= STATUS_OFFLINE)
							{
                                grpItem->m_nContactsOnline--;
                                m_contactItem->m_bOnline = false;
                            }
							else
							{
                                grpItem->m_nContactsOnline++;
                                m_contactItem->m_bOnline = true;
                            }
                            addGroupForUpdate(grpItem->id());
                        }
                    }
					else
					{
                        bChanged = true;
                        m_contactItem = new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
                        grpItem->m_nContacts++;
                        if (!m_bShowOnline && (status > STATUS_OFFLINE))
						{
                            grpItem->m_nContactsOnline++;
                            m_contactItem->m_bOnline = true;
                        }
                        addGroupForUpdate(grpItem->id());
                    }
                }
            }
            break;
        case 2:
            m_contactItem = findContactItem(contact->id(), itemOnline);
            grpItem = NULL;
            if (m_contactItem){
                grpItem = static_cast<GroupItem*>(m_contactItem->parent());
                if ((status <= STATUS_OFFLINE) || ((int)grpItem->id() != contact->getGroup())){
                    grpItem->m_nContacts--;
                    addGroupForUpdate(grpItem->id());
                    deleteItem(m_contactItem);
                    bChanged = true;
                    if (!m_bShowEmpty && (grpItem->child(0) == NULL))
                        delete grpItem;
                    grpItem = NULL;
                    m_contactItem = NULL;
                }
            }
            if (itemOffline){
                m_contactItem = findContactItem(contact->id(), itemOffline);
                grpItem = NULL;
                if (m_contactItem){
                    grpItem = static_cast<GroupItem*>(m_contactItem->parent());
                    if ((status > STATUS_OFFLINE) || ((int)grpItem->id() != contact->getGroup())){
                        grpItem->m_nContacts--;
                        addGroupForUpdate(grpItem->id());
                        deleteItem(m_contactItem);
                        m_contactItem = NULL;
                        bChanged = true;
                        if (m_bShowOnline && (grpItem->child(0) == NULL)){
                            deleteItem(grpItem);
                            grpItem = NULL;
                            if (itemOffline->child(0) == NULL){
                                deleteItem(itemOffline);
                                itemOffline = NULL;
                            }
                        }
                    }
                }
            }
            if ((unread == 0) && !bShow && (status <= STATUS_OFFLINE) && m_bShowOnline)
                break;
            DivItem *divItem;
            if (status <= STATUS_OFFLINE)
			{
                if (itemOffline == NULL)
				{
                    bChanged = true;
                    itemOffline = new DivItem(this, DIV_OFFLINE);
                    setOpen(itemOffline, true);
                }
                divItem = itemOffline;
            }
			else
                divItem = itemOnline;

            grpItem = findGroupItem(contact->getGroup(), divItem);
            if (grpItem == NULL)
			{
                Group *grp = getContacts()->group(contact->getGroup());
                if (grp == NULL)
                    break;
                bChanged = true;
                grpItem = new GroupItem( divItem, grp, true, m_bCheckable );
                addSortItem(divItem);
            }
            m_contactItem = findContactItem(contact->id(), grpItem);
            if (m_contactItem)
			{
                if (m_contactItem->update(contact, status, style, icons, unread))
                    addSortItem(grpItem);
            }
			else
			{
                bChanged = true;
                new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
                grpItem->m_nContacts++;
                addGroupForUpdate(grpItem->id());
            }
        }
    }
    updContacts.clear();
    for (list<ListViewItem*>::iterator it_sort = sortItems.begin(); it_sort != sortItems.end(); ++it_sort){
        if ((*it_sort)->child(0) == NULL)
            continue;
        //(*it_sort)->sort();
        bChanged = true;
    }
    sortItems.clear();
    //center(x, y, 0, 0);
    viewport()->setUpdatesEnabled(true);
    if (bChanged){
        viewport()->repaint();
    }else{
        for (list<ListViewItem*>::iterator it = updatedItems.begin(); it != updatedItems.end(); ++it)
            (*it)->repaint();
    }
    updatedItems.clear();
}

const unsigned UPDATE_TIME = 800;

void UserListBase::addGroupForUpdate(unsigned long id)
{
    for (list<unsigned long>::iterator it = updGroups.begin(); it != updGroups.end(); ++it){
        if (*it == id)
            return;
    }
    updGroups.push_back(id);
    if (!m_bDirty){
        m_bDirty = true;
        updTimer->start(800);
    }
}

void UserListBase::addContactForUpdate(unsigned long id)
{
    for (list<unsigned long>::iterator it = updContacts.begin(); it != updContacts.end(); ++it){
        if (*it == id)
            return;
    }
    updContacts.push_back(id);
    if (!m_bDirty){
        m_bDirty = true;
        updTimer->start(800);
    }
}

void UserListBase::addSortItem(ListViewItem *item)
{
    for (list<ListViewItem*>::iterator it = sortItems.begin(); it != sortItems.end(); ++it){
        if ((*it) == item)
            return;
    }
    sortItems.push_back(item);
}

void UserListBase::addUpdatedItem(ListViewItem *item)
{
    for (list<ListViewItem*>::iterator it = updatedItems.begin(); it != updatedItems.end(); ++it){
        if ((*it) == item)
            return;
    }
    updatedItems.push_back(item);
}

unsigned UserListBase::getUnread(unsigned)
{
    return 0;
}

void UserListBase::fill()
{
    m_pressedItem = NULL;
    clear();
    GroupItem *grpItem;
    UserViewItemBase *divItem;
    UserViewItemBase *divItemOnline = NULL;
    UserViewItemBase *divItemOffline = NULL;
    ContactList *list = getContacts();
    ContactList::GroupIterator grp_it;
    ContactList::ContactIterator contact_it;
    Group *grp;
    Contact *contact;
    switch (m_groupMode){
    case 0:
        divItemOnline  = NULL;
        divItemOffline = NULL;
        while ((contact = ++contact_it) != NULL){
            if (contact->getIgnore() || (contact->getFlags() & CONTACT_TEMPORARY))
                continue;
            unsigned style;
            QString icons;
            unsigned status = getUserStatus(contact, style, icons);
            unsigned unread = getUnread(contact->id());
            bool bShow = false;

            SIM::PropertyHubPtr data = contact->getUserData("list");
            if (!data.isNull() && data->value("ShowAlways").toBool())
                bShow = true;
            if ((unread == 0) && !bShow && (status <= STATUS_OFFLINE) && m_bShowOnline)
                continue;
            divItem = (status <= STATUS_OFFLINE) ? divItemOffline : divItemOnline;
            if (divItem == NULL){
                if (status <= STATUS_OFFLINE){
                    divItemOffline = new DivItem(this, DIV_OFFLINE);
                    setOpen(divItemOffline, true);
                    divItem = divItemOffline;
                }else{
                    divItemOnline = new DivItem(this, DIV_ONLINE);
                    setOpen(divItemOnline, true);
                    divItem = divItemOnline;
                }
            }
            new ContactItem( divItem, contact, status, style, icons, unread, m_bCheckable );
        }
        break;
    case 1:
        if (m_bShowEmpty){
            while ((grp = ++grp_it) != NULL){
                if (grp->id() == 0)
                    continue;
                grpItem = new GroupItem( this, grp, true, m_bCheckable );
            }
            grpItem = new GroupItem( this, list->group(0), true, m_bCheckable );
        }
        while ((contact = ++contact_it) != NULL){
            if (contact->getIgnore() || (contact->getFlags() & CONTACT_TEMPORARY))
                continue;
            unsigned style;
            QString icons;
            unsigned status = getUserStatus(contact, style, icons);
            unsigned unread = getUnread(contact->id());
            bool bShow = false;
            SIM::PropertyHubPtr data = contact->getUserData("list");
            if (!data.isNull() && data->value("ShowAlways").toBool())
                bShow = true;
            if ((status <= STATUS_OFFLINE) && !bShow && (unread == 0) && m_bShowOnline)
                continue;
            grpItem = findGroupItem(contact->getGroup());
            if (grpItem == NULL){
                grp = list->group(contact->getGroup());
                if (grp)
                    grpItem = new GroupItem( this, grp, true, m_bCheckable );
                if (grpItem == NULL)
                    continue;
            }
            m_contactItem = new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
            grpItem->m_nContacts++;
            if ((status > STATUS_OFFLINE) && !m_bShowOnline){
                grpItem->m_nContactsOnline++;
                m_contactItem->m_bOnline = true;
            }
        }
        break;
    case 2:
        divItemOnline = new DivItem(this, DIV_ONLINE);
        setOpen(divItemOnline, true);
        if (m_bShowEmpty){
            while ((grp = ++grp_it) != NULL){
                if (grp->id() == 0)
                    continue;
                grpItem = new GroupItem( divItemOnline, grp, false, m_bCheckable );
            }
            grpItem = new GroupItem( divItemOnline, list->group(0), false, m_bCheckable );
        }
        if (!m_bShowOnline){
            divItemOffline = new DivItem(this, DIV_OFFLINE);
            setOpen(divItemOffline, true);
            grp_it.reset();
            if (m_bShowEmpty){
                while ((grp = ++grp_it) != NULL){
                    if (grp->id() == 0)
                        continue;
                    grpItem = new GroupItem( divItemOffline, grp, true, m_bCheckable );
                }
                grpItem = new GroupItem( divItemOffline, list->group(0), true, m_bCheckable );
            }
        }
        while ((contact = ++contact_it) != NULL){
            if (contact->getIgnore() || (contact->getFlags() & CONTACT_TEMPORARY))
                continue;
            unsigned style;
            QString icons;
            unsigned status = getUserStatus(contact, style, icons);
            unsigned unread = getUnread(contact->id());
            bool bShow = false;
            SIM::PropertyHubPtr data = contact->getUserData("list");
            if (!data.isNull() && data->value("ShowAlways").toBool())
                bShow = true;
            if ((unread == 0) && !bShow && (status <= STATUS_OFFLINE) && m_bShowOnline)
                continue;
            if (status <= STATUS_OFFLINE){
                if (divItemOffline == NULL){
                    divItemOffline = new DivItem(this, DIV_OFFLINE);
                    setOpen(divItemOffline, true);
                }
                divItem = divItemOffline;
            }else{
                divItem = divItemOnline;
            }
            grpItem = findGroupItem(contact->getGroup(), divItem);
            if (grpItem == NULL){
                Group *grp = getContacts()->group(contact->getGroup());
                if (grp == NULL)
                    continue;
                grpItem = new GroupItem( divItem, grp, true, m_bCheckable );
            }
			new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
            grpItem->m_nContacts++;
        }
        break;
    }
    adjustColumn();
}

static void resort(ListViewItem *item)
{
	/*
    if (!item->isExpandable())
        return;
    item->sort();
    for (item = item->firstChild(); item; item = item->nextSibling())
        resort(item);
		*/
}

bool UserListBase::processEvent(Event *e)
{
    if (e->type() == eEventRepaintView){
        //sort();
        for(int c = 0; c < topLevelItemCount(); c++)
        {
            ListViewItem *item = static_cast<ListViewItem*>(topLevelItem(c));
            resort(item);
        }
        viewport()->repaint();
    }
    if (m_bInit){
        switch (e->type()){
        case eEventGroup:{
            EventGroup *ev = static_cast<EventGroup*>(e);
            Group *g = ev->group();
            switch (ev->action()) {
                case EventGroup::eAdded:
                case EventGroup::eChanged:
                    addGroupForUpdate(g->id());
                    break;
                case EventGroup::eDeleted:
                    for (list<unsigned long>::iterator it = updGroups.begin(); it != updGroups.end(); ++it){
                        if (*it == g->id()){
                            updGroups.erase(it);
                            break;
                        }
                    }
                    GroupItem *grpItem;
                    switch (m_groupMode){
                    case 1:
                        grpItem = findGroupItem(g->id());
                        deleteItem(grpItem);
                        break;
                    case 2:
                        for(int c = 0; c < topLevelItemCount(); c++)
                        {
                            ListViewItem *item = static_cast<ListViewItem*>(topLevelItem(c));
                            UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
                            if (i->type() != DIV_ITEM) continue;
                            DivItem *divItem = static_cast<DivItem*>(i);
                            grpItem = findGroupItem(g->id(), divItem);
                            deleteItem(grpItem);
                        }
                        break;
                   }
            }
            break;
        }
        case eEventContact: {
            EventContact *ec = static_cast<EventContact*>(e);
            Contact *contact = ec->contact();
            switch(ec->action()) {
                case EventContact::eDeleted: {
                    for (list<unsigned long>::iterator it = updContacts.begin(); it != updContacts.end(); ++it){
                        if (*it == contact->id()){
                            updContacts.erase(it);
                            break;
                        }
                    }
                    ContactItem *item = findContactItem(contact->id());
                    if (item){
                        if (m_groupMode){
                            GroupItem *grpItem = static_cast<GroupItem*>(item->parent());
                            grpItem->m_nContacts--;
                            if (item->m_bOnline)
                                grpItem->m_nContactsOnline--;
                            addGroupForUpdate(grpItem->id());
                            deleteItem(item);
                            if ((m_groupMode == 2) &&
                                    (grpItem->child(0) == NULL) &&
                                    m_bShowOnline){
                                DivItem *div = static_cast<DivItem*>(grpItem->parent());
                                if (div->state() == DIV_OFFLINE){
                                    deleteItem(grpItem);
                                    if (div->child(0) == NULL)
                                        deleteItem(div);
                                }
                            }
                        }else{
                            ListViewItem *p = static_cast<ListViewItem*>(item->parent());
                            deleteItem(item);
                            if (p->child(0) == NULL)
                                deleteItem(p);
                        }
                    }
                    break;
                }
                case EventContact::eStatus:
                case EventContact::eChanged: {
                    if (!contact->getIgnore() && ((contact->getFlags() & CONTACT_TEMPORARY) == 0)){
                        addContactForUpdate(contact->id());
                    }else{
                        EventContact e(contact, EventContact::eDeleted);
                        processEvent(&e);
                        e.setNoProcess();
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case eEventMessageReceived:{
                EventMessage *em = static_cast<EventMessage*>(e);
                Message *msg = em->msg();
                if (msg->type() == MessageStatus){
                    Contact *contact = getContacts()->contact(msg->contact());
                    if (contact)
                        addContactForUpdate(contact->id());
                }
                break;
            }
        default:
            break;
        }
    }
    return ListView::processEvent(e);
}

GroupItem *UserListBase::findGroupItem(unsigned id, ListViewItem *p)
{
    for(int c = 0; c < (p ? p->childCount() : topLevelItemCount()); c++)
    {
        ListViewItem *item = static_cast<ListViewItem*>(!p ? topLevelItem(c) : p->child(c));
        UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
        if (i->type() == GRP_ITEM){
            GroupItem *grpItem = static_cast<GroupItem*>(item);
            if (grpItem->id() == id)
                return grpItem;
        }
        //if (item->isExpandable())
        {
            GroupItem *res = findGroupItem(id, item);
            if (res)
                return res;
        }
    }
    return NULL;
}

ContactItem *UserListBase::findContactItem(unsigned id, ListViewItem *p)
{
    for(int c = 0; c < (p ? p->childCount() : topLevelItemCount()); c++)
    {
        ListViewItem *item = static_cast<ListViewItem*>(p ? p->child(c) : topLevelItem(c));
        UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
        if (i->type() == USR_ITEM){
            ContactItem *contactItem = static_cast<ContactItem*>(item);
            if (contactItem->id() == id)
                return contactItem;
        }
        //if (item->isExpandable())
        {
            ContactItem *res = findContactItem(id, item);
            if (res)
                return res;
        }
    }
    return NULL;
}

unsigned UserListBase::getUserStatus(Contact *contact, unsigned &style, QString &icons)
{
    style = 0;
    QSet<QString> wrkIcons;
    QString statusIcon;
    unsigned long status = contact->contactInfo(style, statusIcon, &wrkIcons);
    if (!statusIcon.isEmpty())
        icons = statusIcon;
    QStringList sl = wrkIcons.toList();
    icons += QLatin1Char(',') + sl.join(",");
    return status;
}

void UserListBase::deleteItem(ListViewItem *item)
{
    if (item == NULL)
        return;
    /*
    if (item == currentItem())
    {

        ListViewItem *nextItem = static_cast<ListViewItem*>(item->nextSibling());
        if (nextItem == NULL){
            if (item->parent()){
                nextItem = static_cast<ListViewItem*>(item->parent())->child(0);
            }else{
                nextItem = static_cast<ListViewItem*>(topLevelItem(0));
            }
            for (; nextItem ; nextItem = nextItem->nextSibling())
                if (nextItem->nextSibling() == item)
                    break;
        }
        if ((nextItem == NULL) && item->parent()){
            nextItem = static_cast<ListViewItem*>(item->parent());
            if (nextItem->firstChild() && (nextItem->firstChild() != item)){
                for (nextItem = nextItem->firstChild(); nextItem; nextItem = nextItem->nextSibling())
                    if (nextItem->nextSibling() == item)
                        break;
            }
        }
        if (nextItem)
        {
            setCurrentItem(nextItem);
            //ensureItemVisible(nextItem);
            //scrollTo(item);
        }
    }
    */
    delete item;
}

UserList::UserList(QWidget *parent)
    : UserListBase(parent)
{
    m_bCheckable = true;
    m_bInit  = true;
    setMenu(0);
    fill();
}

UserList::~UserList()
{
}

void UserList::select( unsigned int id ) {
    ContactItem *pItem = this->findContactItem( id, NULL );
    if( NULL != pItem )
        pItem->setCheckState( 0, Qt::Checked );
}

bool UserList::isHaveSelected() {
    QList< unsigned int > list = selected();
    return ( list.count() > 0 );
}

QList< unsigned int > UserList::selected( QTreeWidgetItem *pItem ) {
    QList< unsigned int > list;

    QList< QTreeWidgetItem* > listSubItems;

    if( NULL == pItem ) {
        for( int i = 0 ; i < topLevelItemCount() ; i++ ) {
            listSubItems.push_back( topLevelItem( i ) );
        }
    }
    else {
        for( int i = 0 ; i < pItem->childCount() ; i++ ) {
            listSubItems.push_back( pItem->child( i ) );
        }
    }

    foreach( QTreeWidgetItem* pSubItem, listSubItems ) {
        UserViewItemBase *pBaseItem = static_cast<UserViewItemBase*>( pSubItem );
        if( GRP_ITEM == pBaseItem->type() ) {
            list.append( selected( pSubItem ) );
        }
        else if( ( USR_ITEM == pBaseItem->type() ) && ( Qt::Checked == pSubItem->checkState( 0 ) ) ) {
            ContactItem *pContactItem = static_cast<ContactItem*>( pSubItem );
            list.push_back( pContactItem->id() );
        }
    }

    return list;
}

QList< unsigned int > UserList::selected() {
    return selected( NULL );
}

// vim: set expandtab:

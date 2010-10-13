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
#include "roster/userview.h"

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
#include <QMenu>

#include "log.h"


using namespace std;
using namespace SIM;

bool ListView::s_bInit = false;

ListView::ListView(QWidget *parent) : QTreeWidget(parent)
{
//    m_menuId = MenuListView;
//    if (!s_bInit){
//        s_bInit = true;
//        EventMenu(MenuListView, EventMenu::eAdd).process();

//        Command cmd;
//        cmd->id			= CmdListDelete;
//        cmd->text		= I18N_NOOP("&Delete");
//        cmd->icon		= "remove";
//        cmd->accel		= "Del";
//        cmd->menu_id	= MenuListView;
//        cmd->menu_grp	= 0x1000;
//        cmd->flags		= COMMAND_DEFAULT;

//        EventCommandCreate(cmd).process();
//    }
//    setColumnCount(0);
//    m_bAcceptDrop = false;
//    viewport()->setAcceptDrops(true);
//    m_pressedItem = NULL;
//    verticalScrollBar()->installEventFilter(this);
//    m_resizeTimer = new QTimer(this);
}

ListView::~ListView()
{
}

//void ListView::repaint(QTreeWidgetItem* item)
//{
//    update(indexFromItem(item));
//}

//bool ListView::getMenu(QTreeWidgetItem *item, unsigned long &id, void *&param)
//{
//    if (m_menuId == 0)
//        return false;
//    id = m_menuId;
//    param = item;
//    return true;
//}

//void ListView::setMenu(unsigned long menuId)
//{
//    m_menuId = menuId;
//}

//bool ListView::processEvent(Event *e)
//{
//    if (e->type() == eEventCommandExec){
//        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
//        CommandDef *cmd = ece->cmd();
//        if ((cmd->id == CmdListDelete) && (cmd->menu_id == MenuListView)){
//            QTreeWidgetItem *item = (QTreeWidgetItem*)(cmd->param);
//            if (item->treeWidget() == this){
//                emit deleteItem(item);
//                return true;
//            }
//        }
//    }
//    return false;
//}

//void ListView::keyPressEvent(QKeyEvent *e)
//{
//    if (e->key()){
//        int key = e->key();
//        if (e->modifiers() & Qt::ShiftModifier)
//            key |= Qt::SHIFT;
//        if (e->modifiers() & Qt::ControlModifier)
//            key |= Qt::CTRL;
//        if (e->modifiers() & Qt::AltModifier)
//            key |= Qt::ALT;
//        QTreeWidgetItem *item = currentItem();
//        if (item){
//            unsigned long id;
//            void *param;
//            if (getMenu(item, id, param)){
//                EventMenuProcess e(id, param, key);
//                if (e.process() && e.menu())
//                    return;
//            }
//        }
//    }
//    if (e->key() == Qt::Key_F10){
//        showPopup(currentItem(), QPoint());
//        return;
//    }
//    QTreeWidget::keyPressEvent(e);
//}

//QTreeWidgetItem* ListView::currentItem()
//{
//    return QTreeWidget::currentItem();
//}

//void ListView::viewportMousePressEvent(QMouseEvent * /*e*/)
//{
//    //QTreeWidget::viewportMousePressEvent(e);
//}

//void ListView::mousePressEvent(QMouseEvent *e)
//{
//    if (e->button() == Qt::LeftButton)
//    {
//        m_pressedItem = itemAt(e->pos());
//        if(m_pressedItem)
//        {
//            update();
//        }
//    }
//    QTreeWidget::mousePressEvent(e);
//}

//void ListView::mouseMoveEvent(QMouseEvent *e)
//{
//    QTreeWidget::mouseMoveEvent(e);
//}

//QTreeWidgetItem* ListView::itemAt(const QPoint& p)
//{
//    return QTreeWidget::itemAt(p);
//}

//void ListView::mouseReleaseEvent(QMouseEvent *e)
//{
//    QTreeWidget::mouseReleaseEvent(e);
//    if (m_pressedItem){
//        QTreeWidgetItem *item = m_pressedItem;
//        m_pressedItem = NULL;
//        //update(model()->index(item->row(), 0));
//        update();
//        QTreeWidgetItem *citem = itemAt(e->pos());
//        if (item == citem)
//            emit clickItem(item);
//    }
//}

//void ListView::viewportContextMenuEvent( QContextMenuEvent *e)
//{
//    QPoint p = e->globalPos();
//    QTreeWidgetItem *list_item = itemAt(viewport()->mapFromGlobal(p));
//    showPopup(list_item, p);
//}

//void ListView::showPopup(QTreeWidgetItem *item, QPoint p)
//{
//    unsigned long id;
//    void *param;

//    if (item == NULL)
//        return;

//    if (!getMenu(item, id, param))
//        return;
//    if (p.isNull()){
//        QRect rc = visualItemRect(item);
//        p = QPoint(rc.x() + rc.width() / 2, rc.y() + rc.height() / 2);
//        p = viewport()->mapToGlobal(p);
//    }
//    EventMenuProcess eMenu(id, param);
//    eMenu.process();
//    QMenu *menu = eMenu.menu();
//    if (menu){
//        setCurrentItem(item);
//        menu->popup(p);
//    }
//}

//void ListView::contextMenuEvent(QContextMenuEvent* e)
//{
//    unsigned long id;
//    void *param;

//    QTreeWidgetItem* item = itemAt(e->pos());
//    if (item == NULL)
//        return;

//    if (!getMenu(item, id, param))
//        return;
//    EventMenuProcess eMenu(id, param);
//    eMenu.process();
//    QMenu *menu = eMenu.menu();
//    if (menu)
//    {
//        setCurrentItem(item);
//        menu->popup(e->globalPos());
//    }
//}

//bool ListView::eventFilter(QObject *o, QEvent *e)
//{
//    return QTreeWidget::eventFilter(o, e);
//}


//void ListView::resizeEvent(QResizeEvent *e)
//{
//    QTreeWidget::resizeEvent(e);
//}

//QTreeWidgetItem* ListView::firstChild()
//{
//    return topLevelItem(0);
//}

////void ListView::startDrag(Qt::DropActions)
////{
////    emit dragStart();
////}

//QMimeData *ListView::dragObject()
//{
//    return NULL;
//}

//void ListView::setAcceptDrop(bool bAccept)
//{
//    m_bAcceptDrop = bAccept;
//}

//void ListView::dragEnterEvent(QDragEnterEvent *e)
//{
//    //emit dragEnter(e);
//    if (m_bAcceptDrop){
//        e->accept();
//        return;
//    }
//    e->ignore();
//}

//void ListView::dragMoveEvent(QDragMoveEvent *e)
//{
//    if (m_bAcceptDrop){
//        e->accept();
//        return;
//    }
//    e->ignore();
//}

//void ListView::dropEvent(QDropEvent *e)
//{
//    if (m_bAcceptDrop){
//        e->accept();
//        //emit drop(e);
//        return;
//    }
//    e->ignore();
//}

//void ListView::addColumn(const QString& name)
//{
//    setColumnCount(columnCount() + 1);
//    headerItem()->setText(columnCount() - 1, name);
//}

//static char CONTACT_MIME[] = "application/x-contact";

//ContactDragObject::ContactDragObject( Contact *contact ) : QMimeData()
//{
//    QByteArray data;
//    m_id = contact->id();
//    data.resize(sizeof(m_id));
//    memcpy(data.data(), &m_id, sizeof(m_id));
//    setData(CONTACT_MIME, data);
//}

//ContactDragObject::~ContactDragObject()
//{
//    ListView *view = static_cast<ListView*>(parent());
////    if (view && view->m_pressedItem){
////        //ListViewItem *item = view->m_pressedItem;
////        view->m_pressedItem = NULL;
////        //view->update(view->model()->index(item->row(), item->column()));
////        view->update();
////    }
//    Contact *contact = getContacts()->contact(m_id);
//    if (contact && (contact->getFlags() & CONTACT_DRAG))
//        delete contact;
//}

//bool ContactDragObject::canDecode(QMimeSource *s)
//{
//    return (decode(s) != NULL);
//}

//Contact *ContactDragObject::decode( QMimeSource *s )
//{
//    if (!s->provides(CONTACT_MIME))
//        return NULL;
//    QByteArray data = s->encodedData(CONTACT_MIME);
//    unsigned long id;
//    if( data.size() != sizeof( id ) )
//        return NULL;
//    memcpy( &id, data.data(), sizeof(id));
//    return getContacts()->contact(id);
//}

//UserViewItemBase::UserViewItemBase(UserListBase *parent, int type)
//: QTreeWidgetItem(parent, type)
//{
//}

//UserViewItemBase::UserViewItemBase(UserViewItemBase *parent, int type)
//: QTreeWidgetItem(parent, type)
//{
//}

//void UserViewItemBase::setup()
//{
//}

//void UserViewItemBase::setCheckable( bool bCheckable )
//{
//    if( bCheckable )
//    {
//        setFlags( flags() | Qt::ItemIsUserCheckable );
//        setCheckState( 0, Qt::Unchecked );
//    }
//    else
//    {
//        setFlags( flags() & ~Qt::ItemIsUserCheckable );
//    }
//}

//DivItem::DivItem(UserListBase *view, unsigned type)
//: UserViewItemBase(view, DIV_ITEM)
//{
//    m_type = type;
//    setText(0, QString::number(m_type));
//    setFlags(Qt::ItemIsEnabled);
//}

//QVariant DivItem::data( int column, int role ) const
//{
//    QVariant result;
//    QString text;
//    if (role == Qt::DisplayRole)
//    {
//        if (m_type == DIV_ONLINE)
//            text = i18n("Online");
//        else if (m_type == DIV_OFFLINE)
//            text = i18n("Offline");
//        result = QVariant(text);
//    }
//    else return UserViewItemBase::data(column, role);
//    return result;
//}

//GroupItem::GroupItem(UserListBase *view, Group *grp, bool bOffline, bool bCheckable )
//: UserViewItemBase(view, GRP_ITEM)
//, m_id (grp->id() )
//, m_bOffline (bOffline)
//{
//    init(grp);
//    setCheckable( bCheckable );
//}

//GroupItem::GroupItem( UserViewItemBase *view, Group *grp, bool bOffline, bool bCheckable )
//: UserViewItemBase(view, GRP_ITEM)
//, m_id (grp->id() )
//, m_bOffline (bOffline)
//{
//    init(grp);
//    setCheckable( bCheckable );
//}

//void GroupItem::init(Group *grp)
//{
//    m_unread = 0;
//    m_nContacts = 0;
//    m_nContactsOnline = 0;
//    SIM::PropertyHubPtr data = grp->getUserData("list");
//    if (data.isNull())
//        setOpen(true);
//    else if (m_bOffline)
//        setOpen(data->value("OfflineOpen").toBool());
//    else
//        setOpen(data->value("OnlineOpen").toBool());
//    update(grp, true);
//}

//void GroupItem::update(Group *grp, bool bInit)
//{
//    QString s;
//    s = "A";
//    if (grp->id())
//    {
//        s = QString::number(getContacts()->groupIndex(grp->id()));
//        while (s.length() < 12)
//            s = QString("0") + s;
//    }
//    if (s == text(0))
//        return;
//    setText(0, s);
//    if (bInit)
//        return;
//    QTreeWidgetItem *p = parent();
//    if (p)
//        //p->sort();
//        return;
//    //listView()->sort();
//}

//void GroupItem::setOpen(bool bOpen)
//{
//    //UserViewItemBase::setOpen(bOpen);
//    Group *grp = getContacts()->group(m_id);
//    if (grp)
//    {
//        SIM::PropertyHubPtr data = grp->getUserData("list", !bOpen);
//        if (data.isNull())
//            return;

//        if (m_bOffline)
//        {
//            data->setValue("OfflineOpen", bOpen);
//            return;
//        }
//        data->setValue("OnlineOpen", bOpen);
//    }
//}

//QVariant GroupItem::data( int column, int role ) const
//{
//    QVariant result;
//    QString text;
//    Group *grp = getContacts()->group(id());
//    if (role == Qt::DisplayRole)
//    {

//        if (!id())
//            text = i18n("Not in list");
//        else if (grp)
//            text = grp->getName();
//        else
//            text = "???";
//        if (m_nContacts)
//        {
//            text += " (";
//            if (m_nContactsOnline)
//                text += QString::number(m_nContactsOnline) + '/';
//            text += QString::number(m_nContacts) + ')';
//        }
//        result = QVariant(text);
//    }
//    else return UserViewItemBase::data(column, role);
//    return result;
//}

//void GroupItem::setData( int column, int role, const QVariant &value )
//{
//    if( Qt::CheckStateRole == role )
//    {
//        Qt::CheckState cs = (Qt::CheckState)value.toInt();
//        for( int i = 0 ; i < childCount() ; i++ )
//            child( i )->setCheckState( 0, cs );
//    }

//    UserViewItemBase::setData( column, role, value );
//}

//ContactItem::ContactItem( UserViewItemBase *view, Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread, bool bCheckable )
//: UserViewItemBase(view, USR_ITEM)
//, m_id (contact->id())
//{

//    init(contact, status, style, icons, unread);
//    setCheckable(bCheckable);
//    setFlags( flags() | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsSelectable | (bCheckable ? Qt::ItemIsUserCheckable : Qt::NoItemFlags));
//}

//void ContactItem::init(Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread)
//{
//    m_bOnline    = false;
//    m_bBlink	 = false;
//    update(contact, status, style, icons, unread);
//}

//bool ContactItem::update(Contact *contact, unsigned status, unsigned style, const QString &icons, unsigned unread)
//{
//    m_unread = unread;
//    m_style  = style;
//    m_status = status;
//    QString active;
//    active.sprintf("%08lX", (long unsigned int)(0xFFFFFFFF - contact->getLastActive()));
//    m_sExtraIcons = icons;
//    QString icon = getToken(m_sExtraIcons, ',');
//    m_Icon = Icon(icon);
//    setText(CONTACT_ICONS, icons);
//    setText(CONTACT_ACTIVE, active);
//    setText(CONTACT_STATUS, QString::number(9 - status));
//    setup();
//    return true;
//}

//QString ContactItem::key(int column/*, bool ascending */) const //Fixme?
//{
//    if (column == 0)
//    {
//        unsigned mode = CorePlugin::instance()->value("SortMode").toUInt();
//        QString res;
//        for (;;)
//        {
//            int n = 0;
//            switch (mode & 0xFF)
//            {
//            case SORT_STATUS:
//                n = CONTACT_STATUS;
//                break;
//            case SORT_ACTIVE:
//                n = CONTACT_ACTIVE;
//                break;
//            case SORT_NAME:
//                n = CONTACT_TEXT;
//                break;
//            }
//            if (n == 0)
//                break;
//            res += text(n).toLower();
//            mode = mode >> 8;
//        }
//        return res;
//    }
//    return QString::null; //UserViewItemBase::key(column, ascending);
//}

//QVariant ContactItem::data( int column, int role ) const
//{
//    Contact *contact = getContacts()->contact( m_id );
//    if( NULL == contact )
//        return QVariant();

//    QVariant result;

//    if (role == Qt::DisplayRole)
//        result = QVariant(contact->getName());
//    else if (role == Qt::DecorationRole)
//    {
//        QIcon icon = m_Icon;
//        UserView *uv = dynamic_cast<UserView*>(treeWidget());
//        if (m_unread && uv->m_bUnreadBlink)
//        {
//            CommandDef *def = CorePlugin::instance()->messageTypes.find(m_unread);
//            if (def)
//                icon = Icon(def->icon);
//        }
//        result = QVariant(icon);
//    }
//    else if (role == Qt::ToolTipRole)
//        result = QVariant(contact->tipText());
//    else if (role == SIM::ExtraIconsRole)
//        result = QVariant(m_sExtraIcons);
//    else return UserViewItemBase::data(column, role);

//    return result;
//}

//UserListBase::UserListBase(QWidget *parent)
//: ListView(parent)
//, m_bInit		(false)
//, m_bDirty		(false)
//, m_groupMode	(1)
//, m_bShowOnline (false)
//, m_bShowEmpty  (false)
//, m_bCheckable	(false)
//, updTimer		(new QTimer(this))
//, m_contactItem	(NULL)
//{
//    addColumn("");
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
//    connect(updTimer, SIGNAL(timeout()), this, SLOT(drawUpdates()));
//    m_unreadTimer.setInterval(400);
//    connect(&m_unreadTimer, SIGNAL(timeout()), this, SLOT(updateUnread()));
//    m_unreadTimer.start();
//}

//UserListBase::~UserListBase()
//{
//}


//bool UserListBase::updateGroups()
//{
//    QTreeWidgetItem *item = 0;
//    bool changed = false;
//    list<unsigned long>::iterator it;
//    for (it = updGroups.begin(); it != updGroups.end(); ++it)
//    {
//        Group *group = getContacts()->group(*it);
//        if (group == NULL)
//            continue;
//        switch (m_groupMode)
//        {
//        case 1:
//            item = findGroupItem(group->id());
//            if (item)
//                if (!m_bShowEmpty && item->child(0) == NULL)
//                {
//                    delete item;
//                    changed = true;
//                }
//                else
//                {
//                    static_cast<GroupItem*>(item)->update(group);
//                    addUpdatedItem(item);
//                }
//            else if (m_bShowEmpty)
//            {
//                new GroupItem( this, group, true, m_bCheckable );
//                changed = true;
//            }
//            break;
//        case 2:
//            for(int c = 0; c < topLevelItemCount(); c++)
//            {
//                item = topLevelItem(c);
//                UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//                if (i->type() != DIV_ITEM)
//                    continue;
//                DivItem *divItem = static_cast<DivItem*>(i);
//                if (getGrpItem(group, divItem))
//                {
//                    if (!m_bShowEmpty && (item->child(0) == NULL))
//                    {
//                        delete getGrpItem(group, divItem);
//                        changed = true;
//                    }
//                    else
//                    {
//                        getGrpItem(group, divItem)->update(group);
//                        addUpdatedItem(getGrpItem(group, divItem));
//                    }
//                }
//                else if (m_bShowEmpty)
//                {
//                    new GroupItem( divItem, group, divItem->state() == DIV_OFFLINE, m_bCheckable );
//                    changed = true;
//                }
//            }
//            break;
//        }
//    }
//    return changed;
//}

//void UserListBase::updateUnread()
//{
//    for(std::list<ContactItem*>::iterator it = m_unreadItems.begin(); it != m_unreadItems.end(); ++it) {
//        if((*it)->m_unread) {
//            repaint(*it);
//        }
//        else {
//            m_unreadItems.erase(it);
//            break;
//        }
//    }
//}

//bool UserListBase::removeContactFromItem(unsigned long contactId, DivItem* item)
//{
//    if (item)
//    {
//        m_contactItem = findContactItem(contactId, item);
//        if (m_contactItem)
//        {
//            deleteItem(m_contactItem); //<== crash
//            if (item->child(0) == NULL)
//            {
//                deleteItem(item);
//                refreshOnlineOfflineGroups();
//            }
//            return true;
//        }
//    }
//    return false;
//}

//bool UserListBase::updateContactNoGroups(SIM::Contact* contact)
//{
//    unsigned style;
//    QString icons;
//    unsigned status = getUserStatus(contact, style, icons);
//    bool bShow = false;
//    SIM::PropertyHubPtr data = contact->getUserData("list");
//    if (!data.isNull() && data->value("ShowAlways").toBool())
//        bShow = true;
//    bool changed = false;
//    if (status <= STATUS_OFFLINE)
//    {
//        changed |= removeContactFromItem(contact->id(), m_itemOnline);
//        if (getUnread(contact) == 0 && !bShow && m_bShowOnline)
//        {
//            changed |= removeContactFromItem(contact->id(), m_itemOffline);
//            return changed;
//        }
//        if (m_itemOffline == NULL)
//        {
//            m_itemOffline = new DivItem(this, DIV_OFFLINE);
//            m_itemOffline->setExpanded(true);
//            changed = true;
//        }
//        m_contactItem = findContactItem(contact->id(), m_itemOffline);
//        if (m_contactItem)
//        {
//            if (m_contactItem->update(contact, status, style, icons, getUnread(contact)))
//                addSortItem(m_itemOffline);
//            addUpdatedItem(m_contactItem);
//        }
//        else
//        {
//            m_contactItem = new ContactItem( m_itemOffline, contact, status, style, icons, getUnread(contact), m_bCheckable );
//            changed = true;
//        }
//    }
//    else
//    {
//        changed |= removeContactFromItem(contact->id(), m_itemOffline);
//        if (m_itemOnline == NULL)
//        {
//            m_itemOnline = new DivItem(this, DIV_ONLINE);
//            m_itemOnline->setExpanded(true);
//            changed = true;
//        }
//        m_contactItem = findContactItem(contact->id(), m_itemOnline);
//        if (m_contactItem)
//        {
//            if (m_contactItem->update(contact, status, style, icons, getUnread(contact)))
//                addSortItem(m_itemOnline);
//            addUpdatedItem(m_contactItem);
//        }
//        else
//        {
//            m_contactItem = new ContactItem( m_itemOnline, contact, status, style, icons, getUnread(contact), m_bCheckable );
//            changed = true;
//        }
//        if(getUnread(contact))
//            m_unreadItems.push_back(m_contactItem);
//    }
//    return changed;
//}

//bool UserListBase::updateContactGroupMode1(SIM::Contact* contact)
//{
//    bool changed = false;
//    bool bShow = false;
//    SIM::PropertyHubPtr data = contact->getUserData("list");
//    if (!data.isNull() && data->value("ShowAlways").toBool())
//        bShow = true;
//    unsigned style;
//    QString icons;
//    unsigned status = getUserStatus(contact, style, icons);
//    unsigned unread = getUnread(contact->id());
//    m_contactItem = findContactItem(contact->id());
//    GroupItem *grpItem = NULL;
//    if (m_contactItem)
//    {
//        grpItem = static_cast<GroupItem*>(m_contactItem->parent());
//        if (status <= STATUS_OFFLINE && unread == 0 && !bShow && m_bShowOnline || contact->getGroup() != (int)grpItem->id()){
//            grpItem->m_nContacts--;
//            if (m_contactItem->m_bOnline)
//                grpItem->m_nContactsOnline--;
//            addGroupForUpdate(grpItem->id());
//            deleteItem(m_contactItem);
//            changed = true;
//            if (!m_bShowEmpty && (grpItem->child(0) == NULL))
//                delete grpItem;
//            m_contactItem = NULL;
//            grpItem = NULL;
//        }
//    }
//    if (status <= STATUS_OFFLINE && !unread && !bShow && m_bShowOnline)
//        return changed;

//    if (grpItem == NULL)
//    {
//        grpItem = findGroupItem(contact->getGroup());
//        Group *grp = getContacts()->group(contact->getGroup());
//        if (grpItem == NULL && grp)
//        {
//            grpItem = new GroupItem( this, grp, true, m_bCheckable );
//            changed = true;
//        }
//    }
//    if (!grpItem)
//        return changed;

//    if (m_contactItem)
//    {
//        if (m_contactItem->update(contact, status, style, icons, unread))
//            addSortItem(grpItem);
//        addUpdatedItem(m_contactItem);
//        if (!m_bShowOnline && m_contactItem->m_bOnline != (status > STATUS_OFFLINE)){
//            if (status <= STATUS_OFFLINE)
//            {
//                grpItem->m_nContactsOnline--;
//                m_contactItem->m_bOnline = false;
//            }
//            else
//            {
//                grpItem->m_nContactsOnline++;
//                m_contactItem->m_bOnline = true;
//            }
//            addGroupForUpdate(grpItem->id());
//        }
//    }
//    else
//    {
//        changed = true;
//        m_contactItem = new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
//        grpItem->m_nContacts++;
//        if (!m_bShowOnline && status > STATUS_OFFLINE)
//        {
//            grpItem->m_nContactsOnline++;
//            m_contactItem->m_bOnline = true;
//        }
//        addGroupForUpdate(grpItem->id());
//    }
//    return changed;
//}

//bool UserListBase::updateContactGroupMode2(SIM::Contact* contact)
//{
//    bool changed = false;
//    bool bShow = false;
//    SIM::PropertyHubPtr data = contact->getUserData("list");
//    if (!data.isNull() && data->value("ShowAlways").toBool())
//        bShow = true;
//    unsigned style;
//    QString icons;
//    unsigned status = getUserStatus(contact, style, icons);
//    unsigned unread = getUnread(contact->id());
//    m_contactItem = findContactItem(contact->id());
//    GroupItem *grpItem = NULL;
//    m_contactItem = findContactItem(contact->id(), m_itemOnline);
//    grpItem = NULL;
//    if (m_contactItem)
//    {
//        grpItem = static_cast<GroupItem*>(m_contactItem->parent());
//        if (status <= STATUS_OFFLINE || (int)grpItem->id() != contact->getGroup())
//        {
//            grpItem->m_nContacts--;
//            addGroupForUpdate(grpItem->id());
//            deleteItem(m_contactItem);
//            changed = true;
//            if (!m_bShowEmpty && grpItem->child(0) == NULL)
//                delete grpItem;
//            grpItem = NULL;
//            m_contactItem = NULL;
//        }
//    }
//    if (m_itemOffline)
//    {
//        m_contactItem = findContactItem(contact->id(), m_itemOffline);
//        grpItem = NULL;
//        if (m_contactItem)
//        {
//            grpItem = static_cast<GroupItem*>(m_contactItem->parent());
//            if (status > STATUS_OFFLINE || (int)grpItem->id() != contact->getGroup())
//            {
//                grpItem->m_nContacts--;
//                addGroupForUpdate(grpItem->id());
//                deleteItem(m_contactItem);
//                m_contactItem = NULL;
//                changed = true;
//                if (m_bShowOnline && grpItem->child(0) == NULL)
//                {
//                    deleteItem(grpItem);
//                    grpItem = NULL;
//                    if (m_itemOffline->child(0) == NULL)
//                    {
//                        deleteItem(m_itemOffline);
//                        m_itemOffline = NULL;
//                    }
//                }
//            }
//        }
//    }
//    if (unread == 0 && !bShow && status <= STATUS_OFFLINE && m_bShowOnline)
//        return changed;
//    DivItem *divItem=NULL;
//    if (status > STATUS_OFFLINE)
//        divItem = m_itemOnline;
//    else if (m_itemOffline == NULL)
//    {
//        changed = true;
//        m_itemOffline = new DivItem(this, DIV_OFFLINE);
//        m_itemOffline->setExpanded(true);
//        divItem = m_itemOffline;
//    }

//    grpItem = findGroupItem(contact->getGroup(), divItem);
//    if (grpItem == NULL)
//    {
//        Group *grp = getContacts()->group(contact->getGroup());
//        if (grp == NULL)
//            return changed;
//        changed = true;
//        grpItem = new GroupItem( divItem, grp, true, m_bCheckable );
//        addSortItem(divItem);
//    }
//    m_contactItem = findContactItem(contact->id(), grpItem);
//    if (!m_contactItem)
//    {
//        changed = true;
//        new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
//        grpItem->m_nContacts++;
//        addGroupForUpdate(grpItem->id());
//    }
//    else if (m_contactItem->update(contact, status, style, icons, unread))
//        addSortItem(grpItem);
//    return changed;
//}

//bool UserListBase::updateContacts()
//{
//    bool changed = false;
//    for (list<unsigned long>::iterator it = updContacts.begin(); it != updContacts.end(); ++it)
//    { //Fixme, got crash: list operator not incremenable!
//        Contact *contact = getContacts()->contact(*it);
//        if (contact == NULL)
//            continue;

//        switch (m_groupMode)
//        {
//        case 0:
//            changed |= updateContactNoGroups(contact);
//            break;
//        case 1:
//            changed |= updateContactGroupMode1(contact);
//            break;
//        case 2:
//            changed |= updateContactGroupMode2(contact);
//            break;
//        default:
//            log(L_WARN, "Invalid group mode in UserListBase::updateContacts");
//        }
//    }
//    return changed;
//}

//void UserListBase::refreshOnlineOfflineGroups()
//{
//    QTreeWidgetItem* item = 0;
//    m_itemOnline = 0;
//    m_itemOffline = 0;
//    if (updContacts.size() && m_groupMode != 1)
//    {
//        for(int c = 0; c < topLevelItemCount(); c++)
//        {
//            item = topLevelItem(c);
//            UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//            if (i->type() != DIV_ITEM) continue;
//            DivItem *divItem = static_cast<DivItem*>(i);
//            if (divItem->state() == DIV_ONLINE)
//                m_itemOnline = divItem;
//            if (divItem->state() == DIV_OFFLINE)
//                m_itemOffline = divItem;
//        }
//    }
//}

//void UserListBase::drawUpdates()
//{
//    log(L_DEBUG, "UserListBase::drawUpdates()");
//    m_bDirty = false;
//    updTimer->stop();

//    viewport()->setUpdatesEnabled(false);
//    bool bChanged = updateGroups();
//    updGroups.clear();

//    refreshOnlineOfflineGroups();
//    if(updateContacts())
//        bChanged = true;
//    updContacts.clear();
//    for (list<QTreeWidgetItem*>::iterator it_sort = sortItems.begin(); it_sort != sortItems.end(); ++it_sort)
//    {
//        if ((*it_sort)->child(0) == NULL)
//            continue;
//        bChanged = true;
//    }
//    sortItems.clear();
//    viewport()->setUpdatesEnabled(true);

//    viewport()->repaint();
//    for (list<QTreeWidgetItem*>::iterator it = updatedItems.begin(); it != updatedItems.end(); ++it)
//        repaint(*it);
//    updatedItems.clear();
//}

//const unsigned UPDATE_TIME = 800;

//void UserListBase::addGroupForUpdate(unsigned long id)
//{
//    for (list<unsigned long>::iterator it = updGroups.begin(); it != updGroups.end(); ++it)
//    {
//        if (*it == id)
//            return;
//    }
//    updGroups.push_back(id);
//    if (!m_bDirty)
//    {
//        m_bDirty = true;
//        updTimer->start(800);
//        log(L_DEBUG, "updTimer->start[1]");
//    }
//}

//void UserListBase::addContactForUpdate(unsigned long id)
//{
//    for (list<unsigned long>::iterator it = updContacts.begin(); it != updContacts.end(); ++it)
//    {
//        if (*it == id)
//            return;
//    }
//    updContacts.push_back(id);
//    if (!m_bDirty)
//    {
//        m_bDirty = true;
//        updTimer->start(800);
//        log(L_DEBUG, "updTimer->start[2]");
//    }
//}

//void UserListBase::addSortItem(QTreeWidgetItem *item)
//{
//    for (list<QTreeWidgetItem*>::iterator it = sortItems.begin(); it != sortItems.end(); ++it)
//        if ((*it) == item)
//            return;
//    sortItems.push_back(item);

//}

//void UserListBase::addUpdatedItem(QTreeWidgetItem *item)
//{
//    for (list<QTreeWidgetItem*>::iterator it = updatedItems.begin(); it != updatedItems.end(); ++it)
//    {
//        if ((*it) == item)
//            return;
//    }
//    updatedItems.push_back(item);
//    if (!m_bDirty)
//    {
//        m_bDirty = true;
//        updTimer->start(800);
//    }
//}

//unsigned UserListBase::getUnread(unsigned)
//{
//    return 0;
//}

//unsigned UserListBase::getUnread(SIM::Contact *contact)
//{
//    return getUnread(contact->id());
//}


//void UserListBase::fill()
//{
//    m_pressedItem = NULL;
//    clear();
//    GroupItem *grpItem;
//    UserViewItemBase *divItem;
//    UserViewItemBase *divItemOnline = NULL;
//    UserViewItemBase *divItemOffline = NULL;
//    ContactList *list = getContacts();
//    ContactList::GroupIterator grp_it;
//    ContactList::ContactIterator contact_it;
//    Group *grp;
//    Contact *contact;
//    switch (m_groupMode)
//    {
//    case 0:
//        divItemOnline  = NULL;
//        divItemOffline = NULL;
//        while ((contact = ++contact_it) != NULL)
//        {
//            if (contact->getIgnore() || (contact->getFlags() & CONTACT_TEMPORARY))
//                continue;
//            unsigned style;
//            QString icons;
//            unsigned status = getUserStatus(contact, style, icons);
//            unsigned unread = getUnread(contact->id());
//            bool bShow = false;

//            SIM::PropertyHubPtr data = contact->getUserData("list");
//            if (!data.isNull() && data->value("ShowAlways").toBool())
//                bShow = true;
//            if (unread == 0 && !bShow && status <= STATUS_OFFLINE && m_bShowOnline)
//                continue;
//            divItem = (status <= STATUS_OFFLINE) ? divItemOffline : divItemOnline;
//            if (divItem == NULL)
//            {
//                if (status <= STATUS_OFFLINE)
//                {
//                    divItemOffline = new DivItem(this, DIV_OFFLINE);
//                    divItemOffline->setExpanded(true);
//                    divItem = divItemOffline;
//                }
//                else
//                {
//                    divItemOnline = new DivItem(this, DIV_ONLINE);
//                    divItemOnline->setExpanded(true);
//                    divItem = divItemOnline;
//                }
//            }
//            new ContactItem( divItem, contact, status, style, icons, unread, m_bCheckable );
//        }
//        break;
//    case 1:
//        if (m_bShowEmpty)
//        {
//            while ((grp = ++grp_it) != NULL)
//            {
//                if (grp->id() == 0)
//                    continue;
//                grpItem = new GroupItem( this, grp, true, m_bCheckable );
//            }
//            grpItem = new GroupItem( this, list->group(0), true, m_bCheckable );
//        }
//        while ((contact = ++contact_it) != NULL)
//        {
//            if (contact->getIgnore() || (contact->getFlags() & CONTACT_TEMPORARY))
//                continue;
//            unsigned style;
//            QString icons;
//            unsigned status = getUserStatus(contact, style, icons);
//            unsigned unread = getUnread(contact->id());
//            bool bShow = false;
//            SIM::PropertyHubPtr data = contact->getUserData("list");
//            if (!data.isNull() && data->value("ShowAlways").toBool())
//                bShow = true;
//            if (status <= STATUS_OFFLINE && !bShow && unread == 0 && m_bShowOnline)
//                continue;
//            grpItem = findGroupItem(contact->getGroup());
//            if (grpItem == NULL)
//            {
//                grp = list->group(contact->getGroup());
//                if (grp)
//                    grpItem = new GroupItem( this, grp, true, m_bCheckable );
//                if (grpItem == NULL)
//                    continue;
//            }
//            m_contactItem = new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
//            grpItem->m_nContacts++;
//            if (status <= STATUS_OFFLINE || m_bShowOnline)
//                continue;

//            grpItem->m_nContactsOnline++;
//            m_contactItem->m_bOnline = true;
//        }
//        break;
//    case 2:
//        divItemOnline = new DivItem(this, DIV_ONLINE);
//        divItemOnline->setExpanded(true);
//        if (m_bShowEmpty)
//        {
//            while ((grp = ++grp_it) != NULL)
//            {
//                if (grp->id() == 0)
//                    continue;
//                grpItem = new GroupItem( divItemOnline, grp, false, m_bCheckable );
//            }
//            grpItem = new GroupItem( divItemOnline, list->group(0), false, m_bCheckable );
//        }
//        if (!m_bShowOnline)
//        {
//            divItemOffline = new DivItem(this, DIV_OFFLINE);
//            divItemOffline->setExpanded(true);
//            grp_it.reset();
//            if (m_bShowEmpty)
//            {
//                while ((grp = ++grp_it) != NULL)
//                {
//                    if (grp->id() == 0)
//                        continue;
//                    grpItem = new GroupItem( divItemOffline, grp, true, m_bCheckable );
//                }
//                grpItem = new GroupItem( divItemOffline, list->group(0), true, m_bCheckable );
//            }
//        }
//        while ((contact = ++contact_it) != NULL)
//        {
//            if (contact->getIgnore() || (contact->getFlags() & CONTACT_TEMPORARY))
//                continue;
//            unsigned style;
//            QString icons;
//            unsigned status = getUserStatus(contact, style, icons);
//            unsigned unread = getUnread(contact->id());
//            bool bShow = false;
//            SIM::PropertyHubPtr data = contact->getUserData("list");
//            if (!data.isNull() && data->value("ShowAlways").toBool())
//                bShow = true;
//            if (unread == 0 && !bShow && status <= STATUS_OFFLINE && m_bShowOnline)
//                continue;
//            if (status > STATUS_OFFLINE)
//                divItem = divItemOnline;
//            else
//            {
//                if (divItemOffline == NULL)
//                {
//                    divItemOffline = new DivItem(this, DIV_OFFLINE);
//                    divItemOffline->setExpanded(true);
//                }
//                divItem = divItemOffline;
//            }
//            grpItem = findGroupItem(contact->getGroup(), divItem);
//            if (grpItem == NULL)
//            {
//                Group *grp = getContacts()->group(contact->getGroup());
//                if (grp == NULL)
//                    continue;
//                grpItem = new GroupItem( divItem, grp, true, m_bCheckable );
//            }
//            new ContactItem( grpItem, contact, status, style, icons, unread, m_bCheckable );
//            grpItem->m_nContacts++;
//        }
//        break;
//    }
//}

//static void resort(QTreeWidgetItem *item)
//{
//    /*
//    if (!item->isExpandable())
//    return;
//    item->sort();
//    for (item = item->firstChild(); item; item = item->nextSibling())
//    resort(item);
//    */
//}

//ContactItem* UserListBase::getContactItem(Contact *contact)
//{
//    return findContactItem(contact->id());
//}

//GroupItem* UserListBase::getGrpItem(Group *g)
//{
//    return findGroupItem(g->id());
//}

//GroupItem* UserListBase::getGrpItem(Group *group, DivItem *divItem)
//{
//    return findGroupItem(group->id(), divItem);
//}

//bool UserListBase::processEvent(Event *e)
//{
//    if (e->type() == eEventRepaintView)
//    {
//        //sort();
//        for(int c = 0; c < topLevelItemCount(); c++)
//        {
//            QTreeWidgetItem *item = topLevelItem(c);
//            resort(item);
//        }
//        viewport()->repaint();
//    }
//    if (m_bInit)
//    {
//        if (e->type() == eEventGroup)
//        {
//            EventGroup *ev = static_cast<EventGroup*>(e);
//            Group *g = ev->group();
//            if (ev->action() == EventGroup::eAdded ||
//                ev->action() == EventGroup::eChanged)
//                addGroupForUpdate(g->id());
//            else if (ev->action() == EventGroup::eDeleted)
//            {
//                for (list<unsigned long>::iterator it = updGroups.begin(); it != updGroups.end(); ++it)
//                {
//                    if (*it != g->id())
//                        continue;
//                    updGroups.erase(it);
//                    break;
//                }
//                GroupItem *grpItem;
//                if (m_groupMode == 1)
//                    deleteItem(getGrpItem(g));
//                else if (m_groupMode == 2)
//                    for (int c = 0; c < topLevelItemCount(); c++)
//                    {
//                        QTreeWidgetItem *item = topLevelItem(c);
//                        UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//                        if (i->type() != DIV_ITEM)
//                            continue;
//                        DivItem *divItem = static_cast<DivItem*>(i);
//                        grpItem = findGroupItem(g->id(), divItem);
//                        deleteItem(grpItem);
//                    }
//            }
//        }
//        else if (e->type() == eEventContact)
//        {
//            EventContact *ec = static_cast<EventContact*>(e);
//            Contact *contact = ec->contact();
//            if (ec->action() == EventContact::eDeleted)
//            {
//                for (list<unsigned long>::iterator it = updContacts.begin(); it != updContacts.end(); ++it)
//                    if (*it == contact->id())
//                    {
//                        updContacts.erase(it);
//                        break;
//                    }
//                    if (getContactItem(contact))
//                        if (!m_groupMode)
//                        {
//                            QTreeWidgetItem *p = getContactItem(contact)->parent();
//                            deleteItem(getContactItem(contact));
//                            if (p->child(0) == NULL)
//                                deleteItem(p);
//                        }
//                        else
//                        {
//                            GroupItem *grpItem = static_cast<GroupItem*>(getContactItem(contact)->parent());
//                            grpItem->m_nContacts--;
//                            if (getContactItem(contact)->m_bOnline)
//                                grpItem->m_nContactsOnline--;
//                            addGroupForUpdate(grpItem->id());
//                            deleteItem(getContactItem(contact));
//                            if (m_groupMode == 2 && grpItem->child(0) == NULL && m_bShowOnline)
//                            {
//                                DivItem *div = static_cast<DivItem*>(grpItem->parent());
//                                if (div->state() == DIV_OFFLINE)
//                                {
//                                    deleteItem(grpItem);
//                                    if (div->child(0) == NULL)
//                                        deleteItem(div);
//                                }
//                            }
//                        }
//            }
//            else if (ec->action() == EventContact::eStatus ||
//                ec->action() == EventContact::eChanged)
//                if (contact->getIgnore() ||
//                    (contact->getFlags() & CONTACT_TEMPORARY) != 0)
//                {
//                    EventContact e(contact, EventContact::eDeleted);
//                    processEvent(&e);
//                    e.setNoProcess();
//                }
//                else addContactForUpdate(contact->id());
//        }
//        else if (e->type() == eEventMessageReceived)
//        {
//            EventMessage *em = static_cast<EventMessage*>(e);
//            Message *msg = em->msg();
//            Contact *contact = getContacts()->contact(msg->contact());
//            if (msg->type() == MessageStatus && contact)
//                addContactForUpdate(contact->id());
//        }
//    }
//    return ListView::processEvent(e);
//}

//GroupItem *UserListBase::findGroupItem(unsigned id, QTreeWidgetItem *p)
//{
//    for(int c = 0; c < (p ? p->childCount() : topLevelItemCount()); c++)
//    {
//        QTreeWidgetItem *item = (!p ? topLevelItem(c) : p->child(c));
//        UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//        if (i->type() == GRP_ITEM)
//        {
//            GroupItem *grpItem = static_cast<GroupItem*>(item);
//            if (grpItem->id() == id)
//                return grpItem;
//        }
//        //if (item->isExpandable())
//        {
//            GroupItem *res = findGroupItem(id, item);
//            if (res)
//                return res;
//        }
//    }
//    return NULL;
//}

//ContactItem *UserListBase::findContactItem(unsigned id, QTreeWidgetItem *p)
//{
//    for(int c = 0; c < (p ? p->childCount() : topLevelItemCount()); c++)
//    {
//        QTreeWidgetItem *item = (p ? p->child(c) : topLevelItem(c));
//        UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//        if (i->type() == USR_ITEM)
//        {
//            ContactItem *contactItem = static_cast<ContactItem*>(item);
//            if (contactItem->id() == id)
//                return contactItem;
//        }
//        ContactItem *res = findContactItem(id, item);
//        if (res)
//            return res;
//    }
//    return NULL;
//}

//unsigned UserListBase::getUserStatus(Contact *contact, unsigned &style, QString &icons)
//{
//    style = 0;
//    QSet<QString> wrkIcons;
//    QString statusIcon;
//    unsigned long status = contact->contactInfo(style, statusIcon, &wrkIcons);
//    if (!statusIcon.isEmpty())
//        icons = statusIcon;
//    QStringList sl = wrkIcons.toList();
//    icons += QLatin1Char(',') + sl.join(",");
//    return status;
//}

//void UserListBase::deleteItem(QTreeWidgetItem *item)
//{
//    if (item == NULL)
//        return;
//    delete item;
//}

//void UserListBase::select(unsigned int id)
//{
//    ContactItem *pItem = this->findContactItem( id, NULL );
//    if( NULL != pItem )
//        pItem->setCheckState( 0, Qt::Checked );
//}

//QList<unsigned int> UserListBase::selected(QTreeWidgetItem* pItem)
//{
//    QList< unsigned int > list;
//    QList< QTreeWidgetItem* > listSubItems;

//    if( NULL == pItem )
//        for( int i = 0 ; i < topLevelItemCount() ; i++ )
//            listSubItems.push_back( topLevelItem( i ) );
//    else for( int i = 0 ; i < pItem->childCount() ; i++ )
//            listSubItems.push_back( pItem->child( i ) );

//    foreach( QTreeWidgetItem* pSubItem, listSubItems )
//    {
//        UserViewItemBase *pBaseItem = static_cast<UserViewItemBase*>( pSubItem );
//        if( GRP_ITEM == pBaseItem->type() )
//            list.append( selected( pSubItem ) );
//        else if( USR_ITEM == pBaseItem->type() && Qt::Checked == pSubItem->checkState(0) )
//        {
//            ContactItem *pContactItem = static_cast<ContactItem*>( pSubItem );
//            list.push_back( pContactItem->id() );
//        }
//    }

//    return list;
//}


//QList<unsigned int> UserListBase::selected() {
//    return selected( NULL );
//}

//bool UserListBase::isHaveSelected()
//{
//    QList< unsigned int > list = selected();
//    return ( list.count() > 0 );
//}

// vim: set expandtab:

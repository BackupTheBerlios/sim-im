/***************************************************************************
userview.cpp  -  description
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

#include "log.h"

#include "userview.h"
#include "core.h"
#include "contacts/contact.h"
#include "contacts/imcontact.h"
#include "contacts/client.h"
#include "contacts/group.h"
#include "userviewmodel.h"
#include "contacts/contactlist.h"

#include <QPainter>
#include <QPixmap>
#include <QObject>
#include <QMenu>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QWidget>
#include <QCursor>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QFrame>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QToolTip>
#include <QScrollBar>

using namespace std;
using namespace SIM;

const unsigned BLINK_TIMEOUT    = 500;
const unsigned BLINK_COUNT      = 8;

//struct JoinContacts
//{
//    unsigned	contact1;
//    unsigned	contact2;
//};

//static JoinContacts joinContactsData;

UserView::UserView(CorePlugin* plugin, QWidget* parent) : QTreeView(parent),
    m_plugin(plugin),
    m_showOffline(false)
//, m_bBlink		(false)
//, m_bUnreadBlink(false)
//, m_blinkTimer	(new QTimer(this))
//, m_unreadTimer (new QTimer(this))
//, m_current		(NULL)
//, mTipItem		(NULL) //Refactor: rename to m_TipItem
//, m_dropContactId(0)
//, m_dropItem	(NULL)
//, m_searchItem	(NULL)
//, m_edtGroup	(new IntLineEdit(viewport()))
//, m_edtContact	(new IntLineEdit(viewport()))
//, m_userWnd		(NULL)
{
//    m_bShowOnline	=CorePlugin::instance()->value("ShowOnLine").toBool();
//    m_bShowEmpty	=CorePlugin::instance()->value("ShowEmptyGroup").toBool();
//    m_bShowOnline	=CorePlugin::instance()->value("ShowOnLine").toBool();
//    m_bShowEmpty	=CorePlugin::instance()->value("ShowEmptyGroup").toBool();


//    setItemDelegate(new UserViewDelegate(this));
//    setRootIsDecorated(false);
//    setHeaderHidden(true);
//    setAnimated(true);
//    setIndentation(0);
//    setVerticalScrollBarPolicy(CorePlugin::instance()->value("NoScroller").toBool() ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

//    connect(m_blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
//    connect(m_unreadTimer, SIGNAL(timeout()), this, SLOT(unreadBlink()));

//    topLevelWidget()->installEventFilter(this);
//    viewport()->installEventFilter(this);

//    setFrameStyle(QFrame::StyledPanel);
//    setFrameShadow(QFrame::Sunken);
//    EventAddWidget(this, true, EventAddWidget::eMainWindow).process();
//    clear();

//    setGroupMode(CorePlugin::instance()->value("GroupMode").toUInt(), true);

//    m_edtGroup->hide();
//    m_edtContact->hide();
//    QFont font;
//    int size = font.pixelSize();
//    if (size <= 0)
//    {
//        size = font.pointSize();
//        font.setPointSize(size * 3 / 4);
//    }
//    else font.setPixelSize(size * 3 / 4);
//    font.setBold(true);
//    m_edtGroup->setFont(font);
//    connect(m_edtGroup,		SIGNAL(escape()),			this, SLOT(editEscape()));
//    connect(m_edtGroup,		SIGNAL(returnPressed()),	this, SLOT(editGroupEnter()));
//    connect(m_edtGroup,		SIGNAL(lostFocus()),		this, SLOT(editGroupEnter()));
//    connect(m_edtContact,	SIGNAL(escape()),			this, SLOT(editEscape()));
//    connect(m_edtContact,	SIGNAL(returnPressed()),	this, SLOT(editContactEnter()));
//    connect(m_edtContact,	SIGNAL(lostFocus()),		this, SLOT(editContactEnter()));

//    setDragDropMode( QAbstractItemView::DragDrop );
//    setDropIndicatorShown( true );
}

UserView::~UserView()
{
}

bool UserView::init()
{
    setModel(new UserViewModel(SIM::getContactList()));
    setHeaderHidden(true);
    return true;
}

void UserView::setShowOffline(bool s)
{
    m_showOffline = s;
}

bool UserView::isShowOffline() const
{
    return m_showOffline;
}

void UserView::setGroupMode(GroupMode mode)
{
    m_groupMode = mode;
}

UserView::GroupMode UserView::groupMode() const
{
    return m_groupMode;
}

//bool UserView::processEvent(Event *e)
//{
//    switch (e->type())
//    {
//    case eEventRepaintView:
//        setVerticalScrollBarPolicy(CorePlugin::instance()->value("NoScroller").toBool() ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAsNeeded);
//        break;
//    case eEventContact:
//        {
//            EventContact *ec = static_cast<EventContact*>(e);
//            if(ec->action() != EventContact::eOnline)
//                break;
//            Contact *contact = ec->contact();
//            if (m_bInit)
//            {
//                bool bStart = blinks.empty();
//                list<BlinkCount>::iterator it;
//                for (it = blinks.begin(); it != blinks.end(); ++it)
//                {
//                    if (it->id == contact->id())
//                        break;
//                }
//                if (it != blinks.end())
//                {
//                    it->count = BLINK_COUNT;
//                    return false;
//                }
//                BlinkCount bc;
//                bc.id = contact->id();
//                bc.count = BLINK_COUNT;
//                blinks.push_back(bc);
//                if (bStart)
//                    m_blinkTimer->start(BLINK_TIMEOUT);
//                return false;
//            }
//            break;
//        }
//    case eEventMessageReceived:
//    case eEventMessageDeleted:
//    case eEventMessageRead:
//        {
//            EventMessage *em = static_cast<EventMessage*>(e);
//            Message *msg = em->msg();
//            addContactForUpdate(msg->contact());
//            break;
//        }
//    case eEventCommandExec:
//        {
//            EventCommandExec *ece = static_cast<EventCommandExec*>(e);
//            CommandDef *cmd = ece->cmd();
//            log(L_DEBUG, "EventCommandExec: %d", cmd->menu_id);
//            if (cmd->menu_id == MenuContact){
//            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact){
//                    if (cmd->id == CmdContactDelete){
//                        QTreeWidgetItem *item = findContactItem(contact->id());
//                        if (item){
//                        //scrollTo(model()->index(item->row(), item->column()));
//                        QRect rc = visualItemRect(item);
//                        QPoint p = viewport()->mapToGlobal(rc.topLeft());
//                        rc = QRect(p.x(), p.y(), rc.width(), rc.height());
//                        m_bRemoveHistory = CorePlugin::instance()->value("RemoveHistory").toBool();
//                        BalloonMsg::ask((void*)contact->id(),
//                            i18n("Delete \"%1\"?") .arg(contact->getName()),
//                            this, SLOT(deleteContact(void*)), NULL, &rc, NULL,
//                            i18n("Remove history"), &m_bRemoveHistory);
//                        }
//                        return true;
//                    }
//                    if (cmd->id == CmdContactRename){
//                        QTreeWidgetItem *item = findContactItem(contact->id());
//                        if (item){
//                        setCurrentItem(item);
//                        renameContact();
//                        }
//                        return true;
//                    }
//                    if (cmd->id == CmdShowAlways)
//                    {
//                        SIM::PropertyHubPtr data = contact->getUserData("list", true);
//                        if (!data.isNull())
//                        {
//                            bool bShow = false;
//                            if (cmd->flags & COMMAND_CHECKED)
//                                bShow = true;
//                            if (data->value("ShowAlways").toBool() != bShow)
//                            {
//                                data->setValue("ShowAlways", bShow);
//                                EventContact(contact, EventContact::eChanged).process();
//                            }
//                        }
//                        return true;
//                    }
//                    if (cmd->id == CmdClose)
//                    {
//                        UserWnd *wnd = NULL;
//                        QWidgetList list = QApplication::topLevelWidgets();
//                        QWidget * w;
//                        foreach(w,list)
//                        {
//                            if (w->inherits("Container"))
//                            {
//                                Container *c =  static_cast<Container*>(w);
//                                wnd = c->wnd((unsigned long)(cmd->param));
//                                if (wnd)
//                                    break;
//                            }
//                        }
//                        if (wnd)
//                        {
//                            delete wnd;
//                            return true;
//                        }
//                    }
//                    if (cmd->id > CmdSendMessage)
//                    {
//                        Command c;
//                        c->id	   = cmd->id - CmdSendMessage;
//                        c->menu_id = MenuMessage;
//                        c->param   = (void*)(contact->id());
//                        c->flags   = cmd->flags;
//                        EventCommandExec eCmd(c);
//                        if (eCmd.process())
//                            return true;
//                    }
//                }
//            }
//            if (cmd->menu_id == MenuContactGroup)
//            {
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact)
//                {
//                    Group *grp = getContacts()->group(cmd->id - CmdContactGroup);
//                    if (grp && ((int)grp->id() != contact->getGroup()))
//                    {
//                        contact->setGroup(grp->id());
//                        EventContact(contact, EventContact::eChanged).process();
//                        return true;
//                    }
//                }
//            }
//            if (cmd->menu_id == MenuContainer)
//            {
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (contact)
//                {
//                    Container *from = NULL;
//                    Container *to = NULL;
//                    QWidgetList list = QApplication::topLevelWidgets();
//                    QWidget * w;
//                    unsigned max_id = 0;
//                    foreach(w,list)
//                    {
//                        if (w->inherits("Container"))
//                        {
//                            Container *c = static_cast<Container*>(w);
//                            if (c->getId() == cmd->id)
//                                to = c;
//                            if (c->wnd(contact->id()))
//                                from = c;
//                            if (!(c->getId() & CONTAINER_GRP) && max_id < c->getId())
//                                max_id = c->getId();
//                        }
//                    }
//                    if (from && to && from == to)
//                        return true;
//                    if (from)
//                    {
//                        m_userWnd = from->wnd(contact->id());
//                        from->removeUserWnd(m_userWnd);
//                        delete m_userWnd;
//                    }
//                    if (from->wnd(contact->id()) == NULL)
//                        m_userWnd = new UserWnd(contact->id(), NULL, true, true);
//                    if (to == NULL)
//                        to = new Container(max_id + 1);
//                    to->init();
//                    to->addUserWnd(m_userWnd, true);
//                    to->setNoSwitch(true);
//                    raiseWindow(to);
//                    to->setNoSwitch(false);
//                    delete m_userWnd;
//                }
//                return true;
//            }
//            if (cmd->id == CmdOnline)
//            {
//                CorePlugin::instance()->setValue("ShowOnLine", ((cmd->flags & COMMAND_CHECKED) != 0));
//                m_bShowOnline = (cmd->flags & COMMAND_CHECKED);
//                if (cmd->menu_id)
//                {
//                    CommandDef c = *cmd;
//                    c.bar_id	= ToolBarMain;
//                    c.bar_grp   = 0x4000;
//                    EventCommandChange(&c).process();
//                }
//                fill();
//            }
//            if (cmd->id == CmdEmptyGroup)
//            {
//                CorePlugin::instance()->setValue("ShowEmptyGroup", ((cmd->flags & COMMAND_CHECKED) != 0));
//                m_bShowEmpty = (cmd->flags & COMMAND_CHECKED);
//                fill();
//            }
//            if (cmd->id == CmdGrpOff)
//                setGroupMode(0);
//            if (cmd->id == CmdGrpMode1)
//                setGroupMode(1);
//            if (cmd->id == CmdGrpMode2)
//                setGroupMode(2);
//            if (cmd->id == CmdGrpCreate)
//            {
//                if (CorePlugin::instance()->value("GroupMode").toUInt())
//                {
//                    /* Show empty groups because a new group is empty... */
//                    CorePlugin::instance()->setValue("ShowEmptyGroup", true);
//                    m_bShowEmpty = true;
//                    fill();
//                    Group *g = getContacts()->group(0, true);
//                    drawUpdates();
//                    QTreeWidgetItem *item = findGroupItem(g->id());
//                    if (item)
//                    {
//                        setCurrentItem(item);
//                        QTimer::singleShot(0, this, SLOT(renameGroup()));
//                    }
//                }
//                return true;
//            }
//            if (cmd->id == CmdGrpRename)
//            {
//                QTreeWidgetItem *item = findGroupItem((unsigned long)(cmd->param));
//                if (item)
//                {
//                    setCurrentItem(item);
//                    renameGroup();
//                }
//                return true;
//            }
//            if (cmd->id == CmdGrpUp)
//            {
//                unsigned long grp_id = (unsigned long)(cmd->param);
//                getContacts()->moveGroup(grp_id, true);
//                QTreeWidgetItem *item = findGroupItem(grp_id);
//                if (item)
//                    //scrollTo(model()->index(item->row(), item->column()));
//                    setCurrentItem(item);
//                return true;
//            }
//            if (cmd->id == CmdGrpDown)
//            {
//                unsigned long grp_id = (unsigned long)(cmd->param);
//                getContacts()->moveGroup(grp_id, false);
//                QTreeWidgetItem *item = findGroupItem(grp_id);
//                if (item)
//                    //scrollTo(model()->index(item->row(), item->column()));
//                    setCurrentItem(item);
//                return true;
//            }
//            if (cmd->id == CmdGrpDelete)
//            {
//                unsigned long grp_id = (unsigned long)(cmd->param);
//                QTreeWidgetItem *item = findGroupItem(grp_id);
//                Group *g = getContacts()->group(grp_id);
//                if (item && g)
//                {
//                    //scrollTo(model()->index(item->row(), item->column()));
//                    QRect rc = visualItemRect(item);
//                    QPoint p = viewport()->mapToGlobal(rc.topLeft());
//                    rc = QRect(p.x(), p.y(), rc.width(), rc.height());
//                    BalloonMsg::ask((void*)grp_id,
//                        i18n("Delete \"%1\"?") .arg(g->getName()),
//                        this, SLOT(deleteGroup(void*)), NULL, &rc);
//                }
//            }
//            break;
//        }
//    case eEventCheckCommandState:
//        {
//            EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
//            CommandDef *cmd = ecs->cmd();
//            if (cmd->menu_id == MenuGroups)
//            {
//                cmd->flags = cmd->flags & (~COMMAND_CHECKED);
//                if (((cmd->id == CmdGrpOff)   && (CorePlugin::instance()->value("GroupMode").toUInt() == 0)) ||
//                    ((cmd->id == CmdGrpMode1) && (CorePlugin::instance()->value("GroupMode").toUInt() == 1)) ||
//                    ((cmd->id == CmdGrpMode2) && (CorePlugin::instance()->value("GroupMode").toUInt() == 2)) ||
//                    ((cmd->id == CmdOnline)   &&  CorePlugin::instance()->value("ShowOnLine").toBool()    ))
//                    cmd->flags |= COMMAND_CHECKED;
//                if (cmd->id == CmdEmptyGroup)
//                {
//                    if (CorePlugin::instance()->value("GroupMode").toUInt() == 0)
//                        return false;
//                    if (CorePlugin::instance()->value("ShowEmptyGroup").toBool())
//                        cmd->flags |= COMMAND_CHECKED;
//                }
//                return true;
//            }
//            if (cmd->menu_id == MenuContact)
//            {
//                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                if (cmd->id == CmdContactTitle && contact)
//                {
//                    cmd->text_wrk = contact->getName();
//                    return true;
//                }
//                if (cmd->id == CmdShowAlways && contact)
//                {
//                    SIM::PropertyHubPtr data = contact->getUserData("list", true);
//                    cmd->flags &= ~COMMAND_CHECKED;
//                    if (!data.isNull() && data->value("ShowAlways").toBool())
//                        cmd->flags |= COMMAND_CHECKED;
//                    return true;
//                }
//                if (cmd->id == CmdClose)
//                {
//                    UserWnd *wnd = NULL;
//                    QWidgetList list = QApplication::topLevelWidgets();
//                    QWidget * w;
//                    foreach(w,list)
//                    {
//                        if (w->inherits("Container"))
//                        {
//                            wnd = static_cast<Container*>(w)->wnd((unsigned long)(cmd->param));
//                            if (wnd)
//                                break;
//                        }
//                    }
//                    if (wnd)
//                        return true;
//                }
//                if (cmd->id == CmdSendMessage)
//                {
//                    EventMenuGetDef eMenu(MenuMessage);
//                    eMenu.process();
//                    CommandsDef *cmdsMsg = eMenu.defs();
//                    unsigned nCmds = 1;
//                    {
//                        CommandsList it(*cmdsMsg, true);
//                        while (++it)
//                            nCmds++;
//                    }

//                    CommandDef *cmds = new CommandDef[nCmds];
//                    nCmds = 0;

//                    CommandsList it(*cmdsMsg, true);
//                    CommandDef *c;
//                    while ((c = ++it) != NULL)
//                    {
//                        cmds[nCmds] = *c;
//                        cmds[nCmds].id = CmdSendMessage + c->id;
//                        cmds[nCmds].menu_id = MenuContact;
//                        nCmds++;
//                    }
//                    cmd->param = cmds;
//                    cmd->flags |= COMMAND_RECURSIVE;
//                    return true;
//                }
//                if (cmd->id > CmdSendMessage)
//                {
//                    Command c;
//                    c->id	   = cmd->id - CmdSendMessage;
//                    c->menu_id = MenuMessage;
//                    c->param   = cmd->param;
//                    bool res = EventCheckCommandState(c).process();
//                    if (res && (c->flags & COMMAND_RECURSIVE))
//                    {
//                        cmd->flags |= COMMAND_RECURSIVE;
//                        cmd->param = c->param;
//                    }
//                    if (res)
//                        cmd->flags = c->flags;
//                    return res;
//                }
//            }
//            if (cmd->menu_id == MenuContactGroup)
//            {
//                if (cmd->id == CmdContactGroup)
//                {
//                    unsigned grpId = 0;
//                    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
//                    if (contact)
//                        grpId = contact->getGroup();
//                    unsigned nGroups = 0;
//                    Group *grp;
//                    ContactList::GroupIterator it;
//                    while ((grp = ++it) != NULL)
//                        nGroups++;
//                    CommandDef *cmds = new CommandDef[nGroups + 1];
//                    it.reset();
//                    nGroups = 0;
//                    while ((grp = ++it) != NULL)
//                    {
//                        if (grp->id() == 0) continue;
//                        CommandDef &c = cmds[nGroups++];
//                        c = *cmd;
//                        c.id = CmdContactGroup + grp->id();
//                        c.flags = COMMAND_DEFAULT;
//                        if (grp->id() == grpId && contact->id())
//                            c.flags |= COMMAND_CHECKED;
//                        c.text_wrk = grp->getName();
//                    }
//                    CommandDef &c = cmds[nGroups++];
//                    c = *cmd;
//                    c.text = I18N_NOOP("Not in list");
//                    c.id = CmdContactGroup;
//                    c.flags = COMMAND_DEFAULT;
//                    if (grpId == 0)
//                        c.flags = COMMAND_CHECKED;
//                    cmds[nGroups].clear();
//                    cmd->flags |= COMMAND_RECURSIVE;
//                    cmd->param = cmds;
//                    return true;
//                }
//            }
//            if (cmd->menu_id == MenuGroup)
//            {
//                unsigned long grp_id = (unsigned long)(cmd->param);
//                if (grp_id)
//                {
//                    Group *lgroup = getContacts()->group(grp_id);
//                    if (cmd->id == CmdGrpTitle && lgroup)
//                    {
//                        cmd->text_wrk = lgroup->getName();
//                        return true;
//                    }
//                    if (cmd->id == CmdGrpDelete || cmd->id == CmdGrpRename)
//                    {
//                        cmd->flags &= ~COMMAND_CHECKED;
//                        return true;
//                    }
//                    if (cmd->id == CmdGrpUp)
//                    {
//                        if (getContacts()->groupIndex(grp_id) <= 1)
//                            cmd->flags |= COMMAND_DISABLED;
//                        cmd->flags &= ~COMMAND_CHECKED;
//                        return true;
//                    }
//                    if (cmd->id == CmdGrpDown)
//                    {
//                        if (getContacts()->groupIndex(grp_id) >= getContacts()->groupCount() - 1)
//                            cmd->flags |= COMMAND_DISABLED;
//                        cmd->flags &= ~COMMAND_CHECKED;
//                        return true;
//                    }
//                }
//                else if (cmd->id == CmdGrpTitle)
//                {
//                    cmd->text = I18N_NOOP("Not in list");
//                    return true;
//                }
//            }
//            if (cmd->id == CmdGrpCreate)
//            {
//                cmd->flags &= ~COMMAND_CHECKED;
//                return CorePlugin::instance()->value("GroupMode").toUInt() ? true : false;
//            }
//            break;
//        }
//    case eEventIconChanged:
//        viewport()->repaint();
//        break;
//    case eEventRaiseWindow:
//        {
//            EventRaiseWindow *w = static_cast<EventRaiseWindow*>(e);
//            QWidget *o = w->widget();
//            if (o && o->inherits("MainWindow"))
//                QTimer::singleShot(0, this, SLOT(adjustColumn()));
//            break;
//        }
//    default:
//        break;
//    }
//    return UserListBase::processEvent(e);
//}

//void UserView::deleteGroup(void *p)
//{
//    Group *grp = getContacts()->group((unsigned long)p);
//    if (grp)
//        delete grp;
//}

//void UserView::deleteContact(void *p)
//{
//    Contact *contact = getContacts()->contact((unsigned long)p);
//    if (contact == NULL)
//        return;
//    ContactItem *item = findContactItem(contact->id());
//    if (item)
//        setCurrentItem(item);

//    // Looking for unread messages for this contact in order to delete them
//    int no_more_messages_flag;
//    do
//    {
//      no_more_messages_flag = 1;
//      // we should restart unread messages iteration after each message deletion
//      // because deleting message will change "unread" list
//      for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it)
//      {
//          msg_id &message_id = *it;
//          if ( message_id.contact == contact->id())
//          {
//            SIM::Message * message;
//            message = History::load(message_id.id,message_id.client,message_id.contact);
//            EventMessageDeleted(message).process();
//            // may be we should do EventMessageRead instead of EventMessageDeleted when m_bRemoveHistory is flase
//            // I am not sure. shaplov.
//            no_more_messages_flag = 0;
//            break;
//          }
//       }
//    }
//    while (!no_more_messages_flag);

//    CorePlugin::instance()->setValue("RemoveHistory", m_bRemoveHistory);
//    if (!m_bRemoveHistory)
//        contact->setFlags(contact->getFlags() | CONTACT_NOREMOVE_HISTORY);
//    delete contact;
//}

//void UserView::renameGroup()
//{
//    QTreeWidgetItem *item = currentItem();
//    if (item == NULL)
//        return;
//    UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//    if (i->type() != GRP_ITEM)
//        return;
//    GroupItem *grpItem = static_cast<GroupItem*>(item);
//    Group *lgroup = getContacts()->group(grpItem->id());
//    if (lgroup)
//    {
//        //scrollTo(model()->index(item->row(), item->column()));
//        QString name = lgroup->getName();
//        QRect rc = visualItemRect(item);
//        rc.setLeft(rc.left() + 18);
//        m_edtGroup->id = lgroup->id();
//        m_edtGroup->setGeometry(rc);
//        m_edtGroup->setText(name.length() ? name : i18n("New group"));
//        m_edtGroup->setSelection(0, m_edtGroup->text().length());
//        m_edtGroup->show();
//        m_edtGroup->setFocus();
//    }
//}

//void UserView::renameContact()
//{
//    QTreeWidgetItem *item = currentItem();
//    if (item == NULL)
//        return;
//    UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
//    if (i->type() != USR_ITEM)
//        return;
//    ContactItem *contactItem = static_cast<ContactItem*>(item);
//    Contact *contact = getContacts()->contact(contactItem->id());
//    if (contact)
//    {
//        //scrollTo(model()->index(item->row(), item->column()));
//        QString name = contact->getName();
//        QRect rc = visualItemRect(item);
//        rc.setLeft(rc.left() + 18);
//        m_edtContact->id = contact->id();
//        m_edtContact->setGeometry(rc);
//        m_edtContact->setText(name);
//        m_edtContact->setSelection(0, m_edtContact->text().length());
//        m_edtContact->show();
//        m_edtContact->setFocus();
//    }
//}

//void UserView::setGroupMode(unsigned mode, bool bFirst)
//{
//    if (!bFirst && CorePlugin::instance()->value("GroupMode").toUInt() == mode)
//        return;
//    CorePlugin::instance()->setValue("GroupMode", mode);
//    m_groupMode = mode;
//    EventUpdateCommandState(CmdGroupToolbarButton).process();
//    fill();
//}

//bool UserView::eventFilter(QObject *obj, QEvent *e)
//{
//    bool res = ListView::eventFilter(obj, e);
//    if (obj->inherits("QMainWindow") &&
//        e->type() == QEvent::Show)
//        QTimer::singleShot(0, this, SLOT(repaintView()));
//    return res;
//}

//void UserView::mousePressEvent(QMouseEvent *e)
//{
//    stopSearch();
//    UserListBase::mousePressEvent(e);
//}

//void UserView::focusOutEvent(QFocusEvent *e)
//{
//    stopSearch();
//    UserListBase::focusOutEvent(e);
//}

//void UserView::mouseReleaseEvent(QMouseEvent *e)
//{
//    QTreeWidgetItem *item = m_pressedItem;
//    UserListBase::mouseReleaseEvent(e);
//    if (!item || CorePlugin::instance()->value("UseDblClick").toBool())
//        return;

//    m_current = item;
//    QTimer::singleShot(0, this, SLOT(doClick()));
//}

//void UserView::mouseDoubleClickEvent(QMouseEvent *e)
//{
//    UserListBase::mouseDoubleClickEvent(e);
//    m_current = itemAt(e->pos());
//    QTimer::singleShot(0, this, SLOT(doClick()));
//}

//void UserView::doClick()
//{
//    if (m_current == NULL)
//        return;
//    if (!CorePlugin::instance()->value("UseDblClick").toBool())
//        m_current->setExpanded(!m_current->isExpanded());
//    if (static_cast<UserViewItemBase*>(m_current)->type() == USR_ITEM)
//    {
//        ContactItem *item = static_cast<ContactItem*>(m_current);
//        EventDefaultAction(item->id()).process();
//    }
//    m_current = NULL;
//}

//void UserView::keyPressEvent(QKeyEvent *e)
//{
//    if (CorePlugin::instance()->value("UseDblClick").toBool() || m_searchItem)
//    {
//        if (m_searchItem)
//        {
//            int store = 0;
//            list<QTreeWidgetItem*> items;
//            list<QTreeWidgetItem*>::iterator it;
//            search(items);
//            if (!items.empty())
//            {
//                for (it = items.begin(); it != items.end(); ++it)
//                    if (*it == m_searchItem)
//                        store = 1;
//                if (!store) m_searchItem = items.front();
//            }
//            else
//            {
//                m_search = QString::null;
//                m_searchItem = NULL;
//            }
//            setCurrentItem(m_searchItem);
//        }
//        if (e->key() == Qt::Key_Return ||
//            e->key() == Qt::Key_Enter)
//        {
//            m_current = currentItem();
//            QTimer::singleShot(0, this, SLOT(doClick()));
//            return;
//        }
//    }
//    bool bTip = false;
//    if (m_searchItem && (m_searchItem == mTipItem))
//        bTip = true;
//    list<QTreeWidgetItem*> old_items;
//    list<QTreeWidgetItem*> new_items;
//    switch (e->key())
//    {
//    case Qt::Key_Backspace:
//        if (m_search.isEmpty())
//        {
//            UserListBase::keyPressEvent(e);
//            return;
//        }
//        search(old_items);
//        m_search = m_search.left(m_search.length() - 1);
//        if (m_search.isEmpty())
//        {
//            m_searchItem = NULL;
//            list<QTreeWidgetItem*>::iterator it;
//            for (it = closed_items.begin(); it != closed_items.end(); ++it)
//                (*it)->setExpanded(false);
//        }
//        else
//        {
//            search(new_items);
//            if (new_items.empty())
//            {
//                m_search = QString::null;
//                m_searchItem = NULL;
//            }
//            else
//                m_searchItem = new_items.front();
//        }
//        break;
//    case Qt::Key_Escape:
//        if (m_search.isEmpty())
//        {
//            UserListBase::keyPressEvent(e);
//            return;
//        }
//        stopSearch();
//        return;
//    case Qt::Key_Up:
//        if (m_search.isEmpty())
//        {
//            UserListBase::keyPressEvent(e);
//            return;
//        }
//        if (m_searchItem)
//        {
//            search(old_items);
//            list<QTreeWidgetItem*>::iterator it_old;
//            for (it_old = old_items.begin(); it_old != old_items.end(); ++it_old)
//                if ((*it_old) == m_searchItem)
//                    break;
//            if (it_old != old_items.begin())
//                it_old--;
//            if (it_old == old_items.begin())
//            {
//                QApplication::beep();
//                return;
//            }
//            m_searchItem = *it_old;
//        }
//        break;
//    case Qt::Key_Down:
//        if (m_search.isEmpty())
//        {
//            UserListBase::keyPressEvent(e);
//            return;
//        }
//        if (m_searchItem)
//        {
//            search(old_items);
//            list<QTreeWidgetItem*>::iterator it_old;
//            for (it_old = old_items.begin(); it_old != old_items.end(); ++it_old)
//                if (*it_old == m_searchItem)
//                    break;
//            if (it_old != old_items.end())
//                it_old++;
//            if (it_old == old_items.end())
//            {
//                QApplication::beep();
//                return;
//            }
//            m_searchItem = *it_old;
//        }
//        break;
//    case Qt::Key_Plus:
//    case Qt::Key_Minus:
//        if (m_search.isEmpty())
//        {
//            QTreeWidgetItem *item = currentItem();
//            if (item)
//            {
//                UserListBase::keyPressEvent(e);
//                return;
//            }
//        }
//    case Qt::Key_Delete:
//        // e->text() is not empty, but we don't need to specially handle Del
//        UserListBase::keyPressEvent(e);
//        return;
//    default:
//        QString t = e->text();
//        if (t.isEmpty())
//        {
//            UserListBase::keyPressEvent(e);
//            return;
//        }
//        if (m_search.isEmpty())
//        {
//            closed_items.clear();
//            for(int c = 0; c < topLevelItemCount(); c++)
//            {
//                QTreeWidgetItem *item = topLevelItem(c);
//                if (!(item->isExpanded()))
//                    closed_items.push_back(item);
//            }
//        }
//        QString save_search = m_search;
//        search(old_items);
//        m_search += t;
//        search(new_items);
//        if (new_items.empty())
//        {
//            m_search = save_search;
//            search(new_items);
//            QApplication::beep();
//            return;
//        }
//        else
//            m_searchItem = new_items.front();
//    }
//    list<QTreeWidgetItem*>::iterator it_old;
//    list<QTreeWidgetItem*>::iterator it_new;
//    for (it_old = old_items.begin(); it_old != old_items.end(); ++it_old)
//    {
//        for (it_new = new_items.begin(); it_new != new_items.end(); ++it_new)
//            if (*it_new == *it_old)
//                break;
//        if (it_new == new_items.end())
//            new_items.push_back(*it_old);
//    }
//    for (it_new = new_items.begin(); it_new != new_items.end(); ++it_new)
//        update(indexFromItem((*it_new)));
//    setCurrentItem(m_searchItem);
//    if (m_searchItem)
//    {
//        //scrollTo(model()->index(m_searchItem->row(), m_searchItem->column()));
//    }
//    if (m_search.isEmpty() || (m_searchItem == NULL))
//    {
//        QToolTip::hideText();
//        return;
//    }
//    QString tip = i18n("Search: %1") .arg(m_search);
//    QRect tipRect = visualItemRect(m_searchItem);
//    QPoint p = viewport()->mapToGlobal(tipRect.topLeft());
//    QToolTip::showText( mapToGlobal( tipRect.topLeft() ), tip, this, tipRect );
//}

//void UserView::stopSearch()
//{
//    if (m_search.isEmpty())
//        return;
//    list<QTreeWidgetItem*> old_items;
//    search(old_items);
//    m_search = QString::null;
//    m_searchItem = NULL;
//    list<QTreeWidgetItem*>::iterator it_old;
//    for (it_old = old_items.begin(); it_old != old_items.end(); ++it_old)
//        update(indexFromItem((*it_old)));
//    QToolTip::hideText();
//}

//bool UserView::getMenu(QTreeWidgetItem *list_item, unsigned long &id, void* &param)
//{
//    if (list_item == NULL)
//        return false;

//    UserViewItemBase *item = static_cast<UserViewItemBase*>(list_item);
//    switch (item->type()){
//    case GRP_ITEM:{
//            GroupItem *grpItem = static_cast<GroupItem*>(item);
//            id    = MenuGroup;
//            param = (void*)(grpItem->id());
//            return true;
//        }
//    case USR_ITEM:{
//            ContactItem *contactItem = static_cast<ContactItem*>(item);
//            id    = MenuContact;
//            param = (void*)(contactItem->id());
//            return true;
//        }
//    }
//    return false;
//}

//void UserView::editEscape()
//{
//    m_edtGroup->hide();
//    m_edtContact->hide();
//}

//void UserView::editGroupEnter()
//{
//    m_edtGroup->hide();
//    Group *g = getContacts()->group(m_edtGroup->id);
//    if (!(g && m_edtGroup->text().length())) return;
//    g->setName(m_edtGroup->text());
//    EventGroup e(g, EventGroup::eChanged);
//    e.process();
//}

//void UserView::editContactEnter()
//{
//    m_edtContact->hide();
//    Contact *c = getContacts()->contact(m_edtContact->id);
//    if (!(c && m_edtContact->text().length())) return;
//    c->setName(m_edtContact->text());
//    EventContact(c, EventContact::eChanged).process();
//}

//unsigned UserView::getUnread(unsigned contact_id)
//{
//    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it)
//        if (it->contact == contact_id)
//        {
//            if (!m_unreadTimer->isActive())
//            {
//                m_bUnreadBlink = true;
//                m_unreadTimer->start(BLINK_TIMEOUT);
//            }
//            return it->type;
//        }
//    return 0;
//}

//static void resetUnread(QTreeWidgetItem *item, list<QTreeWidgetItem*> &grp)
//{
//    if (static_cast<UserViewItemBase*>(item)->type() == GRP_ITEM)
//    {
//        list<QTreeWidgetItem*>::iterator it;
//        for (it = grp.begin(); it != grp.end(); ++it)
//            if ((*it) == item)
//                break;
//        if (it == grp.end())
//        {
//            GroupItem *group = static_cast<GroupItem*>(item);
//            if (group->m_unread)
//            {
//                group->m_unread = 0;
//                if (!group->isExpanded())
//                    group->treeWidget()->repaint();
//            }
//        }
//    }
//    for(int c = 0; c < item->childCount(); c++)
//    {
//        QTreeWidgetItem *i = item->child(c);
//        resetUnread(i, grp);
//    }
//}

//void UserView::unreadBlink()
//{
//    m_bUnreadBlink = !m_bUnreadBlink;
//    list<unsigned> blinks;
//    list<unsigned>::iterator itb;
//    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it)
//    {
//        for (itb = blinks.begin(); itb != blinks.end(); ++itb)
//            if (*itb == it->contact)
//                break;
//        if (itb != blinks.end())
//            continue;
//        blinks.push_back(it->contact);
//    }
//    list<QTreeWidgetItem*> grps;
//    if (blinks.empty())
//        m_unreadTimer->stop();
//    else
//	{
//        for (itb = blinks.begin(); itb != blinks.end(); ++itb)
//        {
//            ContactItem *contact = findContactItem((*itb), NULL);
//            if (contact == NULL)
//                return;
//            update();
//            //repaintItem(contact);
//            if (CorePlugin::instance()->value("GroupMode").toUInt() && !contact->parent()->isExpanded())
//            {
//                GroupItem *group = static_cast<GroupItem*>(contact->parent());
//                group->m_unread = contact->m_unread;
//                update();
//                grps.push_back(group);
//            }
//        }
//    }
//    if (CorePlugin::instance()->value("GroupMode").toUInt())
//    {
//        for(int c = 0; c < topLevelItemCount(); c++)
//        {
//            QTreeWidgetItem *i = topLevelItem(c);
//            resetUnread(i, grps);
//        }
//    }
//}

//void UserView::blink()
//{
//    m_bBlink = !m_bBlink;
//    list<BlinkCount>::iterator it;
//    for (it = blinks.begin(); it != blinks.end();)
//    {
//        ContactItem *contact = findContactItem(it->id, NULL);
//        if (contact == NULL)
//        {
//            blinks.erase(it);
//            it = blinks.begin();
//            break;
//        }
//        contact->m_bBlink = m_bBlink;
//        update();
//        //repaintItem(contact);
//        ++it;
//    }
//    if (m_bBlink)
//        return;
//    for (it = blinks.begin(); it != blinks.end(); ++it)
//        it->count--;
//    for (it = blinks.begin(); it != blinks.end(); )
//    {
//        if (it->count)
//        {
//            ++it;
//            continue;
//        }
//        blinks.erase(it);
//        it = blinks.begin();
//    }
//    if (blinks.size() == 0)
//        m_blinkTimer->stop();
//}

//void UserView::deleteItem(QTreeWidgetItem *item)
//{
//    if (item == NULL)
//        return;
//    if (item == m_pressedItem)
//        m_pressedItem = NULL;
//    if (item == m_searchItem)
//        stopSearch();
//    UserListBase::deleteItem(item);
//}

//class UserViewContactDragObject : public ContactDragObject
//{
//public:
//    UserViewContactDragObject( const UserView *view, Contact *contact );
//    virtual ~UserViewContactDragObject();
//};

//UserViewContactDragObject::UserViewContactDragObject( const UserView *view, Contact *contact )
//    : ContactDragObject( contact )
//{
//    QTimer *dragTimer = new QTimer(this);
//    connect(dragTimer, SIGNAL(timeout()), view, SLOT(dragScroll()));
//    dragTimer->start(200);
//}

//UserViewContactDragObject::~UserViewContactDragObject()
//{
//}

//QMimeData *UserView::mimeData( const QList<QTreeWidgetItem *> items ) const
//{
//    if( items.count() != 1 )
//        return NULL;

//    QTreeWidgetItem *pItem = items.first();

//    UserViewItemBase *base_item = static_cast<UserViewItemBase*>(pItem);
//    if (base_item->type() != USR_ITEM)
//        return NULL;
//    ContactItem *item = static_cast<ContactItem*>(base_item);
//    Contact *contact = getContacts()->contact(item->id());
//    if (contact == NULL)
//        return NULL;
//    return new UserViewContactDragObject(this, contact);
//}

//QMimeData *UserView::dragObject()
//{
//    UserViewItemBase *base_item = static_cast<UserViewItemBase*>(currentItem());
//    ContactItem *item = static_cast<ContactItem*>(currentItem());
//    Contact *contact = getContacts()->contact(item->id());
//    if (currentItem() == NULL ||
//        contact == NULL ||
//        base_item->type() != USR_ITEM )
//        return NULL;
//    return new UserViewContactDragObject(this, contact);
//}

//void UserView::dragEnterEvent(QDragEnterEvent *e)
//{
//    dragEvent(e, false);
//}

//void UserView::dragMoveEvent(QDragMoveEvent *e)
//{
//    dragEvent(e, false);
//}

//void UserView::dropEvent(QDropEvent *e)
//{
//    dragEvent(e, true);
//}

//void UserView::dragEvent(QDropEvent *e, bool isDrop)
//{
//    QTreeWidgetItem *list_item = itemAt(e->pos());
//    if (list_item == NULL)
//    {
//        e->ignore();
//        return;
//    }
//    UserViewItemBase *item = static_cast<UserViewItemBase*>(list_item);
//    switch (item->type())
//    {
//    case GRP_ITEM:
//        if (ContactDragObject::canDecode(e))
//        {
//            if (isDrop)
//            {
//                Contact *contact = ContactDragObject::decode(e);
//                m_dropItem = item;
//                m_dropContactId = contact->id();
//                contact->setFlags(contact->getFlags() & ~CONTACT_DRAG);
//                QTimer::singleShot(0, this, SLOT(doDrop()));
//            }
//            e->setDropAction( Qt::MoveAction );
//            e->accept();
//            return;
//        }
//        break;
//    case USR_ITEM:
//        {
//            if (ContactDragObject::canDecode(e))
//            {
//                Contact *contact = ContactDragObject::decode(e);
//                if (static_cast<ContactItem*>(item)->id() == contact->id())
//                {
//                    e->setDropAction( Qt::IgnoreAction );
//                    e->accept();
//                    return;
//                }
//                if (isDrop)
//                {
//                    m_dropItem = item;
//                    m_dropContactId = contact->id();
//                    contact->setFlags(contact->getFlags() & ~CONTACT_DRAG);
//                    QTimer::singleShot(0, this, SLOT(doDrop()));
//                    e->ignore();
//                    return;
//                }
//                e->setDropAction( Qt::MoveAction );
//                e->accept();
//            }
//            Message *msg = NULL;
//            CommandDef *cmd;
//            CommandsMapIterator it(CorePlugin::instance()->messageTypes);
//            while ((cmd = ++it) != NULL)
//            {
//                MessageDef *def = (MessageDef*)(cmd->param);
//                if (def && def->drag)
//                {
//                    msg = def->drag(e);
//                    if (msg)
//                    {
//                        unsigned type = cmd->id;
//                        Command cmd;
//                        cmd->id      = type;
//                        cmd->menu_id = MenuMessage;
//                        cmd->param	 = (void*)(static_cast<ContactItem*>(item)->id());
//                        if (EventCheckCommandState(cmd).process())
//                            break;
//                    }
//                }
//            }
//            if (msg)
//            {
//                if (isDrop)
//                {
//                    msg->setContact(static_cast<ContactItem*>(item)->id());
//                    EventOpenMessage(msg).process();
//                }
//                delete msg;
//                return;
//            }
//            if (!e->mimeData()->text().isEmpty())
//            {
//                QString str = e->mimeData()->text();
//                e->accept();
//                if (isDrop)
//                {
//                    Message *msg = new Message(MessageGeneric);
//                    msg->setText(str);
//                    msg->setContact(static_cast<ContactItem*>(item)->id());
//                    EventOpenMessage(msg).process();
//                    delete msg;
//                }
//                return;
//            }
//            break;
//        }
//    }
//    e->accept();
//}

//void UserView::doDrop()
//{
//    if (m_dropItem == NULL)
//        return;
//    Contact *contact = getContacts()->contact(m_dropContactId);
//    if (contact == NULL)
//        return;
//    switch (static_cast<UserViewItemBase*>(m_dropItem)->type()){
//    case GRP_ITEM:
//        {
//            GroupItem *grp_item = static_cast<GroupItem*>(m_dropItem);
//            contact->setGroup(grp_item->id());
//            contact->setIgnore(false);
//            contact->setFlags(contact->getFlags() & ~CONTACT_TEMPORARY);
//            EventContact(contact, EventContact::eChanged).process();
//            break;
//        }
//    case USR_ITEM:
//        {
//            ContactItem *contact_item = static_cast<ContactItem*>(m_dropItem);
//            Contact *contact1 = getContacts()->contact(contact_item->id());
//            if (contact1 == NULL)
//                break;
//            joinContactsData.contact1 = contact_item->id();
//            joinContactsData.contact2 = m_dropContactId;
//            //scrollTo(model()->index(contact_item->row(), contact_item->column()));
//            QRect rc = visualItemRect(contact_item);
//            QPoint p = viewport()->mapToGlobal(rc.topLeft());
//            rc = QRect(p.x(), p.y(), rc.width(), rc.height());
//            BalloonMsg::ask(NULL,
//                            i18n("Join \"%1\" and \"%2\"?")
//                            .arg(contact1->getName())
//                            .arg(contact->getName()),
//                            this,
//                            SLOT(joinContacts(void*)),
//                            SLOT(cancelJoinContacts(void*)), &rc);
//            break;
//        }
//    }
//    m_dropContactId = 0;
//    m_dropItem = NULL;
//}

//void UserView::joinContacts(void*)
//{
//    Contact *contact1 = getContacts()->contact(joinContactsData.contact1);
//    Contact *contact2 = getContacts()->contact(joinContactsData.contact2);
//    if (contact1 == NULL || contact2 == NULL)
//        return;
//    contact1->join(contact2);
//    if (!contact2->getPhones().isEmpty())
//    {
//        QString phones = contact1->getPhones();
//        if (!phones.isEmpty())
//            phones += ';';
//        phones += contact2->getPhones();
//        contact1->setPhones(phones);
//    }
//    if (!contact2->getEMails().isEmpty())
//    {
//        QString mails = contact1->getEMails();
//        if (!mails.isEmpty())
//            mails += ';';
//        mails += contact2->getEMails();
//        contact1->setEMails(mails);
//    }
//    delete contact2;
//    contact1->setup();
//    EventContact(contact1, EventContact::eChanged).process();
//}

//void UserView::cancelJoinContacts(void*)
//{
//    Contact *contact2 = getContacts()->contact(joinContactsData.contact2);
//    if (contact2 && (contact2->getFlags() & CONTACT_TEMPORARY))
//        delete contact2;
//}

//void UserView::sortAll()
//{
//    for(int c = 0; c < topLevelItemCount(); c++)
//        sortAll(topLevelItem(c));
//}

//void UserView::sortAll(QTreeWidgetItem *item)
//{
//    for(int c = 0; c < item->childCount(); c++)
//        sortAll(item->child(c));
//}

//void UserView::search(list<QTreeWidgetItem*> &items)
//{
//    if (m_search.isEmpty())
//        return;
//    list<QTreeWidgetItem*>::iterator it;
//    for (it = closed_items.begin(); it != closed_items.end(); ++it)
//        (*it)->setExpanded(false);
//    for(int c = 0; c < topLevelItemCount(); c++)
//    {
//        QTreeWidgetItem *item = topLevelItem(c);
//        search(item, items);
//    }
//}

//void UserView::search(QTreeWidgetItem *item, list<QTreeWidgetItem*> &items)
//{
//    for(int c = 0; c < item->childCount(); c++)
//    {
//        QTreeWidgetItem *ch = item->child(c);
//        search(ch, items);
//    }
//    if (static_cast<UserViewItemBase*>(item)->type() != USR_ITEM)
//        return;
//    QString name = item->text(CONTACT_TEXT);
//    //log(L_DEBUG, "Contact List search: Examining name %s", (const char *)name.local8Bit());
//    //Search from the beginning of contact name
//    //if (name.left(m_search.length()).upper() == m_search.upper())
//    //Search for substring in contact name
//    if (name.contains(m_search,Qt::CaseInsensitive)>0)
//    {
//        //log(L_DEBUG, "Contact List search: Found name %s", (const char *)name.local8Bit());
//        item->parent()->setExpanded(true);
//        items.push_back(item);
//    }
//    else
//    {
//        void *data;
//        Contact *contact = getContacts()->contact(static_cast<ContactItem*>(item)->id());
//        ClientDataIterator it = contact->clientDataIterator();
//        while ((data = ++it) != NULL)
//		{
//            Client *client = contact->activeClient(data, it.client());
//			if (client == NULL)
//				continue;
//			QString contactName = client->contactName(data);
//			//log(L_DEBUG, "Contact List search: Examining ID %s", (const char *)contactName.local8Bit());
//			if (contactName.contains(m_search,Qt::CaseInsensitive)>0)
//			{
//				//log(L_DEBUG, "Contact List search: Found ID %s", (const char *)contactName.local8Bit());
//				item->parent()->setExpanded(true);
//				items.push_back(item);
//				break;
//			}
//        }
//    }
//}

//void UserView::dragScroll() //rewrite!?
//{
//    QPoint pos = QCursor::pos();
//    pos = viewport()->mapFromGlobal(pos);
//    if (pos.x() < 0 || pos.x() > viewport()->width())
//        return;
//    QTreeWidgetItem *item = NULL;
//    if (pos.y() < 0)
//	{
//        pos = QPoint(pos.x(), -1);
//        item = itemAt(pos);
//    }
//	else if (pos.y() > viewport()->height())
//	{
//        pos = QPoint(pos.x(), viewport()->height() - 1);
//        item = itemAt(pos); //<== FIXME: crash, it does not return item, sometimes in QGList append() no mem allocation is possible :-/ ???
//        if (item)
//		{
//            pos = QPoint(pos.x(), viewport()->height() - 1 + item->sizeHint(0).height());
//            item = itemAt(pos);
//        }
//    }
    
//    //if (item)
//        //scrollTo(model()->index(item->row(), item->column()));
//}

// vim: set expandtab:

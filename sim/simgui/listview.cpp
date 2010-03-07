/***************************************************************************
                          listview.cpp  -  description
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

#include "simgui/listview.h"
#include "contacts/contact.h"
#include "log.h"

#include <QMenu>
#include <QTimer>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDragEnterEvent>
#include <QScrollBar>

#include "misc.h"

using namespace SIM;

bool ListView::s_bInit = false;

ListViewItem::ListViewItem() : QTreeWidgetItem(), m_open(true)
{
    setExpanded(true);
}

ListViewItem::ListViewItem(const QString& /* name */) : QTreeWidgetItem()
{
    setExpanded(true);
}

ListViewItem::ListViewItem(ListView* parent) : QTreeWidgetItem(parent)
{
    setExpanded(true);
}

ListViewItem::ListViewItem(ListViewItem* parent) : QTreeWidgetItem(parent)
{
    setExpanded(true);
}

ListViewItem::~ListViewItem()
{
}

ListView* ListViewItem::listView() const
{
    return static_cast<ListView*>(treeWidget());
}

QPixmap ListViewItem::pixmap(int t)
{
    return icon(t).pixmap(QSize(16,16));
}

void ListViewItem::setPixmap(int col, QPixmap p)
{
    setIcon(col, p);
}

void ListViewItem::repaint()
{
    emitDataChanged();
}

bool ListViewItem::isOpen()
{
    return QTreeWidgetItem::isExpanded();
}

void ListViewItem::setOpen(bool o)
{
    return QTreeWidgetItem::setExpanded(o);
}

bool ListViewItem::isExpandable()
{
    return m_expandable;
}

void ListViewItem::setExpandable(bool e)
{
    m_expandable = e;
}

ListView::ListView(QWidget *parent) : QTreeWidget(parent)
{
    m_menuId = MenuListView;
    if (!s_bInit){
        s_bInit = true;
        EventMenu(MenuListView, EventMenu::eAdd).process();

        Command cmd;
        cmd->id			= CmdListDelete;
        cmd->text		= I18N_NOOP("&Delete");
        cmd->icon		= "remove";
        cmd->accel		= "Del";
        cmd->menu_id	= MenuListView;
        cmd->menu_grp	= 0x1000;
        cmd->flags		= COMMAND_DEFAULT;

        EventCommandCreate(cmd).process();
    }
    setColumnCount(0);
    //setAllColumnsShowFocus(true);
    m_bAcceptDrop = false;
    viewport()->setAcceptDrops(true);
    m_pressedItem = NULL;
    m_expandingColumn = -1;
    verticalScrollBar()->installEventFilter(this);
    //connect(header(), SIGNAL(sizeChange(int,int,int)), this, SLOT(sizeChange(int,int,int)));
    m_resizeTimer = new QTimer(this);
    connect(m_resizeTimer, SIGNAL(timeout()), this, SLOT(adjustColumn()));
}

ListView::~ListView()
{
}

void ListView::sizeChange(int,int,int)
{
    QTimer::singleShot(0, this, SLOT(adjustColumn()));
}

bool ListView::getMenu(ListViewItem *item, unsigned long &id, void *&param)
{
    if (m_menuId == 0)
        return false;
    id    = m_menuId;
    param = item;
    return true;
}

void ListView::setMenu(unsigned long menuId)
{
    m_menuId = menuId;
}

void ListView::setOpen(bool /* o */)
{
}

void ListView::setOpen(ListViewItem* /*item*/, bool/* o*/)
{
}

bool ListView::processEvent(Event *e)
{
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdListDelete) && (cmd->menu_id == MenuListView)){
            ListViewItem *item = (ListViewItem*)(cmd->param);
            if (item->listView() == this){
                emit deleteItem(item);
                return true;
            }
        }
    }
    return false;
}

void ListView::keyPressEvent(QKeyEvent *e)
{
    if (e->key()){
        int key = e->key();
        if (e->modifiers() & Qt::ShiftModifier)
            key |= Qt::SHIFT;
        if (e->modifiers() & Qt::ControlModifier)
            key |= Qt::CTRL;
        if (e->modifiers() & Qt::AltModifier)
            key |= Qt::ALT;
        ListViewItem *item = currentItem();
        if (item){
            unsigned long id;
            void *param;
            if (getMenu(item, id, param)){
                EventMenuProcess e(id, param, key);
                if (e.process() && e.menu())
                    return;
            }
        }
    }
    if (e->key() == Qt::Key_F10){
        showPopup(currentItem(), QPoint());
        return;
    }
    QTreeWidget::keyPressEvent(e);
}

ListViewItem* ListView::currentItem()
{
    return static_cast<ListViewItem*>(QTreeWidget::currentItem());
}

void ListView::viewportMousePressEvent(QMouseEvent * /*e*/)
{
    //QTreeWidget::viewportMousePressEvent(e);
}

void ListView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        m_pressedItem = itemAt(e->pos());
        /*
           if (m_pressedItem && !m_pressedItem->isSelectable())
           m_pressedItem = NULL;
           */
        if (m_pressedItem)
        {
            update();
            //repaintItem(m_pressedItem);
            //update(model()->index(m_pressedItem->row(), 0));
        }
    }
    QTreeWidget::mousePressEvent(e);
}

void ListView::mouseMoveEvent(QMouseEvent *e)
{
    QTreeWidget::mouseMoveEvent(e);
}

ListViewItem* ListView::itemAt(const QPoint& p)
{
    return static_cast<ListViewItem*>(QTreeWidget::itemAt(p));
}

void ListView::mouseReleaseEvent(QMouseEvent *e)
{
    QTreeWidget::mouseReleaseEvent(e);
    if (m_pressedItem){
        ListViewItem *item = m_pressedItem;
        m_pressedItem = NULL;
        //update(model()->index(item->row(), 0));
        update();
        ListViewItem *citem = itemAt(e->pos());
        if (item == citem)
            emit clickItem(item);
    }
}

void ListView::viewportContextMenuEvent( QContextMenuEvent *e)
{
    QPoint p = e->globalPos();
    ListViewItem *list_item = itemAt(viewport()->mapFromGlobal(p));
    showPopup(list_item, p);
}

void ListView::showPopup(ListViewItem *item, QPoint p)
{
    unsigned long id;
    void *param;

    if (item == NULL)
        return;

    if (!getMenu(item, id, param))
        return;
    if (p.isNull()){
        QRect rc = visualItemRect(item);
        p = QPoint(rc.x() + rc.width() / 2, rc.y() + rc.height() / 2);
        p = viewport()->mapToGlobal(p);
    }
    EventMenuProcess eMenu(id, param);
    eMenu.process();
    QMenu *menu = eMenu.menu();
    if (menu){
        setCurrentItem(item);
        menu->popup(p);
    }
}

void ListView::contextMenuEvent(QContextMenuEvent* e)
{
    unsigned long id;
    void *param;

    ListViewItem* item = itemAt(e->pos());
    if (item == NULL)
        return;

    if (!getMenu(item, id, param))
        return;
    EventMenuProcess eMenu(id, param);
    eMenu.process();
    QMenu *menu = eMenu.menu();
    if (menu)
    {
        setCurrentItem(item);
        menu->popup(e->globalPos());
    }
}

bool ListView::eventFilter(QObject *o, QEvent *e)
{
    if ((o == verticalScrollBar()) &&
            ((e->type() == QEvent::Show) || (e->type() == QEvent::Hide)))
        adjustColumn();
    return QTreeWidget::eventFilter(o, e);
}

int ListView::expandingColumn() const
{
    return m_expandingColumn;
}

void ListView::setExpandingColumn(int n)
{
    m_expandingColumn = n;
    adjustColumn();
}

void ListView::resizeEvent(QResizeEvent *e)
{
    QTreeWidget::resizeEvent(e);
    adjustColumn();
}

ListViewItem* ListView::firstChild()
{
    return static_cast<ListViewItem*>(topLevelItem(0));
}

void ListView::adjustColumn()
{
#ifdef WIN32
    if (inResize()){
        if (!m_resizeTimer->isActive())
            m_resizeTimer->start(500);
        return;
    }
#endif
    m_resizeTimer->stop();
    if (m_expandingColumn >= 0){
        int w = width();
        QScrollBar *vBar = verticalScrollBar();
        if (vBar->isVisible())
            w -= vBar->width();
        for (int i = 0; i < columnCount(); i++){
            if (i == m_expandingColumn)
                continue;
            w -= columnWidth(i);
        }
        int minW = 40;
        for (int i = 0; i < topLevelItemCount(); i++){
            ListViewItem *item = static_cast<ListViewItem*>(topLevelItem(i));
            QFontMetrics fm(font());
            int ww = fm.width(item->text(m_expandingColumn));
            const QPixmap pict = item->pixmap(m_expandingColumn);
            if (!pict.isNull())
                ww += pict.width() + 2;
            if (ww > minW)
                minW = ww + 8;
        }
        if (w < minW)
            w = minW;
        setColumnWidth(m_expandingColumn, w - 4);
        viewport()->repaint();
    }
}

void ListView::startDrag(Qt::DropActions)
{
    emit dragStart();
    startDrag(dragObject());
}

void ListView::startDrag(QMimeData * /*d*/)
{
    /*
    if (d)
        d->dragCopy();
        */
}

QMimeData *ListView::dragObject()
{
    return NULL;
}

void ListView::acceptDrop(bool bAccept)
{
    m_bAcceptDrop = bAccept;
}

void ListView::dragEnterEvent(QDragEnterEvent *e)
{
    emit dragEnter(e);
    if (m_bAcceptDrop){
        e->accept();
        return;
    }
    e->ignore();
}

void ListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (m_bAcceptDrop){
        e->accept();
        return;
    }
    e->ignore();
}

void ListView::dropEvent(QDropEvent *e)
{
    if (m_bAcceptDrop){
        e->accept();
        emit drop(e);
        return;
    }
    e->ignore();
}

void ListView::addColumn(const QString& name)
{
    /*
    ListViewItem* item = new ListViewItem(this);
    item->setText(name);
    setHorizontalHeaderItem(columnCount(), item);
    */
    setColumnCount(columnCount() + 1);
    headerItem()->setText(columnCount() - 1, name);
}

static char CONTACT_MIME[] = "application/x-contact";

ContactDragObject::ContactDragObject( Contact *contact ) : QMimeData()
{
    QByteArray data;
    m_id = contact->id();
    data.resize(sizeof(m_id));
    memcpy(data.data(), &m_id, sizeof(m_id));
    setData(CONTACT_MIME, data);
}

ContactDragObject::~ContactDragObject()
{
    ListView *view = static_cast<ListView*>(parent());
    if (view && view->m_pressedItem){
        //ListViewItem *item = view->m_pressedItem;
        view->m_pressedItem = NULL;
        //view->update(view->model()->index(item->row(), item->column()));
        view->update();
    }
    Contact *contact = getContacts()->contact(m_id);
    if (contact && (contact->getFlags() & CONTACT_DRAG))
        delete contact;
}

bool ContactDragObject::canDecode(QMimeSource *s)
{
    return (decode(s) != NULL);
}

Contact *ContactDragObject::decode( QMimeSource *s )
{
    if (!s->provides(CONTACT_MIME))
        return NULL;
    QByteArray data = s->encodedData(CONTACT_MIME);
    unsigned long id;
    if( data.size() != sizeof( id ) )
        return NULL;
    memcpy( &id, data.data(), sizeof(id));
    return getContacts()->contact(id);
}

// vim: set expandtab:

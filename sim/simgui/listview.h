/***************************************************************************
                          listview.h  -  description
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

#ifndef _LISTVIEW_H
#define _LISTVIEW_H

#include "simapi.h"
#include "event.h"

#include <QMimeData>
#include <QMimeSource>
#include <QTreeWidget>
#include <QTreeWidgetItem>

const unsigned long MenuListView		= 0x100;
const unsigned long CmdListDelete	= 0x100;

class QTimer;

class ListView;
class EXPORT ListViewItem : public QTreeWidgetItem
{
public:
    ListViewItem();
    ListViewItem(const QString& name);
    ListViewItem(ListView* parent);
    ListViewItem(ListViewItem* parent);
    virtual ~ListViewItem();

    ListView* listView() const;
    void setPixmap(int col, QPixmap p);
    QPixmap pixmap(int t);

    int height() const {return sizeHint(0).height();}
    int width() const {return sizeHint(0).width();};

    void repaint();
    bool isOpen();
    void setOpen(bool o);
    bool isExpandable();
    void setExpandable(bool e);
private:
    bool m_open;
    bool m_expandable;
};

class EXPORT ListView : public QTreeWidget, public SIM::EventReceiver
{
    Q_OBJECT
    Q_PROPERTY( int expandingColumn READ expandingColumn WRITE setExpandingColumn )
public:
    ListView(QWidget *parent);
    virtual ~ListView();
    int expandingColumn() const;
    void setExpandingColumn(int);
    ListViewItem *m_pressedItem;
    void startDrag(QMimeData*);
    void acceptDrop(bool bAccept);
    void setMenu(unsigned long menuId);
    ListViewItem* currentItem();
    ListViewItem* itemAt(const QPoint& p);
    ListViewItem* firstChild();
    void addColumn(const QString& name);
    void setOpen(bool o);
    void setOpen(ListViewItem* item, bool o);

signals:
    void clickItem(ListViewItem*);
    void deleteItem(ListViewItem*);
    void dragStart();
    void dragEnter(QMimeSource*);
    void drop(QMimeSource*);

public slots:
    void adjustColumn();
    virtual void startDrag(Qt::DropActions);
    void sizeChange(int,int,int);

protected:
    virtual bool getMenu(ListViewItem *item, unsigned long &id, void *&param);
    virtual bool processEvent(SIM::Event *e);
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void resizeEvent(QResizeEvent*);
    virtual QMimeData *dragObject();
    void viewportContextMenuEvent( QContextMenuEvent *e);
    void viewportMousePressEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void showPopup(ListViewItem *item, QPoint p);
    void contextMenuEvent(QContextMenuEvent* e);
    int m_expandingColumn;
    unsigned long m_menuId;
    QTimer	 *m_resizeTimer;
    bool m_bAcceptDrop;
    static bool s_bInit;
};

class EXPORT ContactDragObject : public QMimeData
{
    Q_OBJECT
public:
    ContactDragObject( SIM::Contact *contact );
    ~ContactDragObject();
    static bool canDecode(QMimeSource*);
    static SIM::Contact *decode(QMimeSource*);
protected:
    unsigned long m_id;
};

#endif

// vim: set expandtab:

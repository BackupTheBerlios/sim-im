/***************************************************************************
                          userview.h  -  description
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

#ifndef _USERVIEW_H
#define _USERVIEW_H

#include <QTreeView>

class CorePlugin;
class IntLineEdit;
class QTimer;

struct BlinkCount
{
    unsigned long	id;
    unsigned		count;
};

class UserView : public QTreeView
{
    Q_OBJECT
public:
    enum GroupMode
    {
        NoGroups = 0,
        Mode1 = 1,
        Mode2 = 2
    };

    UserView(CorePlugin* plugin, QWidget* parent = 0);
    ~UserView();

    bool init();

    void setShowOffline(bool s);
    bool isShowOffline() const;

    void setGroupMode(GroupMode mode);
    GroupMode groupMode() const;

//    IntLineEdit *m_edtGroup;
//    IntLineEdit *m_edtContact;
//    QTreeWidgetItem *mTipItem;
//    bool	 m_bUnreadBlink;
//public slots:
//    void editGroupEnter();
//    void editContactEnter();
//    void editEscape();
//    void renameGroup();
//    void renameContact();
//    void deleteGroup(void*);
//    void deleteContact(void*);
//    void joinContacts(void*);
//    void cancelJoinContacts(void*);
//    void blink();
//    void unreadBlink();
//    void doDrop();
//    void doClick();
//    void dragScroll();
//protected:
//    virtual unsigned getUnread(unsigned contact_id);
//    virtual bool getMenu(QTreeWidgetItem *item, unsigned long &id, void* &param);
//    virtual bool processEvent(SIM::Event*);
//    bool eventFilter(QObject *obj, QEvent *e);
//    void setGroupMode(unsigned mode, bool bFirst=false);
//    void keyPressEvent(QKeyEvent *e);
//    void mousePressEvent(QMouseEvent *e);
//    void mouseReleaseEvent(QMouseEvent *e);
//    void mouseDoubleClickEvent(QMouseEvent *e);
//    void dragEnterEvent(QDragEnterEvent *e);
//    virtual void dragMoveEvent(QDragMoveEvent *e);
//    void dropEvent(QDropEvent *e);
//    void focusOutEvent(QFocusEvent*);
//    void sortAll();
//    void sortAll(QTreeWidgetItem*);
//    void dragEvent(QDropEvent *e, bool isDrop);
//    void search(std::list<QTreeWidgetItem*> &items);
//    void search(QTreeWidgetItem*, std::list<QTreeWidgetItem*> &items);
//    void stopSearch();
//    virtual QMimeData *mimeData( const QList<QTreeWidgetItem *> items ) const;
//    QMimeData *dragObject();
//    virtual void deleteItem(QTreeWidgetItem *item);
//    std::list<BlinkCount> blinks;
//    std::list<QTreeWidgetItem*> closed_items;
//    unsigned long m_dropContactId;
//    QTreeWidgetItem *m_dropItem;
//    QTreeWidgetItem *m_current;
//    bool	 m_bBlink;
//    QTimer	 *m_blinkTimer;
//    QTimer	 *m_unreadTimer;
//    QString	 m_search;
//    bool	 m_bRemoveHistory;
//    QTreeWidgetItem *m_searchItem;
//    friend class UserViewDelegate;
//	UserWnd *m_userWnd;
private:
    CorePlugin *m_plugin;
    bool m_showOffline;
    GroupMode m_groupMode;
};

#endif


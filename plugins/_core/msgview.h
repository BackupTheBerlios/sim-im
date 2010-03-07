/***************************************************************************
                          msgview.h  -  description
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

#ifndef _MSGVIEW_H
#define _MSGVIEW_H

#include <vector>
#include <list>

#include "simgui/textshow.h"
#include <QMenu>

class CorePlugin;
class XSL;

using namespace std;

struct CutHistory
{
    unsigned    contact;
    QString     client;
    unsigned    from;
    unsigned    size;
};

struct Msg_Id
{
    unsigned    id;
    QString     client;
};

class MsgViewBase : public TextShow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgViewBase(QWidget *parent, const char *name="", unsigned id=(unsigned)(-1));
    ~MsgViewBase();
    void		addMessage(SIM::Message *msg, bool bUnread=false, bool bSync=true);
    bool		findMessage(SIM::Message *msg);
    void		setSelect(const QString &str);
    void		setXSL(XSL*);
    static		QString parseText(const QString &text, bool bIgnoreColors, bool bUseSmiles);
    unsigned	m_id;
    SIM::Message *currentMessage();
protected slots:
    void update();
protected:
    virtual bool        processEvent(SIM::Event*);
    virtual void        contextMenuEvent( QContextMenuEvent *event );
    void setBackground(unsigned start);
    void setSource(const QString&);
    void setSource(const QUrl&);
    void setColors();
    void reload();
    unsigned    messageId(const QString&, QString &client);
    QString messageText(SIM::Message *msg, bool bUnread);
    QPoint m_popupPos;
    QString m_selectStr;
    unsigned m_nSelection;
    XSL *xsl;
    vector<CutHistory>	m_cut;
    list<Msg_Id>		m_updated;
};

class MsgView : public MsgViewBase
{
    Q_OBJECT
public:
    MsgView(QWidget *parent, unsigned id);
    ~MsgView();
protected slots:
    void		init();
protected:
    virtual bool processEvent(SIM::Event*);
};

#endif


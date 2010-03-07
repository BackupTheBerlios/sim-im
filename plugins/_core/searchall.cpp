/***************************************************************************
                          searchall.cpp  -  description
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

#include "searchall.h"
#include "search.h"
#include "log.h"

#include "contacts/client.h"
#include "simgui/intedit.h"

#include <QTimer>
#include <QShowEvent>

using namespace std;
using namespace SIM;

SearchAll::SearchAll(QWidget *parent) : QWidget(parent)
        //: SearchAllBase(parent)
{
	setupUi(this);
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtMail->setValidator(new EMailValidator(edtMail));
}

void SearchAll::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    emit setAdd(false);
}

void SearchAll::wndDestroyed()
{
    QTimer::singleShot(0, this, SLOT(refresh()));
}

void SearchAll::refresh()
{
    vector<ClientWidget> &widgets = static_cast<SearchDialog*>(topLevelWidget())->m_widgets;
    WND_MAP::iterator it;
    for (it = m_searches.begin(); it != m_searches.end(); ){
        vector<ClientWidget>::iterator itw;
        for (itw = widgets.begin(); itw != widgets.end(); ++itw)
            if (it->first == itw->widget)
                break;
        if (itw != widgets.end()){
            ++it;
            continue;
        }
        m_searches.erase(it);
    }
    if (m_searches.empty())
        emit searchDone(this);
}

void SearchAll::search()
{
    if (grpMail->isChecked() && !edtMail->text().isEmpty()){
        if (!makeSearches())
            return;
        emit searchMail(edtMail->text());
    }
    if(grpName->isChecked() && makeSearches())
        emit searchName(edtFirst->text(), edtLast->text(), edtNick->text());
}

void SearchAll::searchStop()
{
    emit sSearchStop();
    for (WND_MAP::iterator it = m_searches.begin(); it != m_searches.end(); ++it){
        disconnect(this, SIGNAL(sSearchStop()), it->first, SLOT(searchStop()));
        disconnect(this, SIGNAL(searchMail(const QString&)), it->first, SLOT(searchMail(const QString&)));
        disconnect(this, SIGNAL(searchName(const QString&, const QString&, const QString&)), it->first, SLOT(searchName(const QString&, const QString&, const QString&)));
        disconnect(it->first, SIGNAL(searchDone(QWidget*)), this, SLOT(slotSearchDone(QWidget*)));
        disconnect(it->first, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(slotSetColumns(const QStringList&, int, QWidget*)));
        disconnect(it->first, SIGNAL(addItem(const QStringList&, QWidget*)), this, SLOT(slotAddItem(const QStringList&, QWidget*)));
    }
}

bool SearchAll::makeSearches()
{
    m_searches.clear();
    m_attrs.clear();
    vector<ClientWidget> &widgets = static_cast<SearchDialog*>(topLevelWidget())->m_widgets;
    for (unsigned i = 0; i < widgets.size(); i++){
        if ((widgets[i].client == NULL) || (widgets[i].client == (Client*)(-1)))
            continue;
        if ((widgets[i].client->protocol()->description()->flags & PROTOCOL_SEARCH) == 0)
            continue;
        connect(this, SIGNAL(sSearchStop()), widgets[i].widget, SLOT(searchStop()));
        connect(this, SIGNAL(searchMail(const QString&)), widgets[i].widget, SLOT(searchMail(const QString&)));
        connect(this, SIGNAL(searchName(const QString&, const QString&, const QString&)), widgets[i].widget, SLOT(searchName(const QString&, const QString&, const QString&)));
        connect(widgets[i].widget, SIGNAL(searchDone(QWidget*)), this, SLOT(slotSearchDone(QWidget*)));
        connect(widgets[i].widget, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(slotSetColumns(const QStringList&, int, QWidget*)));
        connect(widgets[i].widget, SIGNAL(addItem(const QStringList&, QWidget*)), this, SLOT(slotAddItem(const QStringList&, QWidget*)));
        QStringList l;
        m_searches.insert(WND_MAP::value_type(widgets[i].widget, l));
    }
    return m_searches.size() > 0;
}

void SearchAll::slotSearchDone(QWidget *w)
{
    WND_MAP::iterator it = m_searches.find(w);
    if (it == m_searches.end())
        return;
    m_searches.erase(it);
    disconnect(this, SIGNAL(sSearchStop()), w, SLOT(searchStop()));
    disconnect(this, SIGNAL(searchMail(const QString&)), w, SLOT(searchMail(const QString&)));
    disconnect(this, SIGNAL(searchName(const QString&, const QString&, const QString&)), w, SLOT(searchName(const QString&, const QString&, const QString&)));
    disconnect(w, SIGNAL(searchDone(QWidget*)), this, SLOT(slotSearchDone(QWidget*)));
    disconnect(w, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(slotSetColumns(const QStringList&, int, QWidget*)));
    disconnect(w, SIGNAL(addItem(const QStringList&, QWidget*)), this, SLOT(slotAddItem(const QStringList&, QWidget*)));
    if (m_searches.empty())
        emit searchDone(this);
}

void SearchAll::slotSetColumns(const QStringList &attrs, int, QWidget *w)
{
    WND_MAP::iterator it = m_searches.find(w);
    if (it == m_searches.end())
        return;
    int i;
    for (i = 0; i < attrs.count() / 2; i++)
        it->second.append(attrs[2 * i]);
    QStringList newAttrs;
    for (i = 0; i < attrs.count() / 2; i++){
        QString attr = attrs[i * 2];
        int n;
        for (n = 0; n < m_attrs.count(); n++)
            if (m_attrs[n] == attr)
                break;
        if (n < m_attrs.count())
            continue;
        m_attrs.append(attr);
        newAttrs.append(attr);
        newAttrs.append(attrs[i * 2 + 1]);
    }
    if (newAttrs.count() == 0)
        return;
    emit setColumns(newAttrs, 0, this);
}

void SearchAll::slotAddItem(const QStringList &attrs, QWidget *w)
{
    WND_MAP::iterator it = m_searches.find(w);
    if (it == m_searches.end())
        return;
    QStringList la;
    la.append(attrs[0]);
    la.append(attrs[1]);
    for (int i = 0; i < m_attrs.count(); i++){
        QString attr = m_attrs[i];
        QString v;
        for (int n = 0; n < it->second.count(); n++){
            if (it->second[n] == attr){
                v = attrs[n + 2];
                break;
            }
        }
        la.append(v);
    }
    emit addItem(la, w);
}

/*
#ifndef NO_MOC_INCLUDES
#include "searchall.moc"
#endif
*/


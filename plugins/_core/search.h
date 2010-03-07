/***************************************************************************
                          search.h  -  description
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

#ifndef _SEARCH_H
#define _SEARCH_H

#include <vector>
#include "contacts.h"
#include "ui_searchbase.h"

#include <QMainWindow>
#include <QResizeEvent>
#include <QMoveEvent>
#include <QCloseEvent>

class CorePlugin;
class ListView;
class SearchBase;
class QStatusBar;
class QTimer;

struct ClientWidget
{
    SIM::Client	*client;
    QWidget		*widget;
    QString		name;
};

class SearchWidget : public QWidget, public Ui::Search
{
	Q_OBJECT
public:
	SearchWidget(QWidget* parent);
};

class SearchDialog : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    SearchDialog();
    ~SearchDialog();
public slots:
    void setAdd(bool bAdd);
    void clientActivated(int);
    void aboutToShow(QWidget*);
    void resultShow(QWidget*);
    void resultDestroyed();
    void textChanged(const QString &txt = QString::null);
    void toggled(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
    void addSearch(QWidget*, SIM::Client*, const QString &name);
    void showClient(SIM::Client*);
signals:
    void finished();
    void search();
    void searchStop();
    void createContact(const QString&, unsigned tmpFlags, SIM::Contact *&contact);
    void createContact(unsigned tmpFlags, SIM::Contact *&contact);
protected slots:
    void searchClick();
    void setColumns(const QStringList&, int, QWidget*);
    void addItem(const QStringList&, QWidget *search);
    void searchDone(QWidget*);
    void update();
    void addClick();
    void optionsClick();
    void selectionChanged();
    void dragStart();
    void newSearch();
    void enableOptions(bool);
protected:
    std::vector<ClientWidget>	m_widgets;
    void		setStatus();
    void		setAddButton();
    ListView	*m_result;
    QWidget		*m_current;
    QWidget		*m_currentResult;
    QWidget		*m_active;
    virtual bool processEvent(SIM::Event*);
    void		resizeEvent(QResizeEvent*);
    void		moveEvent(QMoveEvent*);
    void		closeEvent(QCloseEvent*);
    void		fillClients();
    void		attach(QWidget*);
    void		detach(QWidget*);
    bool		checkSearch(QWidget*, bool&);
    SIM::Contact *createContact(unsigned flags);
    void		setTitle();
    bool		m_bAdd;
    bool		m_bColumns;
    unsigned	m_id;
    unsigned	m_result_id;
    SearchWidget *m_search;
    QStatusBar	*m_status;
    QTimer		*m_update;
    friend class SearchAll;
};

#endif


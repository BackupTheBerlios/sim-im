/***************************************************************************
                          mainwin.cpp  -  description
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

#include "simapi.h"

#include "icons.h"
#include "mainwin.h"
#include "core.h"
#include "roster/userview.h"
#include "contacts/contact.h"
#include "simgui/toolbtn.h"
#include "events/eventhub.h"
#include "commands/commandhub.h"
#include "commands/uicommand.h"
#include "imagestorage/imagestorage.h"

#include <QApplication>
#include <QPixmap>
#include <QLayout>
#include <QTimer>
#include <QSizeGrip>
#include <QStatusBar>
#include <QDesktopWidget>
#include "profilemanager.h"

using namespace SIM;

MainWindow::MainWindow(CorePlugin* core)
    : QMainWindow(NULL, Qt::Window)
    , EventReceiver(LowestPriority),
    m_core(core)
{
    log(L_DEBUG, "MainWindow::MainWindow()");
    setObjectName("mainwnd");
    setAttribute(Qt::WA_AlwaysShowToolTips);
    h_lay	 = NULL;
    m_bNoResize = false;

    m_icon = "SIM";
    setWindowIcon(getImageStorage()->icon(m_icon));
    setTitle();

    m_bar = new SIM::ToolBar("Main toolbar", this);

    main = new QWidget(this);
    setCentralWidget(main);

    lay = new QVBoxLayout(main);
    lay->setMargin(0);

    QStatusBar *status = statusBar();
    status->show();
    status->installEventFilter(this);

	m_view = new UserView;
	m_view->init();

    getEventHub()->getEvent("init")->connectTo(this, SLOT(eventInit()));
}

MainWindow::~MainWindow()
{
	delete m_view;
    delete m_bar;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (m_bNoResize)
        return;
    QMainWindow::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ChildRemoved){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        std::list<QWidget*>::iterator it;
        for (it = statusWidgets.begin(); it != statusWidgets.end(); ++it){
            if (*it == ce->child()){
                statusWidgets.erase(it);
                break;
            }
        }
        if(statusWidgets.size() == 0)
        {
            statusBar()->hide();
        }
    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::loadDefaultCommandList()
{
    log(L_DEBUG, "loadDefaultCommandList");
    m_bar->addUiCommand(getCommandHub()->command("show_offline"));
    m_bar->addUiCommand(getCommandHub()->command("groupmode_menu"));
    m_bar->addSeparator();
}

void MainWindow::populateMainToolbar()
{
    QStringList actions = m_core->propertyHub()->value("mainwindow_toolbar_actions").toStringList();
    if(actions.isEmpty()) {
        loadDefaultCommandList();
    }
    else {
        m_bar->loadCommandList(actions);
    }
}

void MainWindow::eventInit()
{
    log(L_DEBUG, "MainWindow::eventInit()");
    setTitle();
    populateMainToolbar();
    this->addToolBar(m_bar);
    raiseWindow(this);
}

bool MainWindow::processEvent(Event *e)
{
	switch(e->type()){
		case eEventSetMainIcon:
			{
				EventSetMainIcon *smi = static_cast<EventSetMainIcon*>(e);
				m_icon = smi->icon();
				setWindowIcon(Icon(m_icon));
				break;
			}
		case eEventCommandExec:
			{
				EventCommandExec *ece = static_cast<EventCommandExec*>(e);
				CommandDef *cmd = ece->cmd();
				if (cmd->id == CmdQuit)
					quit();
				break;
			}
		case eEventAddWidget:
			{
				EventAddWidget *aw = static_cast<EventAddWidget*>(e);
				switch(aw->place()) {
					case EventAddWidget::eMainWindow:
						addWidget(aw->widget(), aw->down());
						break;
					case EventAddWidget::eStatusWindow:
						addStatus(aw->widget(), aw->down());
						break;
					default:
						return false;
				}
				return true;
			}
		case eEventIconChanged:
			setWindowIcon(Icon(m_icon));
			break;
		case eEventContact:
			{
				EventContact *ec = static_cast<EventContact*>(e);
				Contact *contact = ec->contact();
				if (contact == getContacts()->owner())
					setTitle();
				break;
			}
		default:
			break;
	}
	return false;
}

void MainWindow::quit()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	CorePlugin::instance()->prepareConfig();
    save_state();
    m_core->propertyHub()->setValue("mainwindow_toolbar_actions", m_bar->saveCommandList());
    QMainWindow::closeEvent(e);
    qApp->quit();
}

void MainWindow::addWidget(QWidget *w, bool bDown)
{
    w->setParent(main);
    w->move(QPoint());
    if (bDown){
        lay->addWidget(w);
    }else{
        lay->insertWidget(0, w);
    }
    if (isVisible())
        w->show();
}

void MainWindow::addStatus(QWidget *w, bool)
{
    QStatusBar *status = statusBar();
    w->setParent(status);
    w->move(QPoint());
    statusWidgets.push_back(w);
    status->addWidget(w, true);
    w->show();
    status->setSizeGripEnabled(true);
    status->show();
}

void MainWindow::setTitle()
{
    QString title;
    Contact *owner = getContacts()->owner();
    if (owner)
        title = owner->getName();
    if (title.isEmpty())
        title = "SIM";
    setWindowTitle(title);
}

void MainWindow::focusInEvent(QFocusEvent *e)
{
    QMainWindow::focusInEvent(e);
	m_view->setFocus();
}



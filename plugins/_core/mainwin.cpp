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

#include "mainwin.h"
#include "core.h"
#include "roster/userview.h"
#include "contacts/contact.h"
#include "contacts/contactlist.h"
#include "events/eventhub.h"
#include "commands/commandhub.h"
#include "commands/uicommand.h"
#include "imagestorage/imagestorage.h"
#include "clientmanager.h"

#include <QApplication>
#include <QPixmap>
#include <QLayout>
#include <QTimer>
#include <QSizeGrip>
#include <QStatusBar>
#include <QDesktopWidget>
#include "profilemanager.h"

#include "log.h"

using namespace SIM;

MainWindow::MainWindow(CorePlugin* core)
    : QMainWindow(NULL, Qt::Window)
    , m_core(core)
    , m_noresize(false)
{
    log(L_DEBUG, "MainWindow::MainWindow()");
    setAttribute(Qt::WA_AlwaysShowToolTips);

    setWindowIcon(getImageStorage()->icon("SIM"));
    updateTitle();

    m_bar = new SIM::ToolBar("Main toolbar", this);
    addToolBar(m_bar);

    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    m_layout = new QVBoxLayout(m_centralWidget);
    m_layout->setMargin(0);

    statusBar()->show();
    statusBar()->setSizeGripEnabled(false);
    statusBar()->installEventFilter(this);

    m_view = new UserView(core);
    m_view->init();
    addWidget(m_view);

}

MainWindow::~MainWindow()
{
    delete m_view;
    delete m_bar;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (m_noresize)
        return;
    QMainWindow::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
//    if (e->type() == QEvent::ChildRemoved){
//        QChildEvent *ce = static_cast<QChildEvent*>(e);
//        std::list<QWidget*>::iterator it;
//        for (it = statusWidgets.begin(); it != statusWidgets.end(); ++it){
//            if (*it == ce->child()){
//                statusWidgets.erase(it);
//                break;
//            }
//        }
//        if(statusWidgets.size() == 0)
//        {
//            statusBar()->hide();
//        }
//    }
    return QMainWindow::eventFilter(o, e);
}

void MainWindow::loadDefaultCommandList()
{
    log(L_DEBUG, "loadDefaultCommandList");
    m_bar->addUiCommand(getCommandHub()->command("show_offline"));
    m_bar->addUiCommand(getCommandHub()->command("groupmode_menu"));
    m_bar->addSeparator();
    m_bar->addUiCommand(getCommandHub()->command("common_status"));
    m_bar->addSeparator();
    m_bar->addUiCommand(getCommandHub()->command("main_menu"));
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

void MainWindow::init()
{
    log(L_DEBUG, "MainWindow::init()");
    updateTitle();
    populateMainToolbar();
    refreshStatusWidgets();
}

//bool MainWindow::processEvent(Event *e)
//{
//	switch(e->type()){
//		case eEventSetMainIcon:
//			{
//				EventSetMainIcon *smi = static_cast<EventSetMainIcon*>(e);
//				m_icon = smi->icon();
//				setWindowIcon(Icon(m_icon));
//				break;
//			}
//		case eEventCommandExec:
//			{
//				EventCommandExec *ece = static_cast<EventCommandExec*>(e);
//				CommandDef *cmd = ece->cmd();
//				if (cmd->id == CmdQuit)
//					quit();
//				break;
//			}
//		case eEventAddWidget:
//			{
//				EventAddWidget *aw = static_cast<EventAddWidget*>(e);
//				switch(aw->place()) {
//					case EventAddWidget::eMainWindow:
//						addWidget(aw->widget(), aw->down());
//						break;
//					case EventAddWidget::eStatusWindow:
//						addStatus(aw->widget(), aw->down());
//						break;
//					default:
//						return false;
//				}
//				return true;
//			}
//		case eEventIconChanged:
//			setWindowIcon(Icon(m_icon));
//			break;
//		case eEventContact:
//			{
//				EventContact *ec = static_cast<EventContact*>(e);
//				Contact *contact = ec->contact();
//				if (contact == getContacts()->owner())
//					setTitle();
//				break;
//			}
//		default:
//			break;
//	}
//	return false;
//}

//void MainWindow::quit()
//{
//    close();
//}

//void MainWindow::closeEvent(QCloseEvent *e)
//{
//	CorePlugin::instance()->prepareConfig();
//    save_state();
//    m_core->propertyHub()->setValue("mainwindow_toolbar_actions", m_bar->saveCommandList());
//    QMainWindow::closeEvent(e);
//    qApp->quit();
//}

void MainWindow::addWidget(QWidget *w)
{
    w->setParent(m_centralWidget);
    w->move(QPoint());
    m_layout->addWidget(w);
    if(isVisible())
        w->show();
}

void MainWindow::refreshStatusWidgets()
{
    qDeleteAll(m_statusWidgets);
    m_statusWidgets.clear();
    QList<ClientPtr> clients = getClientManager()->allClients();
    foreach(const ClientPtr& client, clients)
    {
        QWidget* statusWidget = client->createStatusWidget();
        if(statusWidget)
        {
            m_statusWidgets.append(statusWidget);
            statusWidget->setParent(statusBar());
            statusBar()->addWidget(statusWidget);
            statusWidget->show();
        }
    }
}

//void MainWindow::addStatus(QWidget *w, bool)
//{
//    QStatusBar *status = statusBar();
//    w->setParent(status);
//    w->move(QPoint());
//    statusWidgets.push_back(w);
//    status->addWidget(w, true);
//    w->show();
//    status->setSizeGripEnabled(true);
//    status->show();
//}

void MainWindow::updateTitle()
{
    QString title;
    ContactPtr owner = getContactList()->ownerContact();
    if (owner)
        title = owner->name();
    if (title.isEmpty())
        title = "SIM";
    setWindowTitle(title);
}

//void MainWindow::focusInEvent(QFocusEvent *e)
//{
//    QMainWindow::focusInEvent(e);
//    m_view->setFocus();
//}

UserView* MainWindow::userview() const
{
    return m_view;
}



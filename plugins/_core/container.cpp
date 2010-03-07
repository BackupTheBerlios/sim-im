/***************************************************************************
                          container.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "log.h"

#include "container.h"
#include "userwnd.h"
#include "core.h"
#include "buffer.h"
#include "icons.h"
#include "contacts/contact.h"
#include "contacts/client.h"
#include "simgui/toolbtn.h"

#include <QSplitter>
#include <QLayout>
#include <QStatusBar>
#include <QProgressBar>
#include <QStackedWidget>
#include <QTimer>
#include <QToolBar>
#include <QMenu>
#include <QApplication>
#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QByteArray>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QMoveEvent>
#include <QVariant>
#include <QColor>

using namespace std;
using namespace SIM;

const unsigned ACCEL_MESSAGE = 0x1000;


class Splitter : public QSplitter
{
public:
    Splitter(QWidget *p) : QSplitter(Qt::Vertical, p) {}
protected:
    virtual QSizePolicy sizePolicy() const;
};

//FIXME: Obsolete?
//static void copyData(SIM::Data *dest, const SIM::Data *src, unsigned count)
//{
//    for(unsigned i = 0; i < count; i++)
//        dest[i] = src[i];
//}

ContainerStatus::ContainerStatus(QWidget *parent)
    : QStatusBar(parent)
{
    QSize s;
    {
        QProgressBar p(this);
        addWidget(&p);
        s = minimumSizeHint();
    }
    setMinimumSize(QSize(0, s.height()));
}

void ContainerStatus::resizeEvent(QResizeEvent *e)
{
    QStatusBar::resizeEvent(e);
    emit sizeChanged(width());
}

QSizePolicy Splitter::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

static DataDef containerData[] =
    {
        { "Id"          , DATA_ULONG,   1, 0 },
        { "Windows"     , DATA_STRING,  1, 0 },
        { "ActiveWindow", DATA_ULONG,   1, 0 },
        { "Geometry"    , DATA_LONG,    5, 0 },
        { "BarState"    , DATA_LONG,    7, 0 },
        { "StatusSize"  , DATA_ULONG,   1, 0 },
        { "WndConfig"   , DATA_STRLIST, 1, 0 },
        { NULL          , DATA_UNKNOWN, 0, 0 }
    };

Container::Container(unsigned id, const char *cfg) 
    : QMainWindow()
    , m_bNoRead     (false)
    , m_bInit       (false)
    , m_bInSize     (false)
    , m_bStatusSize (false)
    , m_bBarChanged (false)
    , m_bReceived   (false)
    , m_bNoSwitch   (false)
    , m_avatar_window(this)
    , m_avatar_label(&m_avatar_window)
    , m_tabBar      (NULL)
    , m_wnds        (NULL)
{
    

    m_avatar_window.setWidget(&m_avatar_label);
    //m_avatar_window.setOrientation(Qt::Vertical);
    m_avatar_window.setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_avatar_window.setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    addDockWidget(Qt::LeftDockWidgetArea, &m_avatar_window);
    setAttribute(Qt::WA_DeleteOnClose, true);

    setIconSize(QSize(16,16));

    if(cfg && *cfg)
    {
        Buffer config;
        config << "[Title]\n" << cfg;
        config.setWritePos(0);
        config.getSection();
        load_data(containerData, &data, &config);
    }
    else
        load_data(containerData, &data, NULL);


	if (cfg != NULL)
		return;

	setId(id);
	setContainerGeometry();
}

void Container::setContainerGeometry()
{
	
	//copyData(data.barState, CorePlugin::instance()->data.ContainerBar, 7);
	//copyData(data.geometry, CorePlugin::instance()->data.ContainerGeometry, 5);
	if(data.geometry[WIDTH].toLong() == -1 || data.geometry[HEIGHT].toLong() == -1)
	{
		QWidget *desktop = QApplication::desktop();
		data.geometry[WIDTH].asLong() = desktop->width() / 3;
		data.geometry[HEIGHT].asLong() = desktop->height() / 3;
	}
	if(data.geometry[TOP].toLong() != -1 || data.geometry[LEFT].toLong() != -1)
	{
		QWidgetList list = QApplication::topLevelWidgets();
		for(int i = 0; i < 2; i++)
		{
			QWidget *w;
			bool bOK = true;
			foreach(w,list)
			{
				if(w == this)
					continue;
				if(w->inherits("Container"))
				{
					int dw = w->pos().x() - data.geometry[LEFT].toLong();
					int dh = w->pos().y() - data.geometry[TOP].toLong();
					if (dw < 0)
						dw = -dw;
					if (dh < 0)
						dh = -dh;
					if (dw < 3 && dh < 3)
					{
						long nl = data.geometry[LEFT].toLong();
						long nt = data.geometry[TOP].toLong();
						nl += 21;
						nt += 20;
						QWidget *desktop = QApplication::desktop();
						if (nl + data.geometry[WIDTH].toLong() > desktop->width())
							nl = 0;
						if (nt + data.geometry[WIDTH].toLong() > desktop->width())
							nt = 0;
						if (nl != data.geometry[LEFT].toLong() && nt != data.geometry[TOP].toLong())
						{
							data.geometry[LEFT].asLong() = nl;
							data.geometry[TOP].asLong()  = nt;
							bOK = false;
						}
					}
				}
			}
			if (bOK)
				break;
		}
	}
	setStatusSize(CorePlugin::instance()->value("ContainerStatusSize").toUInt());
	/*
	m_bInSize = true;
	::restoreGeometry(this, data.geometry, bPos, true);
	m_bInSize = false;
	*/
}
Container::~Container()
{
    if( NULL != m_tabBar )
    {
        list<UserWnd*> wnds = m_tabBar->windows();
        list<UserWnd*>::iterator it;
        for (it = wnds.begin(); it != wnds.end(); ++it)
            disconnect(*it, SIGNAL(closed(UserWnd*)), this, SLOT(removeUserWnd(UserWnd*)));
    }
    list<UserWnd*>::iterator it;
    for (it = m_childs.begin(); it != m_childs.end(); ++it)
        delete (*it);
    free_data(containerData, &data);
}

void Container::init()
{
    if (m_bInit)
        return;

    frm = new QFrame(this);
    setCentralWidget(frm);

    QObject::connect(CorePlugin::instance(), SIGNAL(modeChanged()), this, SLOT(modeChanged()));

    lay = new QVBoxLayout(frm);
    m_wnds = new QStackedWidget(frm);
    m_wnds->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay->addWidget(m_wnds);

    m_tabSplitter = new Splitter(frm);
    m_tabSplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_tabBar = new UserTabBar(m_tabSplitter);
    m_tabBar->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
    m_tabBar->hide();

    m_bInit = true;

    m_status = new ContainerStatus(m_tabSplitter);
    lay->addWidget(m_tabSplitter);
    connect(m_tabBar, SIGNAL(selected(int)), this, SLOT(contactSelected(int)));
    //connect(this, SIGNAL(toolBarPositionChanged(QToolBar*)), this, SLOT(toolbarChanged(QToolBar*)));
    connect(m_status, SIGNAL(sizeChanged(int)), this, SLOT(statusChanged(int)));
    setupAccel();
    showBar();
	setStatusBar(m_status);

    for (list<UserWnd*>::iterator it = m_childs.begin(); it != m_childs.end(); ++it)
        addUserWnd((*it), false);

    m_childs.clear();

    QStringList windows = getWindows().split(',');
    Q_FOREACH(const QString &win, windows) 
	{
        unsigned long id = win.toULong();
        Contact *contact = getContacts()->contact(id);
        if (contact == NULL)
            continue;
        Buffer config;
        QString cfg = getWndConfig(id);
        if (!cfg.isEmpty())
		{
            config << "[Title]\n" << (const char*)cfg.toLocal8Bit();
            config.setWritePos(0);
            config.getSection();
        }
        addUserWnd(new UserWnd(id, &config, false, true), true);
    }

    if (m_tabBar->count() == 0)
        QTimer::singleShot(0, this, SLOT(close()));
    setWindows(QString::null);
    clearWndConfig();
    m_tabBar->raiseTab(getActiveWindow());
    show();
}

QShortcut* Container::makeShortcut(unsigned int key, unsigned int id)
{
	QShortcut* shortcut = new QShortcut(QKeySequence(key), this);
	shortcut->setProperty("id", id);
    connect(shortcut, SIGNAL(activated()), this, SLOT(accelActivated()));
	return shortcut;
}

void Container::setupAccel()
{
	m_shortcuts.clear();
    m_shortcuts.append(makeShortcut(Qt::Key_1 + Qt::ALT, 1));
    m_shortcuts.append(makeShortcut(Qt::Key_2 + Qt::ALT, 2));
    m_shortcuts.append(makeShortcut(Qt::Key_3 + Qt::ALT, 3));
    m_shortcuts.append(makeShortcut(Qt::Key_4 + Qt::ALT, 4));
    m_shortcuts.append(makeShortcut(Qt::Key_5 + Qt::ALT, 5));
    m_shortcuts.append(makeShortcut(Qt::Key_6 + Qt::ALT, 6));
    m_shortcuts.append(makeShortcut(Qt::Key_7 + Qt::ALT, 7));
    m_shortcuts.append(makeShortcut(Qt::Key_8 + Qt::ALT, 8));
    m_shortcuts.append(makeShortcut(Qt::Key_9 + Qt::ALT, 9));
    m_shortcuts.append(makeShortcut(Qt::Key_0 + Qt::ALT, 10));
    m_shortcuts.append(makeShortcut(Qt::Key_Left + Qt::ALT, 11));
    m_shortcuts.append(makeShortcut(Qt::Key_Right + Qt::ALT, 12));
    m_shortcuts.append(makeShortcut(Qt::Key_Home + Qt::ALT, 13));
    m_shortcuts.append(makeShortcut(Qt::Key_End + Qt::ALT, 14));

    EventMenuGetDef eMenu(MenuMessage);
    eMenu.process();
    CommandsDef *cmdsMsg = eMenu.defs();
    CommandsList it(*cmdsMsg, true);
    CommandDef *c;
    while ((c = ++it) != NULL)
	{
        if (c->accel.isEmpty())
            continue;
        m_shortcuts.append(makeShortcut(QKeySequence::fromString(c->accel), ACCEL_MESSAGE + c->id));
    }
}

void Container::setNoSwitch(bool bState)
{
    m_bNoSwitch = bState;
}

list<UserWnd*> Container::windows()
{
    return m_tabBar->windows();
}

QByteArray Container::getState()
{
    clearWndConfig();
    QString windows;
    if (m_tabBar == NULL)
        return save_data(containerData, &data);
    list<UserWnd*> userWnds = m_tabBar->windows();
    for(list<UserWnd*>::iterator it = userWnds.begin(); it != userWnds.end(); ++it)
	{
        if(!windows.isEmpty())
            windows += ',';
        windows += QString::number((*it)->id());
        setWndConfig((*it)->id(), (*it)->getConfig());
    }
    setWindows(windows);
    UserWnd *userWnd = m_tabBar->currentWnd();
    if(userWnd)
        setActiveWindow(userWnd->id());
    ::saveGeometry(this, data.geometry);
    saveToolbar(m_bar, data.barState);
    if(m_tabBar->isVisible())
        setStatusSize(m_status->width());
    return save_data(containerData, &data);
}

QString Container::name()
{
    UserWnd *wnd = m_tabBar ? m_tabBar->currentWnd() : 0;
    if (wnd)
        return wnd->getName();
    return i18n("Container");
}

Q_DECLARE_METATYPE( UserWnd* )

void Container::addUserWnd(UserWnd *wnd, bool bRaise)
{
    if(m_wnds == NULL)
	{
        m_childs.push_back(wnd);
        if(m_childs.size() == 1)
		{
            setWindowIcon(Icon(wnd->getIcon()));
            setWindowTitle(wnd->getLongName());
        }
        return;
    }
    connect(wnd, SIGNAL(closed(UserWnd*)), this, SLOT(removeUserWnd(UserWnd*)));
    connect(wnd, SIGNAL(statusChanged(UserWnd*)), this, SLOT(statusChanged(UserWnd*)));
    m_wnds->addWidget(wnd);
//    m_tabSplitter->addWidget(m_wnds);
    bool bHighlight = false;
    for(list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it)
	{
        if(it->contact == wnd->id())
		{
            bHighlight = true;
            break;
        }
    }

    int tab = m_tabBar->addTab(Icon(wnd->getIcon()),wnd->getName());
    m_tabBar->setTabData(tab,QVariant::fromValue(wnd));
    if (bRaise)
        m_tabBar->setCurrentIndex(tab);
    else
        m_tabBar->repaint();
    contactSelected(0);
    if ((m_tabBar->count() > 1) && !m_tabBar->isVisible())
	{
        m_tabBar->show();
        if (getStatusSize()){
            QList<int> s;
            s.append(1);
            s.append(getStatusSize());
            m_bStatusSize = true;
            m_tabSplitter->setSizes(s);
            m_bStatusSize = false;
        }
        m_tabSplitter->setStretchFactor(m_tabSplitter->indexOf(m_status), 0);
    }
}

void Container::raiseUserWnd(int id/*UserWnd *wnd*/)
{
    if (m_tabBar == NULL)
        return;
    m_tabBar->raiseTab(id);
    contactSelected(0);
}

void Container::removeUserWnd(UserWnd *wnd)
{
    disconnect(wnd, SIGNAL(closed(UserWnd*)), this, SLOT(removeUserWnd(UserWnd*)));
    disconnect(wnd, SIGNAL(statusChanged(UserWnd*)), this, SLOT(statusChanged(UserWnd*)));
    m_wnds->removeWidget(wnd);
    m_tabBar->removeTab(wnd->id());
    if (m_tabBar->count() == 0)
        QTimer::singleShot(0, this, SLOT(close()));
    if (m_tabBar->count() == 1)
        m_tabBar->hide();
    contactSelected(0);
}

UserWnd *Container::wnd(unsigned id)
{
    if (m_tabBar == NULL){
        for (list<UserWnd*>::iterator it = m_childs.begin(); it != m_childs.end(); ++it){
            if ((*it)->id() == id)
                return (*it);
        }
        return NULL;
    }
    return m_tabBar->wnd(id);
}

UserWnd *Container::wnd()
{
    if(m_tabBar == NULL)
	{
        if (m_childs.empty())
            return NULL;
        return m_childs.front();
    }
    return m_tabBar->currentWnd();
}

void Container::showBar()
{
    EventToolbar e(ToolBarContainer, this);
    e.process();
    m_bar = e.toolBar();
    m_bBarChanged = true;
    restoreToolbar(m_bar, data.barState);
    m_bar->show();
	addToolBar(m_bar);
    m_bBarChanged = false;
    contactSelected(0);
    //m_avatar_window.area()->moveDockWindow(&m_avatar_window, 0);
}

void Container::contactSelected(int)
{
    UserWnd *userWnd = m_tabBar ? m_tabBar->currentWnd() : 0;
    if (userWnd == NULL)
        return;
    m_wnds->setCurrentWidget(userWnd);
    userWnd->setFocus();
    m_bar->setParam((void*)userWnd->id());
    Command cmd;
    cmd->id = CmdContainerContact;
    cmd->text_wrk = userWnd->getName();
    cmd->icon  = userWnd->getIcon();
    cmd->param = (void*)(userWnd->id());
    cmd->popup_id = MenuContainerContact;
    cmd->flags = BTN_PICT;
    EventCommandChange e(cmd);
    e.setNoProcess();
    m_bar->processEvent(&e);
    setMessageType(userWnd->type());
    setWindowIcon(Icon(cmd->icon));
    setWindowTitle(userWnd->getLongName());
    m_bar->checkState();
    m_status->showMessage(userWnd->status());
    if (isActiveWindow())
        userWnd->markAsRead();

    if(CorePlugin::instance()->value("ShowAvatarInContainer").toBool())
    {
        Client *client = NULL;
        unsigned j=0;
        QImage img;
        while (j < getContacts()->nClients())
        {
            client = getContacts()->getClient(j++);
            img = client->userPicture(userWnd->id());
            if (!img.isNull())
                break;
        }

        if(!img.isNull())
        {
            m_avatar_label.setPixmap(QPixmap::fromImage(img));
            if (!m_avatar_label.isVisible())
                m_avatar_window.show();
        }
        else
        {
            m_avatar_label.clear();
            m_avatar_window.hide();
        }
    }
    else
    {
        m_avatar_label.clear();
        m_avatar_window.hide();
    }
}

void Container::setMessageType(unsigned type)
{
    CommandDef *def;
    def = CorePlugin::instance()->messageTypes.find(type);
    if (def == NULL)
        return;
    Command cmd;
    cmd->id			 = CmdMessageType;
    cmd->text		 = def->text;
    cmd->icon		 = def->icon;
    cmd->bar_id		 = ToolBarContainer;
    cmd->bar_grp	 = 0x2000;
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->popup_id	 = MenuMessage;
    cmd->flags		 = BTN_PICT;
    EventCommandChange eCmd(cmd);
    eCmd.setNoProcess();
    m_bar->processEvent(&eCmd);
}

void Container::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (m_bInSize)
        return;
    ::saveGeometry(this, data.geometry);
    //CorePlugin::instance()->data.ContainerGeometry[WIDTH]  = data.geometry[WIDTH];
    //CorePlugin::instance()->data.ContainerGeometry[HEIGHT] = data.geometry[HEIGHT];
}

void Container::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
    if (m_bInSize)
        return;
    ::saveGeometry(this, data.geometry);
    //CorePlugin::instance()->data.ContainerGeometry[LEFT] = data.geometry[LEFT];
    //CorePlugin::instance()->data.ContainerGeometry[TOP]  = data.geometry[TOP];
}

void Container::toolbarChanged(QToolBar*)
{
    if (m_bBarChanged)
        return;
    saveToolbar(m_bar, data.barState);
    //copyData(CorePlugin::instance()->data.ContainerBar, data.barState, 7);
}

void Container::statusChanged(int width)
{
    if (m_tabBar->isVisible() && !m_bStatusSize){
        setStatusSize(width);
        CorePlugin::instance()->setValue("ContainerStatusSize", width);
    }
}

void Container::statusChanged(UserWnd *wnd)
{
    if (wnd == m_tabBar->currentWnd())
        m_status->showMessage(wnd->status());
}

void Container::accelActivated()
{
	QShortcut* sender = dynamic_cast<QShortcut*>(QObject::sender());
	unsigned int id = 0;
	if(sender)
		id = sender->property("id").toUInt();
    if ((unsigned)id >= ACCEL_MESSAGE){
        Command cmd;
        cmd->id      = id - ACCEL_MESSAGE;
        cmd->menu_id = MenuMessage;
        cmd->param   = (void*)(m_tabBar->currentWnd()->id());
        EventCommandExec(cmd).process();
        return;
    }
    switch (id){
    case 11:
		if (m_tabBar->current() == 0)
			m_tabBar->setCurrent(m_tabBar->count() - 1);
		else
		    m_tabBar->setCurrent(m_tabBar->current() - 1);
        break;
    case 12:
		if (m_tabBar->current() == (unsigned)m_tabBar->count() - 1)
			m_tabBar->setCurrent(0);
		else 
			m_tabBar->setCurrent(m_tabBar->current() + 1);
        break;
    case 13:
        m_tabBar->setCurrent(0);
        break;
    case 14:
        m_tabBar->setCurrent(m_tabBar->count() - 1);
        break;
    default:
        m_tabBar->setCurrent(id - 1);
    }
}

static const char *accels[] =
    {
        "Alt+1",
        "Alt+2",
        "Alt+3",
        "Alt+4",
        "Alt+5",
        "Alt+6",
        "Alt+7",
        "Alt+8",
        "Alt+9",
        "Alt+0"
    };

#if 0
i18n("male", "%1 is typing")
i18n("female", "%1 is typing")
#endif

void Container::flash()
{
    QApplication::alert( this );
}

bool Container::processEvent(Event *e)
{
	if (m_tabBar == NULL)
		return false;
	switch (e->type()){
		case eEventMessageReceived:
			{
				EventMessage *em = static_cast<EventMessage*>(e);
				Message *msg = em->msg();
				if (msg->type() == MessageStatus){
					Contact *contact = getContacts()->contact(msg->contact());
					if (contact)
						contactChanged(contact);
					return false;
				}
				if (msg->getFlags() & MESSAGE_NOVIEW)
					return false;
				if (CorePlugin::instance()->getContainerMode())
				{
					if (isActiveWindow() && !isMinimized())
					{
						UserWnd *userWnd = m_tabBar->currentWnd();
						if (userWnd && (userWnd->id() == msg->contact()))
							userWnd->markAsRead();
					}
					else
					{
						UserWnd *userWnd = wnd(msg->contact());
						if (userWnd)
							QTimer::singleShot(0, this, SLOT(flash()));
					}
				}
				// no break here - otherwise we have to duplicate the code below...
			}
		case eEventMessageRead:
			{
				EventMessage *em = static_cast<EventMessage*>(e);
				Message *msg = em->msg();
				UserWnd *userWnd = wnd(msg->contact());
				if (userWnd){
					bool bHighlight = false;
					for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
						if (it->contact != msg->contact())
							continue;
						bHighlight = true;
						break;
					}
					m_tabBar->setHighlighted(msg->contact(), bHighlight);
				}
				break;
			}
		case eEventActiveContact:
			{
				EventActiveContact *eac = static_cast<EventActiveContact*>(e);
				if (!isActiveWindow())
					return false;
				UserWnd *userWnd = m_tabBar->currentWnd();
				if (userWnd) {
					eac->setContactID(userWnd->id());
					return true;
				}
				break;
			}
		case eEventContact:
			{
				EventContact *ec = static_cast<EventContact*>(e);
				Contact *contact = ec->contact();
				UserWnd *userWnd = wnd(contact->id());
				if(!userWnd)
					break;
				switch(ec->action())
				{
					case EventContact::eDeleted:
						{
							removeUserWnd(userWnd);
							break;
						}
					case EventContact::eChanged:
						{
							if (contact->getIgnore()){
								removeUserWnd(userWnd);
								break;
							}
							m_tabBar->changeTab(contact->id());
							contactChanged(contact);
							break;
						}
					case EventContact::eStatus:
						{
							unsigned style = 0;
							QSet<QString> wrkIcons;
							QString statusIcon;
							contact->contactInfo(style, statusIcon, &wrkIcons);
							bool bTyping = wrkIcons.contains("typing");
							if (userWnd->m_bTyping != bTyping)
							{
								userWnd->m_bTyping = bTyping;
								if (bTyping){
									userWnd->setStatus(g_i18n("%1 is typing", contact) .arg(contact->getName()));
								}else{
									userWnd->setStatus("");
								}
								userWnd = m_tabBar->currentWnd();
								if (userWnd && (contact->id() == userWnd->id()))
                                    m_status->showMessage(userWnd->status());
							}
						}
					default:
						break;
				}
				break;
			}
		case eEventClientsChanged:
			setupAccel();
			break;
		case eEventContactClient:
			{
				EventContactClient *ecc = static_cast<EventContactClient*>(e);
				contactChanged(ecc->contact());
				break;
			}
		case eEventInit:
			init();
			break;
		case eEventCommandExec:
			{
				EventCommandExec *ece = static_cast<EventCommandExec*>(e);
				CommandDef *cmd = ece->cmd();
				UserWnd *userWnd = m_tabBar->currentWnd();
				if (userWnd && ((unsigned long)(cmd->param) == userWnd->id())){
					if (cmd->menu_id == MenuContainerContact){
						m_tabBar->raiseTab(cmd->id);
						return true;
					}
					if (cmd->id == CmdClose){
						delete userWnd;
						return true;
					}
					if (cmd->id == CmdInfo && cmd->menu_id != MenuContact){
						CommandDef c = *cmd;
						c.menu_id = MenuContact;
						c.param   = (void*)userWnd->id();
						EventCommandExec(&c).process();
						return true;
					}
				}
				break;
			}
		case eEventCheckCommandState:
			{
				EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
				CommandDef *cmd = ecs->cmd();
				UserWnd *userWnd = m_tabBar->currentWnd();
				if (userWnd && ((unsigned long)(cmd->param) == userWnd->id()) &&
						(cmd->menu_id == MenuContainerContact) &&
						(cmd->id == CmdContainerContacts)){
					list<UserWnd*> userWnds = m_tabBar->windows();
					CommandDef *cmds = new CommandDef[userWnds.size() + 1];
					unsigned n = 0;
					for (list<UserWnd*>::iterator it = userWnds.begin(); it != userWnds.end(); ++it){
						cmds[n].id = (*it)->id();
						cmds[n].flags = COMMAND_DEFAULT;
						cmds[n].text_wrk = (*it)->getName();
						cmds[n].icon  = (*it)->getIcon();
						cmds[n].text  = "_";
						cmds[n].menu_id = n + 1;
						if (n < sizeof(accels) / sizeof(const char*))
							cmds[n].accel = accels[n];
						if (*it == m_tabBar->currentWnd())
							cmds[n].flags |= COMMAND_CHECKED;
						n++;
					}
					cmd->param = cmds;
					cmd->flags |= COMMAND_RECURSIVE;
					return true;
				}
				break;
			}
		default:
			break;
	}
	return false;
}

void Container::modeChanged()
{
    if (isReceived() && CorePlugin::instance()->getContainerMode())
        QTimer::singleShot(0, this, SLOT(close()));
    if (CorePlugin::instance()->getContainerMode() == 0){
        list<UserWnd*> wnds = m_tabBar->windows();
        for (list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
            if ((*it) != m_tabBar->currentWnd())
                delete (*it);
        }
    }
}

void Container::wndClosed()
{
    list<UserWnd*> wnds = m_tabBar->windows();
    for (list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
        if ((*it)->isClosed())
            delete (*it);
    }
}

bool Container::event(QEvent *e)
{
#ifdef WIN32
    if (e->type() == QEvent::WindowActivate)
        init();
#endif
    if ((e->type() == QEvent::WindowActivate)/* ||
            (((e->type() == QEvent::ShowNormal) ||
              (e->type() == QEvent::ShowMaximized)) && isActiveWindow())*/){
        UserWnd *userWnd = m_tabBar->currentWnd();
        if (m_bNoRead)
            m_bNoRead = false;
        if (userWnd)
            userWnd->markAsRead();

        if (m_bNoSwitch)
            m_bNoSwitch = false;
        else if ((userWnd == NULL) || !m_tabBar->isHighlighted(m_tabBar->current()))
        {
            list<UserWnd*> wnds = m_tabBar->windows();
            for (int i=0; i<m_tabBar->count();++i) //list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
            {                
                if (m_tabBar->isHighlighted(i))
                {
                    raiseUserWnd(i);
                    break;
                }
            }
        }
    }
    return QMainWindow::event(e);
}

void Container::contactChanged(Contact *contact)
{
    UserWnd *userWnd = NULL;
    if (m_tabBar){
        userWnd = m_tabBar->currentWnd();
    }else if (!m_childs.empty()){
        userWnd = m_childs.front();
    }
    if (userWnd && contact && (contact->id() == userWnd->id())){
        Command cmd;
        cmd->id = CmdContainerContact;
        cmd->text_wrk = userWnd->getName();
        cmd->icon  = userWnd->getIcon();
        cmd->param = (void*)(contact->id());
        cmd->popup_id = MenuContainerContact;
        cmd->flags = BTN_PICT;
        EventCommandChange e(cmd);
        m_bar->processEvent(&e);
        e.setNoProcess();
        setWindowIcon(Icon(cmd->icon));
        setWindowTitle(userWnd->getLongName());
    }
}

void Container::setReadMode()
{
    log(L_DEBUG, "Set read mode");
    m_bNoRead = false;
}

UserTabBar::UserTabBar(QWidget *parent) : QTabBar(parent)
{
    setShape(QTabBar::TriangularSouth);
}

UserWnd *UserTabBar::wnd(unsigned id)
{
    UserWnd *res = NULL;
    for (int t = 0; t < count(); t++)
    {
        UserWnd *wnd = wndForTab(t);
        if (wnd && wnd->id() == id)
        {
            res = wnd;
            break;
        }
    }
    return res;
}

int UserTabBar::tab(unsigned contactid)
{
    UserWnd *res = NULL;
    for (int t = 0; t < count(); t++)
    {
        UserWnd *wnd = wndForTab(t);
        if (wnd && wnd->id() == contactid)
        {
            res = wnd;
            return t;
        }
    }
    return -1;
}

void UserTabBar::raiseTab(unsigned id)
{
    for (int t = 0; t < count(); t++)
    {
        UserWnd *wnd = wndForTab(t);
        if (wnd && wnd->id() == id)
        {
            setCurrent(t);
            break;
        }
    }
}

list<UserWnd*> UserTabBar::windows()
{
    list<UserWnd*> res;
    for (int i = 0; i < count(); i++)
    {
        res.push_back(wndForTab(i));
    }
    return res;
}

void UserTabBar::setCurrent(unsigned n)
{
    setCurrentIndex(n);
}

unsigned UserTabBar::current()
{
    return currentIndex();
}

void UserTabBar::slotRepaint()
{
    repaint();
}

void UserTabBar::removeTab(unsigned id)
{
    layoutTabs();
    for (int t = 0; t < count(); t++)
    {
        UserWnd *wnd = wndForTab(t);
        if (wnd && wnd->id() == id)
        {
            QTabBar::removeTab(t);
            QTimer::singleShot(0, this, SLOT(slotRepaint()));
            break;
        }
    }
}

void UserTabBar::changeTab(unsigned id)
{
    layoutTabs();
    for (int t = 0; t < count(); t++)
    {
        UserWnd *wnd = wndForTab(t);
        if (wnd && wnd->id() == id)
        {
            setTabText(t,wnd->getName());
            QTimer::singleShot(0, this, SLOT(slotRepaint()));
            break;
        }
    }
}

void UserTabBar::setHighlighted(unsigned contactid, bool bHighlight) //bHighlight
{
    int index = tab(contactid);
    if (index == -1) return; //corresponding contact not found
    
    QColor c;
    if (bHighlight)
        c = QColor ( 255, 0, 0 ); //red
    else
        c = QColor (   0, 0, 0 ); //black

    this->setTabTextColor(index,c);
        
//    for(std::list<UserTab*>::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
//	{
//        UserTab *tab = *it;
//        if (tab->wnd()->id() == id)
//		{
//			repaint();
//            break;
//        }
//    }
}

bool UserTabBar::isHighlighted(int id /*UserWnd *wnd*/)
{
//    for(std::list<UserTab*>::iterator it = m_tabs.begin(); it != m_tabs.end(); ++it)
//	{
//        UserTab* tab = *it;
//        if (tab->wnd() == wnd)
//            return tab->isHighlighted();
//    }
    return this->tabTextColor(id)==QColor(255,0,0);
}

void UserTabBar::resizeEvent(QResizeEvent *e)
{
    QTabBar::resizeEvent(e);
    QTimer::singleShot(0, this, SLOT(slotRepaint()));
}

void UserTabBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::RightButton)
    {
        int id = tabAt(e->pos());
        if(id == -1)
            return;
        setCurrentIndex(id);
        UserWnd* wnd = wndForTab(id);
        if(NULL == wnd)
            return;

        EventMenuProcess eMenu(MenuContact, (void*)wnd->id());
        eMenu.process();
        QMenu *menu = eMenu.menu();
        if(menu)
            menu->popup(e->globalPos());
        return;
    }
    QTabBar::mousePressEvent(e);
}

UserWnd *UserTabBar::currentWnd()
{
    return wndForTab(currentIndex());
}

void UserTabBar::layoutTabs()
{
    //QTabBar::layoutTabs();
}

UserWnd* UserTabBar::wndForTab(int tab) {
  QVariant v = tabData(tab);
  if(!v.isValid())
    return NULL;
  if(!v.canConvert<UserWnd*>())
    return NULL;

  return v.value<UserWnd*>();
}


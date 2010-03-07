/***************************************************************************
                          search.cpp  -  description
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

#include "icons.h"
#include "search.h"
#include "usercfg.h"
#include "core.h"

#include "nonim.h"
#include "searchall.h"

#include "simgui/ballonmsg.h"
#include "simgui/toolbtn.h"
#include "simgui/listview.h"

#include "contacts/contact.h"
#include "contacts/client.h"
#include "contacts/group.h"
//#include "searchbase.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QFrame>
#include <QLineEdit>
#include <QMenu>
#include <QMoveEvent>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QResizeEvent>
#include <QStatusBar>
#include <QTimer>
#include <QValidator>

using namespace std;
using namespace SIM;

const unsigned COL_KEY			= 0x100;
const unsigned COL_SEARCH_WND	= 0x101;

SearchWidget::SearchWidget(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

SearchDialog::SearchDialog()
{
    SET_WNDPROC("search")
    setWindowIcon(Icon("find"));
    setButtonsPict(this);
    setWindowTitle(i18n("Search"));
    m_current = NULL;
    m_currentResult = NULL;
    m_bAdd = true;
    m_id		= 0;
    m_result_id = 0;
    m_active	= NULL;
    m_search	= new SearchWidget(this);
    m_update = new QTimer(this);
    connect(m_update, SIGNAL(timeout()), this, SLOT(update()));
    setCentralWidget(m_search);
    m_status = statusBar();
    m_result = NULL;
    setAdd(false);
    m_search->btnOptions->setIcon(Icon("1downarrow"));
    m_search->btnAdd->setIcon(Icon("add"));
    m_search->btnNew->setIcon(Icon("new"));
    connect(m_search->wndCondition, SIGNAL(aboutToShow(QWidget*)), this, SLOT(aboutToShow(QWidget*)));
    connect(m_search->wndResult, SIGNAL(aboutToShow(QWidget*)), this, SLOT(resultShow(QWidget*)));
    fillClients();
    connect(m_search->cmbClients, SIGNAL(activated(int)), this, SLOT(clientActivated(int)));
    m_result = new ListView(m_search->wndResult);
    m_result->addColumn(i18n("Results"));
    //m_result->setShowSortIndicator(true);
    m_result->setExpandingColumn(0);
    m_result->setFrameShadow(QFrame::Sunken);
    m_result->setLineWidth(1);
    addResult(m_result);
    showResult(NULL);
    aboutToShow(m_search->wndCondition->currentWidget());
    connect(m_search->btnSearch, SIGNAL(clicked()), this, SLOT(searchClick()));
    m_search->cmbClients->setFocus();
    connect(m_search->btnOptions, SIGNAL(clicked()), this, SLOT(optionsClick()));
    connect(m_search->btnAdd, SIGNAL(clicked()), this, SLOT(addClick()));
    m_search->btnOptions->setEnabled(false);
    m_search->btnAdd->setEnabled(false);
    connect(m_result, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_result, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(m_search->btnNew, SIGNAL(clicked()), this, SLOT(newSearch()));
    m_result->setMenu(MenuSearchItem);
    resultShow(m_result);
}

SearchDialog::~SearchDialog()
{
    // Fixme Todin
	//::saveGeometry(this, CorePlugin::instance()->data.SearchGeometry);
}

void SearchDialog::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    m_result->adjustColumn();
	/* Fixme Todin
    if (isVisible())
		::saveGeometry(this, CorePlugin::instance()->data.SearchGeometry);
		*/
}

void SearchDialog::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
	/* Fixme Todin
    if (isVisible())
		::saveGeometry(this, CorePlugin::instance()->data.SearchGeometry);
		*/
}

void SearchDialog::closeEvent(QCloseEvent *e)
{
    QMainWindow::closeEvent(e);
    emit finished();
}

void SearchDialog::setAdd(bool bAdd)
{
    if (m_bAdd == bAdd)
        return;
    m_bAdd = bAdd;
    setAddButton();
    setTitle();
}

void SearchDialog::setAddButton()
{
    QString text;
    QIcon icon;
    if (m_active){
        icon = Icon("cancel");
        text = i18n("&Cancel");
    }else if (m_bAdd){
        icon = Icon("add");
        text = i18n("&Add");
    }else{
        icon = Icon("find");
        text = i18n("&Search");
    }
    m_search->btnSearch->setText(text); //Fixme: btnSearch broken
    m_search->btnSearch->setIcon(icon);
}

void SearchDialog::fillClients()
{
    vector<ClientWidget> widgets = m_widgets;
    m_widgets.clear();
    m_search->cmbClients->clear();
    unsigned nClients = 0;
    int current    = -1;
    int defCurrent = -1;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        QWidget *search = client->searchWindow(m_search->wndCondition);
        if (search == NULL)
            continue;
        unsigned n;
        for (n = 0; n < widgets.size(); n++){
            if ((widgets[n].client != client) || !widgets[n].name.isEmpty())
                continue;
            delete search;
            search = widgets[n].widget;
            widgets[n].widget = NULL;
            break;
        }
        if (n >= widgets.size())
            m_id = m_search->wndCondition->addWidget(search);
        m_search->cmbClients->addItem(Icon(client->protocol()->description()->icon),
                                      CorePlugin::instance()->clientName(client));
        ClientWidget cw;
        cw.client = client;
        cw.widget = search;
        m_widgets.push_back(cw);
        if (search == m_current)
            current = m_widgets.size() - 1;
        if (client->protocol()->description()->flags & PROTOCOL_SEARCH)
            nClients++;
        if (client->name() == CorePlugin::instance()->value("SearchClient").toString())
            defCurrent = m_widgets.size() - 1;
    }


    if (nClients > 1){
        unsigned n;
        QWidget *search = NULL;
        for (n = 0; n < widgets.size(); n++){
            if (widgets[n].client == (Client*)(-1)){
                search = widgets[n].widget;
                widgets[n].widget = NULL;
                break;
            }
        }
        if (search == NULL){
            search = new SearchAll(NULL);
            m_id = m_search->wndCondition->addWidget(search);
        }
        m_search->cmbClients->addItem(Icon("find"), i18n("All networks"));
        ClientWidget cw;
        cw.client = (Client*)(-1);
        cw.widget = search;
        m_widgets.push_back(cw);
        if ((search == m_current) || ((m_current == NULL) && (current < 0) && (defCurrent < 0)))
            current = m_widgets.size() - 1;
    }
    unsigned n;
    QWidget *search = NULL;
    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].client == NULL){
            search = widgets[n].widget;
            widgets[n].widget = NULL;
            break;
        }
    }
    if (search == NULL){
        search = new NonIM(NULL);
        m_id = m_search->wndCondition->addWidget(search);
    }
    m_search->cmbClients->addItem(Icon("nonim"), i18n("Non-IM contact"));
    ClientWidget cw;
    cw.client = NULL;
    cw.widget = search;
    m_widgets.push_back(cw);
    if (search == m_current)
        current = m_widgets.size() - 1;

    if (m_update->isActive()){
        m_update->stop();
    }else if (m_result){
        m_result->viewport()->setUpdatesEnabled(false);
    }
    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].name.isEmpty())
            continue;
        unsigned i;
        for (i = 0; i < m_widgets.size(); i++)
            if (widgets[n].client == m_widgets[i].client)
                break;
        if (i >= m_widgets.size())
            continue;
        m_search->cmbClients->addItem(Icon(widgets[n].client->protocol()->description()->icon),
                                      widgets[n].name);
        m_widgets.push_back(widgets[n]);
        widgets[n].widget = NULL;
    }
    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].widget){
            if (widgets[n].widget == m_active)
                searchDone(m_active);
            if (widgets[n].widget == m_current)
                m_current = NULL;
            for (int i = 0; i < m_result->topLevelItemCount(); i++){
                ListViewItem *item = static_cast<ListViewItem*>(m_result->topLevelItem(i));
                if ((QWidget*)(item->text(COL_SEARCH_WND).toULong()) == widgets[n].widget)
                    delete item;
            }
            delete widgets[n].widget;
        }
    }

    if (current == -1)
        current = defCurrent;
    if (current == -1)
        current = 0;
    m_search->cmbClients->setCurrentIndex(current);
    clientActivated(current);
    setStatus();
    m_update->start(500);
}

void SearchDialog::clientActivated(int n)
{
    if ((unsigned)n >= m_widgets.size())
        return;
    searchDone(m_active);
    if (m_widgets[n].widget != m_current)
        showResult(NULL);
    m_search->wndCondition->setCurrentWidget(m_widgets[n].widget);
    setTitle();
}

void SearchDialog::setTitle()
{
    unsigned n = m_search->cmbClients->currentIndex();
    if (n >= m_widgets.size())
        return;
    Client *client = m_widgets[n].client;
    QString name;
    if ((client != NULL) && (client != (Client*)(-1)))
        name = client->name();
    CorePlugin::instance()->setValue("SearchClient", name);
    if (m_bAdd){
        setWindowTitle(i18n("Add") + ": " + m_search->cmbClients->currentText());
        setWindowIcon(Icon("add"));
    }else{
        setWindowTitle(i18n("Search") + ": " + m_search->cmbClients->currentText());
        setWindowIcon(Icon("find"));
    }
}

void SearchDialog::toggled(bool)
{
    textChanged();
}

bool SearchDialog::processEvent(Event *e)
{
    switch (e->type()){
    case eEventClientsChanged:
    case eEventClientChanged:
        fillClients();
        break;
    case eEventCommandExec:{
            if (m_result != m_currentResult)
                return false;
            EventCommandExec *ece = static_cast<EventCommandExec*>(e);
            CommandDef *cmd = ece->cmd();
            if (cmd->menu_id == MenuSearchGroups){
                Group *grp = getContacts()->group(cmd->id - CmdContactGroup);
                if (grp){
                    Contact *contact = NULL;
                    if ((QWidget*)(cmd->param) == m_search->btnSearch){
                        if (m_current){
                            connect(this, SIGNAL(createContact(unsigned,SIM::Contact*&)), m_current, SLOT(createContact(unsigned,SIM::Contact*&)));
                            emit createContact(CONTACT_TEMP, contact);
                            disconnect(this, SIGNAL(createContact(unsigned,SIM::Contact*&)), m_current, SLOT(createContact(unsigned,SIM::Contact*&)));
                        }
                    }else{
                        contact = createContact(CONTACT_TEMP);
                    }
                    if (contact){
                        if ((contact->getFlags() & CONTACT_TEMP) == 0){
                            QString err = i18n("%1 already in contact list") .arg(contact->getName());
                            if ((QWidget*)(cmd->param) == m_search->btnAdd){
                                BalloonMsg::message(err, m_search->btnAdd);
                            }else if ((QWidget*)(cmd->param) == m_search->btnSearch){
                                BalloonMsg::message(err, m_search->btnSearch);
                            }else{
                                BalloonMsg::message(err, m_result);
                            }
                            return true;
                        }
                        contact->setFlags(contact->getFlags() & ~CONTACT_TEMP);
                        contact->setGroup(grp->id());
                        EventContact(contact, EventContact::eChanged).process();
                    }
                }
                return true;
            }
            if (cmd->id == CmdSearchInfo){
                Contact *contact = createContact(CONTACT_TEMP);
                if (contact == NULL)
                    return true;
                Command cmd;
                cmd->id		 = CmdInfo;
                cmd->menu_id = MenuContact;
                cmd->param   = (void*)(contact->id());
                CorePlugin::instance()->showInfo(cmd);
                return true;
            }
            if (cmd->id == CmdSearchMsg){
                Contact *contact = createContact(CONTACT_TEMP);
                if (contact == NULL)
                    return true;
                Message *m = new Message(MessageGeneric);
                m->setContact(contact->id());
                EventOpenMessage(m).process();
                delete m;
            }
            break;
        }
    case eEventCheckCommandState:{
            EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
            CommandDef *cmd = ecs->cmd();
            if ((cmd->id == CmdSearchOptions) && (cmd->menu_id == MenuSearchItem)){
                EventMenuGetDef eMenu(MenuSearchOptions);
                eMenu.process();
                CommandsDef *def = eMenu.defs();
                if (def){
                    CommandsList list(*def, true);
                    CommandDef *s;
                    unsigned nItems = 0;
                    while ((s = ++list) != NULL)
                        nItems++;
                    if (nItems){
                        CommandDef *cmds = new CommandDef[nItems * 2 + 1];
                        list.reset();
                        nItems = 0;
                        unsigned prev = 0;
                        while ((s = ++list) != NULL){
                            if (s->flags & COMMAND_CHECK_STATE){
                                CommandDef cCheck = *s;
                                if (!EventCheckCommandState(&cCheck).process())
                                    continue;
                            }
                            if (prev && ((prev & 0xFF00) != (s->menu_grp & 0xFF00)))
                                cmds[nItems++].text = "_";
                            prev = s->menu_grp;
                            cmds[nItems++] = *s;
                        }
                        cmd->param = cmds;
                        cmd->flags |= COMMAND_RECURSIVE;
                        return true;
                    }
                }
                return false;
            }
            if ((cmd->id == CmdContactGroup) && (cmd->menu_id == MenuSearchGroups)){
                Group *grp;
                ContactList::GroupIterator it;
                unsigned nGrp = 0;
                while ((grp = ++it) != NULL)
                    nGrp++;
                it.reset();
                CommandDef *cmds = new CommandDef[nGrp + 1];
                nGrp = 0;
                while ((grp = ++it) != NULL){
                    if (grp->id() == 0)
                        continue;
                    cmds[nGrp].id      = CmdContactGroup + grp->id();
                    cmds[nGrp].menu_id = MenuSearchGroups;
                    cmds[nGrp].text    = "_";
                    cmds[nGrp].text_wrk = grp->getName();
                    nGrp++;
                }
                cmds[nGrp].id      = CmdContactGroup;
                cmds[nGrp].menu_id = MenuSearchGroups;
                cmds[nGrp].text    = I18N_NOOP("Not in list");

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

void SearchDialog::textChanged(const QString&)
{
    if (m_active != NULL){
        m_search->btnSearch->setEnabled(true);
        return;
    }
    bool bEnable = false;
    checkSearch(m_current, bEnable) && checkSearch(m_currentResult, bEnable);
    m_search->btnSearch->setEnabled(bEnable);
}

bool SearchDialog::checkSearch(QWidget *w, bool &bEnable)
{
    if (w == NULL)
        return true;

    const QList<QWidget *> l = qFindChildren<QWidget *>(w);
    foreach(QWidget *obj,l){
        if ((obj->parent() == NULL) ||
             qobject_cast<QToolBar*>(obj->parent()) ||
             qobject_cast<QComboBox*>(obj->parent()))
            continue;

        const QLineEdit *edit = qobject_cast<QLineEdit*>(obj);
        if (edit){
            if (edit->isEnabled()){
                if (!edit->text().isEmpty()){
                    const QValidator *v = edit->validator();
                    if (v){
                        QString text = edit->text();
                        int pos = 0;
                        if (v->validate(text, pos) == QValidator::Acceptable){
                            bEnable = true;
                        }else{
                            bEnable = false;
                            return false;
                        }
                    }else{
                        bEnable = true;
                    }
                }
            }
            continue;
        }
        const QComboBox *cmb = qobject_cast<QComboBox*>(obj);
        if (cmb){
            if (cmb->isEnabled() && !cmb->currentText().isEmpty())
                bEnable = true;
            continue;
        }
    }
    return true;
}

void SearchDialog::detach(QWidget *w)
{
    const QList<QWidget *> l = qFindChildren<QWidget *>(w);
    foreach(QWidget *obj,l){
        if (qobject_cast<QLineEdit*>(obj))
            disconnect(obj, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
        if (qobject_cast<QComboBox*>(obj))
            disconnect(obj, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
        if (qobject_cast<QRadioButton*>(obj))
            disconnect(obj, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    }
}

void SearchDialog::attach(QWidget *w)
{
    if (w == NULL)
        return;
    const QList<QWidget *> l = qFindChildren<QWidget *>(w);
    foreach(QWidget *obj,l){
        if (qobject_cast<QLineEdit*>(obj))
            connect(obj, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
        if (qobject_cast<QComboBox*>(obj))
            connect(obj, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
        if (qobject_cast<QRadioButton*>(obj))
            connect(obj, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    }
}

void SearchDialog::aboutToShow(QWidget *w)
{
    if (m_current)
        detach(m_current);
    m_current = w;
    attach(m_current);
    textChanged();
}

void SearchDialog::resultShow(QWidget *w)
{
    if (m_currentResult){
        if (m_currentResult != m_result)
            disconnect(m_currentResult, SIGNAL(enableOptions(bool)), this, SLOT(enableOptions(bool)));
        disconnect(m_currentResult, SIGNAL(destroyed()), this, SLOT(resultDestroyed()));
        detach(m_currentResult);
    }
    m_currentResult = w;
    attach(m_currentResult);
    connect(m_currentResult, SIGNAL(destroyed()), this, SLOT(resultDestroyed()));
    if (m_currentResult != m_result)
        connect(m_currentResult, SIGNAL(enableOptions(bool)), this, SLOT(enableOptions(bool)));
    textChanged();
}

void SearchDialog::resultDestroyed()
{
    m_currentResult = NULL;
}

void SearchDialog::addResult(QWidget *w)
{
    m_result_id = m_search->wndResult->addWidget(w);
}

void SearchDialog::showResult(QWidget *w)
{
    if (w == NULL)
        w = m_result;
    m_search->wndResult->setCurrentWidget(w);
    selectionChanged();
}

const unsigned NO_GROUP = 0x10000;

void SearchDialog::searchClick()
{
    if (m_bAdd){
        if (CorePlugin::instance()->value("GroupMode").toUInt())
        {
            EventMenuProcess eMenu(MenuSearchGroups, m_search->btnSearch);
            eMenu.process();
            QMenu *popup = eMenu.menu();
            if (popup)
                popup->popup(CToolButton::popupPos(m_search->btnSearch, popup));
        }
        else
        {
            Command cmd;
            cmd->id = CmdContactGroup;
            cmd->menu_id = MenuSearchGroups;
            cmd->param = m_search->btnSearch;
            EventCommandExec(cmd).process();
        }
        return;
    }
    if (m_active){
        emit searchStop();
        searchDone(m_active);
        return;
    }
    m_active = m_current;
    m_result->clear();
    m_search->btnAdd->setEnabled(false);
    m_search->btnOptions->setEnabled(false);
    setAddButton();
    setStatus();
    m_bColumns = false;
    connect(this, SIGNAL(search()), m_active, SLOT(search()));
    connect(this, SIGNAL(searchStop()), m_active, SLOT(searchStop()));
    connect(m_active, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(setColumns(const QStringList&, int, QWidget*)));
    connect(m_active, SIGNAL(addItem(const QStringList&,QWidget*)), this, SLOT(addItem(const QStringList&,QWidget*)));
    connect(m_active, SIGNAL(searchDone(QWidget*)), this, SLOT(searchDone(QWidget*)));
    emit search();
    m_result->setFocus();
}

void SearchDialog::setStatus()
{
    if (m_result == NULL)
        return;
    QString message = i18n("Search");
    if (m_result->firstChild()){
        message += ": ";
        message += i18n("%n contact found", "%n contacts found", m_result->columnCount());
    }
    m_status->showMessage(message);
}

void SearchDialog::searchDone(QWidget*)
{
    if (m_active == NULL)
        return;
    m_status->clearMessage();
    disconnect(this, SIGNAL(search()), m_active, SLOT(search()));
    disconnect(this, SIGNAL(searchStop()), m_active, SLOT(searchStop()));
    disconnect(m_active, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(setColumns(const QStringList&, int, QWidget*)));
    disconnect(m_active, SIGNAL(addItem(const QStringList&,QWidget*)), this, SLOT(addItem(const QStringList&,QWidget*)));
    disconnect(m_active, SIGNAL(searchDone(QWidget*)), this, SLOT(searchDone(QWidget*)));
    m_active = NULL;
    textChanged();
    setAddButton();
}

void SearchDialog::setColumns(const QStringList &columns, int n, QWidget*)
{
    int i;
    if (!m_bColumns){
		m_result->setColumnCount(0);
		/*
        for (i = m_result->columnCount() - 1; i >= 0; i--)
            m_result->removeColumn(i);
			*/
        m_bColumns = true;
    }
    for (i = 0; i < columns.count() / 2; i++)
        m_result->addColumn(columns[2 * i + 1]);
    m_result->setExpandingColumn(n);
    m_result->adjustColumn();
}

class SearchViewItem : public ListViewItem
{
public:
SearchViewItem(ListView *view) : ListViewItem(view) {}
    QString key(int column, bool ascending) const;
};

QString SearchViewItem::key(int column, bool ascending) const
{
	/*
    if (column)
        return ListViewItem::key(column, ascending);
		*/
    QString res = text(COL_KEY);
    return res;
}

void SearchDialog::addItem(const QStringList &values, QWidget *wnd)
{
    ListViewItem *item = NULL;
    for (int i = 0; i < m_result->topLevelItemCount(); i++){
        item = static_cast<ListViewItem*>(m_result->topLevelItem(i));
        if (item->text(COL_KEY) == values[1])
            break;
    }
    if (item){
        QWidget *oldSearch = (QWidget*)(item->text(COL_SEARCH_WND).toULong());
        for (unsigned i = 0; i < m_widgets.size(); i++){
            if (m_widgets[i].widget == wnd){
                item->setText(COL_SEARCH_WND, QString::number((unsigned long)wnd));
                return;
            }
            if (m_widgets[i].widget == oldSearch)
                return;
        }
        return;
    }
    if (m_update->isActive()){
        m_update->stop();
    }else{
        m_result->viewport()->setUpdatesEnabled(false);
    }
    item = new SearchViewItem(m_result);
    item->setPixmap(0, Pict(values[0]));
    item->setText(COL_KEY, values[1]);
    for (int i = 2; i < values.count(); i++)
        item->setText(i - 2, values[i]);
    item->setText(COL_SEARCH_WND, QString::number((unsigned long)wnd));
    setStatus();
    m_update->start(500);
}

void SearchDialog::update()
{
    m_update->stop();
    m_result->viewport()->setUpdatesEnabled(true);
    m_result->viewport()->repaint();
    m_result->adjustColumn();
}

void SearchDialog::selectionChanged()
{
    if (m_result && ((m_currentResult == NULL) || (m_currentResult == m_result))){
        bool bEnable = (m_result->selectedItems().count() > 0);
        enableOptions(bEnable);
    }
}

void SearchDialog::enableOptions(bool bEnable)
{
    m_search->btnAdd->setEnabled(bEnable);
    m_search->btnOptions->setEnabled(bEnable);
}

void SearchDialog::addClick()
{
    if (CorePlugin::instance()->value("GroupMode").toUInt()){
        EventMenuProcess eMenu(MenuSearchGroups, m_search->btnAdd);
        eMenu.process();
        QMenu *popup = eMenu.menu();
        if (popup)
            popup->popup(CToolButton::popupPos(m_search->btnAdd, popup));
    }else{
        Command cmd;
        cmd->id = CmdContactGroup;
        cmd->menu_id = MenuSearchGroups;
        cmd->param = m_search->btnAdd;
        EventCommandExec(cmd).process();
    }
}

Contact *SearchDialog::createContact(unsigned flags)
{
    Contact *contact = NULL;
    if (m_result->currentItem() == NULL)
        return NULL;
    QWidget *w = (QWidget*)(m_result->currentItem()->text(COL_SEARCH_WND).toULong());
	connect(this, SIGNAL(createContact(const QString&, unsigned, SIM::Contact*&)), w, SLOT(createContact(const QString&, unsigned, SIM::Contact*&)));
    QString name = m_result->currentItem()->text(0);
    emit createContact(name, flags, contact);
    disconnect(this, SIGNAL(createContact(const QString&, unsigned, SIM::Contact*&)), w, SLOT(createContact(const QString&, unsigned, SIM::Contact*&)));
    return contact;
}

void SearchDialog::dragStart()
{
    Contact *contact = createContact(CONTACT_DRAG);
    if (contact == NULL)
        return;
//    m_result->startDrag(new ContactDragObject(m_result, contact));
}

void SearchDialog::optionsClick()
{
    EventMenuProcess eMenu(MenuSearchOptions, NULL);
    eMenu.process();
    QMenu *popup = eMenu.menu();
    if (popup)
        popup->popup(CToolButton::popupPos(m_search->btnOptions, popup));
}

void SearchDialog::newSearch()
{
    searchStop();
    const QList<QWidget *> l = qFindChildren<QWidget *>(this);
    foreach(QWidget *obj,l){
        QWidget *parent = static_cast<QWidget*>(obj)->parentWidget();

        QLineEdit *le = qobject_cast<QLineEdit*>(obj);
        if (le && parent && qobject_cast<QComboBox*>(parent) == NULL)
            le->clear();

        QComboBox *cb = qobject_cast<QComboBox*>(obj);
        if (cb && parent && qobject_cast<QFrame*>(parent) == NULL)
            cb->setCurrentIndex(0);
    }
    m_result->clear();
	/*
    for (int i = m_result->columnCount() - 1; i >= 0; i--)
        m_result->removeColumn(i);
		*/
	m_result->setColumnCount(0);
    m_result->addColumn(i18n("Results"));
    m_result->setExpandingColumn(0);
    m_result->adjustColumn();
}

void SearchDialog::addSearch(QWidget *w, Client *client, const QString &name)
{
    for (unsigned i = 0; i < m_widgets.size(); i++){
        if ((m_widgets[i].client == client) && (m_widgets[i].name == name)){
            delete w;
            m_search->cmbClients->setCurrentIndex(i);
            clientActivated(i);
            return;
        }
    }
    m_id = m_search->wndCondition->addWidget(w);
    ClientWidget cw;
    cw.widget = w;
    cw.client = client;
    cw.name   = name;
    m_widgets.push_back(cw);
    m_search->cmbClients->addItem(Icon(client->protocol()->description()->icon), name);
    m_search->cmbClients->setCurrentIndex(m_widgets.size() - 1);
    clientActivated(m_widgets.size() - 1);
}

void SearchDialog::showClient(Client *client)
{
    for (unsigned i = 0; i < m_widgets.size(); i++){
        if (m_widgets[i].client != client)
            continue;
        m_search->cmbClients->setCurrentIndex(i);
        clientActivated(i);
        return;
    }
}

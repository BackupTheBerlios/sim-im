/***************************************************************************
                          jabberbrowser.cpp  -  description
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
#include "jabberclient.h"
#include "jabberbrowser.h"
#include "jabbersearch.h"
#include "jabber.h"
#include "discoinfo.h"
#include "simgui/listview.h"
#include "simgui/toolbtn.h"
#include "jidsearch.h"
#include "simgui/ballonmsg.h"

#include "core.h"

#include "contacts/contact.h"

#include <QPixmap>
#include <QToolBar>
#include <QTimer>
#include <QLabel>
#include <QApplication>
#include <QShowEvent>
#include <QDesktopWidget>

using namespace std;
using namespace SIM;

const unsigned BROWSE_INFO	= 8;

JabberWizard::JabberWizard(QWidget *parent, const QString &title, const QString &icon, JabberClient *client, const QString &jid, const QString &node, const QString &type)
        : QWizard(parent, Qt::Window)
{
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_type = type;
    m_search = new JabberSearch;
    m_search->init(this, client, jid, node, title, m_type == "register");
    addPage(m_search);
	/*
    m_result = new QLabel(this);
    addPage(m_result);
	*/
    m_result->setText(i18n("Process"));
    //helpButton()->hide();
    SET_WNDPROC("jbrowser")
    setWindowIcon(Icon(icon));
    setWindowTitle(title);
    connect(this, SIGNAL(selected(const QString&)), this, SLOT(slotSelected(const QString&)));
}

void JabberWizard::search()
{
    //showPage(m_result);
}

void JabberWizard::textChanged(const QString&)
{
    setNext();
}

void JabberWizard::slotSelected(const QString&)
{
    //if (currentPage() != m_result)
        return;
    //setFinishEnabled(m_result, false);
    QString condition = m_search->condition(NULL); //Fixme unreachable code
    m_id = m_search->m_client->process(m_search->m_jid, m_search->m_node, condition, m_type);
}

bool JabberWizard::processEvent(Event *e)
{
    if (e->type() == eEventAgentRegister){
        EventAgentRegister *ear = static_cast<EventAgentRegister*>(e);
        agentRegisterInfo *ai = ear->registerInfo();
        if (m_id == ai->id){
            if (ai->err_code){
                QString err = i18n(ai->error);
                if (err.isEmpty())
                    err = i18n("Error %1").arg(ai->err_code);
                m_result->setText(err);
            }else{
                m_result->setText(i18n("Done"));
                //setFinishEnabled(m_result, true);
                QTimer::singleShot(0, this, SLOT(close()));
            }
            return true;
        }
    }
    return false;
}

void JabberWizard::setNext()
{
    //nextButton()->setEnabled(m_search->canSearch());
}

void JabberWizard::initTitle()
{
    if (m_search->m_title.isEmpty())
        return;
    setWindowTitle(m_search->m_title);
}

JabberBrowser::JabberBrowser()
{
    m_client = NULL;
    m_info   = NULL;

    m_list = new ListView(this);
    m_list->addColumn(i18n("Name"));
    m_list->addColumn(i18n("JID"));
    m_list->addColumn(i18n("Node"));
    m_list->setExpandingColumn(0);
    m_list->setMenu(0);
    connect(m_list, SIGNAL(currentChanged(ListViewItem*)), this, SLOT(currentChanged(ListViewItem*)));
    connect(m_list, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_list, SIGNAL(dragStart()), this, SLOT(dragStart()));

    EventToolbar e(BarBrowser, this);
    e.process();
    m_bar = e.toolBar();
    m_bar->setParam(this);
    restoreToolbar(m_bar, JabberPlugin::plugin->data.browser_bar);
    m_bar->show();
    resize(qApp->desktop()->width(), qApp->desktop()->height());
    setCentralWidget(m_list);
    m_historyPos = -1;

    Command cmd;
    cmd->id		= CmdUrl;
    cmd->param	= this;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolCombo *cmbUrl = dynamic_cast<CToolCombo*>(eWidget.widget());
    if (cmbUrl){
        QString h = JabberPlugin::plugin->getBrowserHistory();
        while (h.length())
            cmbUrl->insertItem(INT_MAX,getToken(h, ';'));
        cmbUrl->setText(QString::null);
    }

    m_reg    = NULL;
    m_config = NULL;
    m_search = NULL;
    m_bInProcess = false;
    m_list->setMenu(MenuSearchItem);
}

JabberBrowser::~JabberBrowser()
{
    if (m_info)
        delete m_info;
    save();
}

void JabberBrowser::setClient(JabberClient *client)
{
    if (m_client == client)
        return;
    m_client = client;
    QString url;
    if (m_client->getUseVHost())
        url = m_client->getVHost();
    if (url.isEmpty())
        url = m_client->getServer();
    goUrl(url, QString::null);
}

void JabberBrowser::goUrl(const QString &url, const QString &node)
{
    int i = 0;
    vector<QString>::iterator it;
    for (it = m_history.begin(); it != m_history.end(); ++it, i++){
        if (i > m_historyPos)
            break;
    }
    m_history.erase(it, m_history.end());
    m_history.push_back(url);
    i = 0;
    for (it = m_nodes.begin(); it != m_nodes.end(); ++it, i++){
        if (i > m_historyPos)
            break;
    }
    m_nodes.erase(it, m_nodes.end());
    m_nodes.push_back(node);
    m_historyPos++;
    go(url, node);
}

void JabberBrowser::go(const QString &url, const QString &node)
{
    setNavigation();
    Command cmd;
    m_list->clear();
    cmd->id		= CmdBrowseInfo;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdBrowseSearch;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdRegister;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdBrowseConfigure;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();

    m_bInProcess = true;
    ListViewItem *item = new ListViewItem(m_list);
    item->setText(COL_JID, url);
    item->setText(COL_NAME, url);
    item->setText(COL_NODE, node);
    m_bError = false;
    unsigned mode = 0;
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO){
        item->setText(COL_ID_DISCO_ITEMS, m_client->discoItems(url, node));
        item->setText(COL_ID_DISCO_INFO, m_client->discoInfo(url, node));
        mode = BROWSE_DISCO | BROWSE_INFO;
    }
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_BROWSE){
        if (node.isEmpty()){
            item->setText(COL_ID_BROWSE, m_client->browse(url));
            mode |= BROWSE_BROWSE;
        }
    }
    item->setText(COL_MODE, QString::number(mode));
    item->setPixmap(COL_NAME, Pict("empty"));
    cmd->id		= CmdUrl;
    cmd->param	= this;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolCombo *cmbUrl = dynamic_cast<CToolCombo*>(eWidget.widget());
    if (cmbUrl)
        cmbUrl->setText(url);
    cmd->id		= CmdNode;
    EventCommandWidget eWidget2(cmd);
    eWidget2.process();
    CToolCombo *cmbNode = dynamic_cast<CToolCombo*>(eWidget2.widget());
    if (cmbNode)
        cmbNode->setText(node);
    startProcess();
    if (item->text(COL_ID_DISCO_INFO).isEmpty())
        stop(i18n("Client offline"));
}

void JabberBrowser::startProcess()
{
    Command cmd;
    cmd->id			 = CmdUrl;
    cmd->text		 = I18N_NOOP("Stop");
    cmd->icon		 = "cancel";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    cmd->param		 = this;
    EventCommandChange(cmd).process();
}

void JabberBrowser::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);
    selectionChanged();
}

void JabberBrowser::selectionChanged()
{
    emit enableOptions(m_list->selectedItems().count() > 0);
}

void JabberBrowser::save()
{
    saveToolbar(m_bar, JabberPlugin::plugin->data.browser_bar);
}

bool JabberBrowser::processEvent(Event *e)
{
    if (e->type() == eEventAgentInfo){
        EventAgentInfo *eai = static_cast<EventAgentInfo*>(e);
        JabberAgentInfo *data = eai->agentInfo();
        if (m_search_id == data->ReqID.str()){
            if (data->Type.str().isEmpty()){
                if (data->nOptions.toULong()){
                    QString err = i18n(data->Label.str());
                    if (err.isEmpty())
                        err = i18n("Error %1") .arg(data->nOptions.toULong());
                    m_search_id = QString::null;
                    Command cmd;
                    cmd->id		= CmdBrowseSearch;
                    cmd->param	= this;
                    EventCommandWidget eWidget(cmd);
                    eWidget.process();
                    QWidget *parent = eWidget.widget();
                    if (parent == NULL)
                        parent = this;
                    BalloonMsg::message(err, parent);
                    delete m_search;
                }else{
                    m_search->jidSearch->addWidget(data);
                    connect(this, SIGNAL(addSearch(QWidget*, SIM::Client*, const QString&)), topLevelWidget(), SLOT(addSearch(QWidget*, SIM::Client*, const QString&)));
                    emit addSearch(m_search, m_client, m_search->m_jid);
                    disconnect(this, SIGNAL(addSearch(QWidget*, SIM::Client*, const QString&)), topLevelWidget(), SLOT(addSearch(QWidget*, SIM::Client*, const QString&)));
                }
                m_search_id = QString::null;
                m_search    = NULL;
                return true;
            }
            m_search->jidSearch->addWidget(data);
            return true;
        }
        if (m_reg_id == data->ReqID.str()) {
            if (data->Type.str().isEmpty()){
                if (data->nOptions.toULong()){
                    QString err = i18n(data->Label.str());
                    if (err.isEmpty())
                        err = i18n("Error %1") .arg(data->nOptions.toULong());
                    m_reg_id = QString::null;
                    delete m_reg;
                    m_reg = NULL;
                    Command cmd;
                    cmd->id		= CmdRegister;
                    cmd->param	= this;
                    EventCommandWidget eWidget(cmd);
                    eWidget.process();
                    QWidget *parent = eWidget.widget();
                    if (parent == NULL)
                        parent = this;
                    BalloonMsg::message(err, parent);
                }else if (m_reg){
                    m_reg->m_search->addWidget(data);
                    QTimer::singleShot(0, this, SLOT(showReg()));
                }
                m_reg_id = QString::null;
                return true;
            }
            if (m_reg)
                m_reg->m_search->addWidget(data);
            return true;
        }
        if (m_config_id == data->ReqID.str()){
            if (data->Type.str().isEmpty()){
                if (data->nOptions.toULong()){
                    QString err = i18n(data->Label.str());
                    if (err.isEmpty())
                        err = i18n("Error %1") .arg(data->nOptions.toULong());
                    m_config_id = QString::null;
                    delete m_config;
                    m_config = NULL;
                    Command cmd;
                    cmd->id     = CmdBrowseConfigure;
                    cmd->param	= this;
                    EventCommandWidget eWidget(cmd);
                    eWidget.process();
                    QWidget *parent = eWidget.widget();
                    if (parent == NULL)
                        parent = this;
                    BalloonMsg::message(err, parent);
                }else if (m_config){
                    m_config->m_search->addWidget(data);
                    QTimer::singleShot(0, this, SLOT(showConfig()));
                }
                m_config_id = QString::null;
                return true;
            }
            if (m_config)
                m_config->m_search->addWidget(data);
            return true;
        }
    } else
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if ((cmd->menu_id == MenuSearchOptions) && isVisible()){
            cmd->flags &= ~COMMAND_CHECKED;
            switch (cmd->id){
            case CmdBrowseSearch:
                if (haveFeature("jabber:iq:search"))
                    return true;
                break;
            case CmdRegister:
                if (haveFeature("jabber:iq:register"))
                    return true;
                break;
            case CmdBrowseConfigure:
                if (haveFeature("jabber:iq:data"))
                    return true;
                break;
            }
            return false;
        }
        if (cmd->param != this)
            return false;
        if (cmd->menu_id != MenuBrowser)
            return false;
        cmd->flags &= ~COMMAND_CHECKED;
        switch (cmd->id){
        case CmdOneLevel:
            if (!JabberPlugin::plugin->getAllLevels())
                cmd->flags |= COMMAND_CHECKED;
            return true;
        case CmdAllLevels:
            if (JabberPlugin::plugin->getAllLevels())
                cmd->flags |= COMMAND_CHECKED;
            return true;
        case CmdModeDisco:
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO)
                cmd->flags |= COMMAND_CHECKED;
            return true;
        case CmdModeBrowse:
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_BROWSE)
                cmd->flags |= COMMAND_CHECKED;
            return true;
        case CmdModeAgents:
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_AGENTS)
                cmd->flags |= COMMAND_CHECKED;
            return true;
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (((cmd->menu_id == MenuSearchItem) || (cmd->menu_id == MenuSearchOptions)) && isVisible()){
            Command c;
            c->id    = cmd->id;
            if (cmd->id == CmdSearchInfo)
                c->id = CmdBrowseInfo;
            c->param = this;
            EventCommandExec(c).process();
        }
        if (cmd->param != this)
            return false;
        ListViewItem *item = m_list->currentItem();
        if (cmd->menu_id == MenuBrowser){
            cmd->flags &= ~COMMAND_CHECKED;
            unsigned mode = JabberPlugin::plugin->getBrowseType();
            switch (cmd->id){
            case CmdOneLevel:
                JabberPlugin::plugin->setAllLevels(false);
                changeMode();
                return true;
            case CmdAllLevels:
                JabberPlugin::plugin->setAllLevels(true);
                changeMode();
                return true;
            case CmdModeDisco:
                mode ^= BROWSE_DISCO;
                JabberPlugin::plugin->setBrowseType(mode);
                changeMode();
                return true;
            case CmdModeBrowse:
                mode ^= BROWSE_BROWSE;
                JabberPlugin::plugin->setBrowseType(mode);
                changeMode();
                return true;
            case CmdModeAgents:
                mode ^= BROWSE_AGENTS;
                JabberPlugin::plugin->setBrowseType(mode);
                changeMode();
                return true;
            }
            return false;
        }
        if (item){
            if (cmd->id == CmdBrowseSearch){
                if (m_search)
                    delete m_search;
                m_search = new JIDSearch(this, m_client, item->text(COL_JID), item->text(COL_NODE), item->text(COL_TYPE));
                m_search->jidSearch->init(this, m_client, m_search->m_jid, m_search->m_node, QString::null, false);
                m_search_id = m_client->get_agent_info(item->text(COL_JID), item->text(COL_NODE), "search");
                return true;
            }
            if (cmd->id == CmdRegister){
                if (m_reg)
                    delete m_reg;
                m_reg = new JabberWizard(this, i18n("%1 Register") .arg(item->text(COL_NAME)), "reg", m_client, item->text(COL_JID), item->text(COL_NODE), "register");
                connect(m_reg, SIGNAL(destroyed()), this, SLOT(regFinished()));
                m_reg_id = m_client->get_agent_info(item->text(COL_JID), item->text(COL_NODE), "register");
                return true;
            }
            if (cmd->id == CmdBrowseConfigure){
                if (m_config)
                    delete m_config;
                m_config = new JabberWizard(this, i18n("%1 Configure") .arg(item->text(COL_NAME)), "configure", m_client, item->text(COL_JID), item->text(COL_NODE), "data");
                connect(m_config, SIGNAL(destroyed()), this, SLOT(configFinished()));
                m_config_id = m_client->get_agent_info(item->text(COL_JID), item->text(COL_NODE), "data");
                return true;
            }
            if (cmd->id == CmdBrowseInfo){
                if (m_info == NULL)
                    m_info = new DiscoInfo(this, m_list->currentItem()->text(COL_FEATURES), item->text(COL_NAME), item->text(COL_TYPE), item->text(COL_CATEGORY));
                m_info->reset();
                raiseWindow(m_info);
                return true;
            }
        }
        if (cmd->id == CmdBack){
            if (m_historyPos){
                m_historyPos--;
                QString url  = m_history[m_historyPos];
                QString node = m_nodes[m_historyPos];
                go(url, node);
            }
        }
        if (cmd->id == CmdForward){
            if (m_historyPos + 1 < (int)(m_history.size())){
                m_historyPos++;
                QString url  = m_history[m_historyPos];
                QString node = m_nodes[m_historyPos];
                go(url, node);
            }
        }
        if (cmd->id == CmdUrl){
            if (m_bInProcess){
                stop(QString::null);
                return true;
            }
            QString jid;
            QString node;
            Command cmd;
            cmd->id    = CmdUrl;
            cmd->param = this;
            EventCommandWidget eWidget(cmd);
            eWidget.process();
            CToolCombo *cmbUrl = dynamic_cast<CToolCombo*>(eWidget.widget());
            if (cmbUrl)
                jid = cmbUrl->lineEdit()->text();
            cmd->id = CmdNode;
            EventCommandWidget eWidget2(cmd);
            eWidget2.process();
            CToolCombo *cmbNode = dynamic_cast<CToolCombo*>(eWidget2.widget());
            if (cmbNode)
                node = cmbNode->lineEdit()->text();
            if (!jid.isEmpty()){
                addHistory(jid);
                goUrl(jid, node);
            }
            return true;
        }
    } else
    if (e->type() == eEventDiscoItem){
        if (!m_bInProcess)
            return false;
        EventDiscoItem *edi = static_cast<EventDiscoItem*>(e);
        DiscoItem *item = edi->item();
        ListViewItem *it = findItem(COL_ID_DISCO_ITEMS, item->id);
        if (it){
            if (item->jid.isEmpty()){
                it->setText(COL_ID_DISCO_ITEMS, QString::null);
                if (it != m_list->topLevelItem(0)){
                    checkDone();
                    adjustColumn(it);
                    return true;
                }
                QString err;
                if (!item->name.isEmpty()){
                    err = item->name;
                }else if (!item->node.isEmpty()){
                    err = i18n("Error %1") .arg(item->node.toULong());
                }
                if (!err.isEmpty()){
                    unsigned mode = it->text(COL_MODE).toLong();
                    if (((mode & BROWSE_BROWSE) == 0) || (it->text(COL_ID_BROWSE).isEmpty() & m_bError))
                        stop(err);
                    m_bError = true;
                }
                checkDone();
                adjustColumn(it);
                return true;
            }
            if (it->child(0) == NULL){
                it->setExpandable(true);
                if ((it == m_list->topLevelItem(0)) || (it == m_list->currentItem()))
                    it->setOpen(true);
            }
			ListViewItem *i;
			for(int c = 0; c < m_list->topLevelItemCount(); c++)
			{
				ListViewItem *i= static_cast<ListViewItem*>(m_list->topLevelItem(c));
				if ((i->text(COL_JID) == item->jid) &&
						(i->text(COL_NODE) == item->node))
					return true;
			}
            i = new ListViewItem(it);
            i->setText(COL_JID, item->jid);
            i->setText(COL_NAME, item->name.isEmpty() ? item->jid : item->name);
            i->setText(COL_NODE, item->node);
            int mode = 0;
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO){
                i->setText(COL_ID_DISCO_INFO, m_client->discoInfo(item->jid, item->node));
                mode |= BROWSE_INFO;
            }
            i->setText(COL_MODE, QString::number(mode));
            if (JabberPlugin::plugin->getAllLevels())
                loadItem(i);
            return true;
        }
        it = findItem(COL_ID_DISCO_INFO, item->id);
        if (it){
            if (item->jid.isEmpty()){
                it->setText(COL_ID_DISCO_INFO, QString::null);
                checkDone();
                adjustColumn(it);
                return true;
            }
            if (it->text(COL_NAME) == it->text(COL_JID))
                it->setText(COL_NAME, item->name);
            it->setText(COL_CATEGORY, item->category);
            it->setText(COL_TYPE, item->type);
            it->setText(COL_FEATURES, item->features);
            if ((JabberPlugin::plugin->getAllLevels()) || (it == m_list->currentItem()))
                loadItem(it);
            setItemPict(it);
            if (it == m_list->currentItem())
                currentChanged(it);
            return true;
        }
        it = findItem(COL_ID_BROWSE, item->id);
        if (it){
            if (item->jid.isEmpty()){
                it->setText(COL_ID_BROWSE, QString::null);
                if (it != m_list->topLevelItem(0)){
                    checkDone();
                    adjustColumn(it);
                    return true;
                }
                QString err;
                if (!item->name.isEmpty()){
                    err = item->name;
                }else if (!item->node.isEmpty()){
                    err = i18n("Error %1") .arg(item->node.toULong());
                }
                if (!err.isEmpty()){
                    unsigned mode = it->text(COL_MODE).toLong();
                    if (((mode & BROWSE_DISCO) == 0) || (it->text(COL_ID_DISCO_ITEMS).isEmpty() & m_bError))
                        stop(err);
                    m_bError = true;
                }
                checkDone();
                adjustColumn(it);
                return true;
            }
            ListViewItem* i=NULL;
			if (it->text(COL_JID) != item->jid){
				
				for(int c = 0; c < it->childCount(); c++)
				{
					i= static_cast<ListViewItem*>(it->child(0));
					if ((i->text(COL_JID) == item->jid) &&
							(i->text(COL_NODE) == item->node))
						break;
				}
				if (i) //Fixme warning C4701: potentially uninitialized local variable 'i' used
				{
                    it = i;
                }else{
                    if (it->child(0) == NULL){
                        it->setExpandable(true);
                        if ((it == m_list->topLevelItem(0)) || (it == m_list->currentItem()))
                            it->setOpen(true);
                    }
                    it = new ListViewItem(it);
                    it->setText(COL_JID, item->jid);
                    it->setText(COL_MODE, "0");
                    if (JabberPlugin::plugin->getAllLevels())
                        loadItem(it);
                }
            }
            if (it->text(COL_NAME).isEmpty() || (it->text(COL_NAME) == it->text(COL_JID)))
                it->setText(COL_NAME, item->name);
            it->setText(COL_NODE, QString::null);
            it->setText(COL_CATEGORY, item->category);
            it->setText(COL_TYPE, item->type);
            it->setText(COL_FEATURES, item->features);
            if (JabberPlugin::plugin->getAllLevels() || (it == m_list->currentItem()))
                loadItem(it);
            setItemPict(it);
            return true;
        }
    }
    return false;
}

void JabberBrowser::configFinished()
{
    m_config = NULL;
}

void JabberBrowser::regFinished()
{
    m_reg = NULL;
}

void JabberBrowser::setNavigation()
{
    Command cmd;
    cmd->id		= CmdBack;
    cmd->flags	= m_historyPos ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdForward;
    cmd->flags	= (m_historyPos + 1 < (int)(m_history.size())) ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();
}

void JabberBrowser::currentChanged(ListViewItem*)
{
    Command cmd;
    cmd->id		= CmdBrowseInfo;
    cmd->flags	= m_list->currentItem() ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdBrowseSearch;
    cmd->flags	= haveFeature("jabber:iq:search") ? 0 : COMMAND_DISABLED;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdRegister;
    cmd->flags	= haveFeature("jabber:iq:register") ? 0 : COMMAND_DISABLED;
    EventCommandDisabled(cmd).process();

    cmd->id		= CmdBrowseConfigure;
    cmd->flags	= haveFeature("jabber:iq:data") ? 0 : COMMAND_DISABLED;
    EventCommandDisabled(cmd).process();

    ListViewItem *item = m_list->currentItem();
    if (item == NULL)
        return;
    loadItem(item);
}

void JabberBrowser::loadItem(ListViewItem *item)
{
    bool bProcess = false;
    unsigned mode = item->text(COL_MODE).toLong();
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO){
        if (((mode & BROWSE_DISCO) == 0) && item->text(COL_ID_DISCO_ITEMS).isEmpty()){
            item->setText(COL_ID_DISCO_ITEMS, m_client->discoItems(item->text(COL_JID), item->text(COL_NODE)));
            mode |= BROWSE_DISCO;
            bProcess = true;
        }
        if (((mode & BROWSE_INFO) == 0) && item->text(COL_ID_DISCO_INFO).isEmpty()){
            item->setText(COL_ID_DISCO_INFO, m_client->discoInfo(item->text(COL_JID), item->text(COL_NODE)));
            mode |= BROWSE_INFO;
            bProcess = true;
        }
    }
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_BROWSE){
        if (((mode & BROWSE_BROWSE) == 0) && item->text(COL_ID_BROWSE).isEmpty() && haveFeature("iq:id:browse", item->text(COL_FEATURES))){
            item->setText(COL_ID_BROWSE, m_client->browse(item->text(COL_JID)));
            mode |= BROWSE_BROWSE;
            bProcess = true;
        }
    }
    item->setText(COL_MODE, QString::number(mode));
    if (!m_bInProcess && bProcess){
        m_bInProcess = true;
        startProcess();
    }
}

void JabberBrowser::changeMode()
{
    if (JabberPlugin::plugin->getAllLevels()){
        if (m_list->firstChild())
            changeMode(m_list->firstChild());
    }else{
        if (m_list->firstChild())
            loadItem(m_list->firstChild());
        if (m_list->currentItem())
            loadItem(m_list->currentItem());
    }
}

void JabberBrowser::changeMode(ListViewItem *item)
{
    loadItem(item);
    for(int c = 0; c < item->childCount(); c++)
    {
//        ListViewItem *i = static_cast<ListViewItem*>(item->child(c));
        changeMode(item);
    }
}

void JabberBrowser::dragStart()
{
/*
    ListViewItem *item = m_list->currentItem();
    if (item == NULL)
        return;
    Contact *contact;
    QString resource;
    JabberUserData *data = m_client->findContact(item->text(COL_JID), QString::null, false, contact, resource);
    if (data == NULL){
        m_client->findContact(item->text(COL_JID), item->text(COL_NAME), true, contact, resource);
        contact->setFlags(CONTACT_DRAG);
    }
    m_list->startDrag(new ContactDragObject(m_list, contact));
*/
}

void JabberBrowser::stop(const QString &err)
{
    if (!m_bInProcess)
        return;
    m_bInProcess = false;
    Command cmd;
    cmd->id			 = CmdUrl;
    cmd->text		 = I18N_NOOP("JID");
    cmd->icon		 = "run";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    cmd->param		 = this;
    EventCommandChange(cmd).process();
    if (!err.isEmpty()){
        Command cmd;
        cmd->id		= CmdUrl;
        cmd->param	= this;
        EventCommandWidget eWidget(cmd);
        eWidget.process();
        QWidget *parent = eWidget.widget();
        if (parent == NULL)
            parent = this;
        BalloonMsg::message(err, parent);
    }
}

const unsigned MAX_HISTORY = 10;

void JabberBrowser::addHistory(const QString &str)
{
    QStringList l = JabberPlugin::plugin->getBrowserHistory().split(';');
    l.removeAll(str);
    l.prepend(str);
    QString res;
    Command cmd;
    cmd->id		= CmdUrl;
    cmd->param	= this;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    CToolCombo *cmbUrl = dynamic_cast<CToolCombo*>(eWidget.widget());
    if (cmbUrl)
        cmbUrl->clear();
    unsigned i = 0;
    Q_FOREACH(const QString &str, l) {
        if (i++ > MAX_HISTORY)
            break;
        if (!res.isEmpty())
            res += ';';
        cmbUrl->addItem(str);
        res += quoteChars(str, ";");
    }
    JabberPlugin::plugin->setBrowserHistory(res);
}

bool JabberBrowser::haveFeature(const char *feature)
{
    QString features;
    if (m_list->currentItem())
        features = m_list->currentItem()->text(COL_FEATURES);
    return haveFeature(feature, features);
}

bool JabberBrowser::haveFeature(const char *feature, const QString &features)
{
    if (features.isEmpty())
        return false;
    QString ff = features;
    while (!ff.isEmpty()){
        QString f = getToken(ff, '\n');
        if (f == feature)
            return true;
    }
    return false;
}

void JabberBrowser::showReg()
{
    if (m_reg){
        m_reg->initTitle();
        QTimer::singleShot(0, m_reg, SLOT(setNext()));
        m_reg->show();
    }
}

void JabberBrowser::showConfig()
{
    if (m_config){
        m_config->initTitle();
        QTimer::singleShot(0, m_config, SLOT(setNext()));
        m_config->show();
    }
}

ListViewItem *JabberBrowser::findItem(unsigned col, const QString &id)
{
    if (m_list->firstChild() == NULL)
        return NULL;
    return findItem(col, id, m_list->firstChild());
}

ListViewItem *JabberBrowser::findItem(unsigned col, const QString &id, ListViewItem *item)
{
	if (item->text(col) == id)
		return item;
	for(int c = 0; c < item->childCount(); c++)
	{
		ListViewItem *i= static_cast<ListViewItem*>(item->child(c));
		ListViewItem *res = findItem(col, id, i);
		if (res)
			return res;
	}
	return NULL;
}

void JabberBrowser::checkDone()
{
    if (m_list->firstChild() && checkDone(m_list->firstChild()))
        stop(QString::null);
}

bool JabberBrowser::checkDone(ListViewItem *item)
{
	if (!item->text(COL_ID_DISCO_ITEMS).isEmpty() ||
			!item->text(COL_ID_DISCO_INFO).isEmpty() ||
			!item->text(COL_ID_BROWSE).isEmpty()){
		return false;
	}
	for(int c = 0; c < item->childCount(); c++)
	{
		ListViewItem *i= static_cast<ListViewItem*>(item->child(c));
		if (!checkDone(i))
			return false;
	}
	return true;
}

void JabberBrowser::setItemPict(ListViewItem *item)
{
    const char *name = "Jabber";
    QString category = item->text(COL_CATEGORY);
    QString type     = item->text(COL_TYPE);
    if (category == "headline"){
        name = "info";
    }else if (category == "directory"){
        name = "find";
    }else if (category == "conference"){
        name = "chat";
    }else if (category == "proxy"){
        name = "connect";
    }else if (type == "icq"){
        name = "ICQ";
    }else if (type == "aim"){
        name = "AIM";
    }else if (type == "msn"){
        name = "MSN";
    }else if (type == "yahoo"){
        name = "Yahoo!";
    }else if (type == "jud"){
        name = "find";
    }else if (type == "sms"){
        name = "sms";
    }else if ((type == "x-gadugadu") || (type == "gg")){
        name = "GG";
    }else if ((type == "rss") || (type == "weather")){
        name = "info";
    }
    item->setPixmap(COL_NAME, Pict(name));
}

void JabberBrowser::adjustColumn(ListViewItem *item)
{
    for (; item; item = static_cast<ListViewItem*>(item->parent())){
        if (item->isExpandable() && !item->isOpen())
            return;
    }
    m_list->adjustColumn();
}

void JabberBrowser::search()
{
}

void JabberBrowser::textChanged(const QString&)
{
}

void JabberWizard::layOutButtonRow(QHBoxLayout *layout){}
void JabberWizard::layOutTitleRow(QHBoxLayout *layout, const QString &title){}

#if 0

I18N_NOOP("Choose a user and password to register with the server");

#endif

/*
#ifndef NO_MOC_INCLUDES
#include "jabberbrowser.moc"
#endif
*/


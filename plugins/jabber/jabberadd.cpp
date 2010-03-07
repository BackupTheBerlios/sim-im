/***************************************************************************
                          jabberadd.cpp  -  description
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

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QShowEvent>

#include "icons.h"
#include "simgui/intedit.h"
#include "simgui/listview.h"
#include "misc.h"
#include "log.h"
#include "contacts/contact.h"

#include "jabberclient.h"
#include "jabberadd.h"
#include "jabber.h"
#include "jabbersearch.h"
#include "jabberbrowser.h"

using namespace std;
using namespace SIM;

const unsigned FILL_FIRST	= 0x0001;
const unsigned FILL_LAST	= 0x0002;
const unsigned FILL_NICK	= 0x0004;
const unsigned FILL_MAIL	= 0x0008;
const unsigned FILL_ALL		= (FILL_FIRST + FILL_LAST + FILL_NICK + FILL_MAIL);

JabberAdd::JabberAdd(JabberClient *client, QWidget *parent) : QWidget(parent)
        //: JabberAddBase(parent)
{
	setupUi(this);
    m_client   = client;
    m_browser  = NULL;
    m_bBrowser = false;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
    connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
    connect(grpJID,  SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(grpMail, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(grpName, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(btnBrowser, SIGNAL(clicked()), this, SLOT(browserClick()));
    QIcon is = Icon("1rightarrow");
    btnBrowser->setIcon(is);
}

JabberAdd::~JabberAdd()
{
    if (m_browser)
        delete m_browser;
}

void JabberAdd::browserDestroyed()
{
    m_browser = NULL;
}

void JabberAdd::radioToggled(bool)
{
    setBrowser(false);
	log(L_DEBUG, "JabberAdd::radioToggled() FIXMEEEE!!!!");
	/*
    if (isVisible())
        emit setAdd(grpJID->isChecked());
		*/
}

void JabberAdd::showEvent(QShowEvent *e)
{
	log(L_DEBUG, "JabberAdd::showEvent() FIXMEEEE!!!!");
    QWidget::showEvent(e);
		/*
    emit setAdd(grpJID->isChecked());
    if (m_browser && m_bBrowser)
        emit showResult(m_browser);
		*/
}

void JabberAdd::browserClick()
{
    setBrowser(!m_bBrowser);
}

void JabberAdd::setBrowser(bool bBrowser)
{
    if (m_bBrowser == bBrowser)
        return;
    m_bBrowser = bBrowser;
    if (m_bBrowser && (m_browser == NULL)){
        m_browser = new JabberBrowser;
        emit addResult(m_browser);
        m_browser->setClient(m_client);
        connect(m_browser, SIGNAL(destroyed()), this, SLOT(browserDestroyed()));
    }
    emit showResult(m_bBrowser ? m_browser : NULL);
    QIcon is = Icon(m_bBrowser ? "1leftarrow" : "1rightarrow");
    btnBrowser->setIcon(is);
    if (m_bBrowser){
        edtJID->setEnabled(false);
        edtMail->setEnabled(false);
        edtFirst->setEnabled(false);
        edtLast->setEnabled(false);
        edtNick->setEnabled(false);
        lblFirst->setEnabled(false);
        lblLast->setEnabled(false);
        lblNick->setEnabled(false);
        emit setAdd(false);
    }else{
		log(L_DEBUG, "JabberAdd::setBrowser() FIXMEEEE!!!!");
        //grpJID->slotToggled();
        //grpName->slotToggled();
        //grpMail->slotToggled();
    }
}

void JabberAdd::createContact(unsigned tmpFlags, Contact *&contact)
{
	log(L_DEBUG, "JabberAdd::createContact() FIXMEEEE!!!!");
	/*
    if (!grpJID->isChecked() || edtJID->text().isEmpty())
        return;
    QString resource;
    if (m_client->findContact(edtJID->text(), QString::null, false, contact, resource))
        return;
    QString name = edtJID->text();
    int n = name.indexOf('@');
    if (n > 0)
        name = name.left(n);
    m_client->findContact(edtJID->text(), name, true, contact, resource, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
	*/
}

void JabberAdd::search()
{
	log(L_DEBUG, "JabberAdd::search() FIXMEEEE!!!!");
	/*
    if (m_bBrowser)
        return;
    if (grpName->isChecked())
        searchName(edtFirst->text(), edtLast->text(), edtNick->text());
    if (grpMail->isChecked())
        searchMail(edtMail->text());
		*/
}

void JabberAdd::searchMail(const QString &mail)
{
    m_mail	= mail;
    m_first	= QString::null;
    m_last	= QString::null;
    m_nick	= QString::null;
    startSearch();
}

void JabberAdd::searchName(const QString &first, const QString &last, const QString &nick)
{
    m_first = first;
    m_last  = last;
    m_nick	= nick;
    m_mail	= QString::null;
    startSearch();
}

void JabberAdd::startSearch()
{
    m_disco_items.clear();
    m_fields.clear();
    m_labels.clear();
    m_agents.clear();
    m_nFields = 0;
    m_id_disco = QString::null;
    QString url;
    if (m_client->getUseVHost())
        url = m_client->getVHost();
    if (url.isEmpty())
        url = m_client->getServer();
    m_id_browse = m_client->browse(url);
}

void JabberAdd::addAttr(const QString &name, const QString &label)
{
    for (unsigned i = 0; i < m_fields.size(); i++){
        if (m_fields[i] == name)
            return;
    }
    m_fields.push_back(name);
    m_labels.push_back(label);
}

void JabberAdd::searchStop()
{
    m_id_browse = QString::null;
    m_id_disco  = QString::null;
    m_disco_items.clear();
    m_fields.clear();
    m_labels.clear();
    m_agents.clear();
    m_nFields = 0;
}

void JabberAdd::addAttrs()
{
    if (m_fields.size() <= m_nFields)
        return;
    QStringList attrs;
    for (; m_nFields < m_fields.size(); m_nFields++){
        attrs.append(m_fields[m_nFields]);
        attrs.append(m_labels[m_nFields]);
    }
    emit setColumns(attrs, 0, this);
}

bool JabberAdd::processEvent(Event *e)
{
    if (e->type() == eEventDiscoItem){
        EventDiscoItem *edi = static_cast<EventDiscoItem*>(e);
        DiscoItem *item = edi->item();
        if (m_id_browse == item->id){
            if (item->jid.isEmpty()){
                if (!item->node.isEmpty()){
                    QString url;
                    if (m_client->getUseVHost())
                        url = m_client->getVHost();
                    if (url.isEmpty())
                        url = m_client->getServer();
                    m_id_disco  = m_client->discoItems(url, QString::null);
                }
                m_id_browse = QString::null;
                checkDone();
                return true;
            }
            addSearch(item->jid, QString::null, item->features, item->type);
            return true;
        }
        if (m_id_disco == item->id){
            if (item->jid.isEmpty()){
                m_id_disco = QString::null;
                checkDone();
                return true;
            }
            ItemInfo info;
            info.jid  = item->jid;
            info.node = item->node;
            info.id   = m_client->discoInfo(info.jid, info.node);
            m_disco_items.push_back(info);
            return true;
        }
        list<ItemInfo>::iterator it;
        for (it = m_disco_items.begin(); it != m_disco_items.end(); ++it){
            if (it->id == item->id){
                addSearch(it->jid, it->node, item->features, item->type);
                m_disco_items.erase(it);
                checkDone();
                break;
            }
        }
    } else
    if (e->type() == eEventAgentInfo){
        EventAgentInfo *eai = static_cast<EventAgentInfo*>(e);
        JabberAgentInfo *data = eai->agentInfo();
        list<AgentSearch>::iterator it;
        for (it = m_agents.begin(); it != m_agents.end(); ++it)
            if (it->id_info == data->ReqID.str())
                break;
        if (it == m_agents.end())
            return false;
        if (data->Type.str().isEmpty()){
            it->id_info = QString::null;
            if (m_first.isEmpty())
                it->fill |= FILL_FIRST;
            if (m_last.isEmpty())
                it->fill |= FILL_LAST;
            if (m_nick.isEmpty())
                it->fill |= FILL_NICK;
            if (m_mail.isEmpty())
                it->fill |= FILL_MAIL;
            if (it->fill != FILL_ALL){
                m_agents.erase(it);
                checkDone();
                return true;
            }
            it->id_search = m_client->search(it->jid, it->node, it->condition);
            if (it->condition.left(6) != "x:data"){
                addAttr("", i18n("JID"));
                addAttr("first", i18n("First Name"));
                addAttr("last", i18n("Last Name"));
                addAttr("nick", i18n("Nick"));
                addAttr("email", i18n("E-Mail"));
                addAttrs();
            }
            return true;
        }
        if (data->Type.str() == "x"){
            it->condition = "x:data";
            it->fill = 0;
            return true;
        }
        QString value;
        QString field;
        if (!data->Field.str().isEmpty() &&
                (data->Type.str() == "text-single" ||
                 data->Type.str() == "text-private" ||
                 data->Type.str() == "text-multi")){
            field = data->Field.str();
            if ((data->Field.str() == "first") && !m_first.isEmpty()){
                value = m_first;
                it->fill |= FILL_FIRST;
            }
            if ((data->Field.str() == "last") && !m_last.isEmpty()){
                value = m_last;
                it->fill |= FILL_LAST;
            }
            if (((data->Field.str() == "nickname") || (data->Field.str() == "nick")) && !m_nick.isEmpty()){
                value = m_nick;
                it->fill |= FILL_NICK;
            }
            if ((data->Field.str() == "email") && !m_mail.isEmpty()){
                value = m_mail;
                it->fill |= FILL_MAIL;
            }
        }
        if ((data->Type.str() == "first") && !m_first.isEmpty()){
            field = data->Type.str();
            value = m_first;
            it->fill |= FILL_FIRST;
        }
        if ((data->Type.str() == "last") && !m_last.isEmpty()){
            field = data->Type.str();
            value = m_last;
            it->fill |= FILL_LAST;
        }
        if (((data->Type.str() == "nickname") || (data->Type.str() == "nick")) && !m_nick.isEmpty()){
            field = data->Type.str();
            value = m_nick;
            it->fill |= FILL_NICK;
        }
        if ((data->Type.str() == "email") && !m_mail.isEmpty()){
            field = data->Type.str();
            value = m_mail;
            it->fill |= FILL_MAIL;
        }
        if (!value.isEmpty()){
            if (!it->condition.isEmpty())
                it->condition += ';';
            it->condition += field;
            it->condition += '=';
            it->condition += quoteChars(value, ";");
        }
        return true;
    }
    if (e->type() == eEventJabberSearch){
        EventSearch *es = static_cast<EventSearch*>(e);
        JabberSearchData *data = es->searchData();
        list<AgentSearch>::iterator it;
        for (it = m_agents.begin(); it != m_agents.end(); ++it)
            if (it->id_search == data->ID.str())
                break;
        if (it == m_agents.end())
            return false;
        if (data->JID.str().isEmpty()){
            addAttr("", i18n("JID"));
            for (unsigned i = 0; i < data->nFields.toULong(); i++){
                addAttr(get_str(data->Fields, i * 2), get_str(data->Fields, i * 2 + 1));
                it->fields.push_back(get_str(data->Fields, i * 2));
            }
            addAttrs();
            return true;
        }
        QString icon = "Jabber";
        if (it->type == "icq"){
            icon = "ICQ";
        }else if (it->type == "aim"){
            icon = "AIM";
        }else if (it->type == "msn"){
            icon = "MSN";
        }else if (it->type == "yahoo"){
            icon = "Yahoo!";
        }else if (it->type == "sms"){
            icon = "sms";
        }else if ((it->type == "x-gadugadu") || (it->type == "gg")){
            icon = "GG";
        }
        if (!data->Status.str().isEmpty()){
            if (data->Status.str() == "online"){
                icon += "_online";
            }else{
                icon += "_offline";
            }
        }
        QStringList l;
        l.append(icon);
        l.append(data->JID.str());
        for (unsigned i = 0; i < m_fields.size(); i++){
            QString v;
            if (m_fields[i].isEmpty()){
                v = data->JID.str();
            }else if ((m_fields[i] == "first") && !data->First.str().isEmpty()){
                v = data->First.str();
            }else if ((m_fields[i] == "last") && !data->Last.str().isEmpty()){
                v = data->Last.str();
            }else if ((m_fields[i] == "nick") && !data->Nick.str().isEmpty()){
                v = data->Nick.str();
            }else if ((m_fields[i] == "email") && !data->EMail.str().isEmpty()){
                v = data->EMail.str();
            }else{
                for (unsigned n = 0; n < it->fields.size(); n++){
                    if (it->fields[n] == m_fields[i]){
                        v = get_str(data->Fields, n);
                        break;
                    }
                }
            }
            l.append(v);
        }
        emit addItem(l, this);
    } else
    if (e->type() == eEventJabberSearchDone){
        EventSearchDone *esd = static_cast<EventSearchDone*>(e);
        QString id = esd->userID();
        list<AgentSearch>::iterator it;
        for (it = m_agents.begin(); it != m_agents.end(); ++it)
            if (it->id_search == id)
                break;
        if (it == m_agents.end())
            return false;
        m_agents.erase(it);
        checkDone();
        return true;
    }
    return false;
}

void JabberAdd::addSearch(const QString &jid, const QString &node, const QString &features, const QString &type)
{
    if (features.isEmpty())
        return;
    QString f = features;
    while (!f.isEmpty()){
        QString feature = getToken(f, '\n');
        if (feature == "jabber:iq:search"){
            AgentSearch as;
            as.jid = jid;
            as.node = node;
            as.id_info = m_client->get_agent_info(jid, node, "search");
            as.fill = 0;
            as.type = type;
            m_agents.push_back(as);
            return;
        }
    }
}

void JabberAdd::checkDone()
{
    if (m_id_browse.isEmpty() && m_id_disco.isEmpty() &&
            m_disco_items.empty() && m_agents.empty())
        emit searchDone(this);
}

void JabberAdd::createContact(const QString &name, unsigned tmpFlags, Contact *&contact)
{
    QString resource;
    if (m_client->findContact(name, QString::null, false, contact, resource))
        return;
    if (m_client->findContact(name, QString::null, true, contact, resource, false) == NULL)
        return;
    contact->setFlags(contact->getFlags() | tmpFlags);
}

/*
#ifndef NO_MOC_INCLUDES
#include "jabberadd.moc"
#endif
*/


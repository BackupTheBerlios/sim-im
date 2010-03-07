/***************************************************************************
                          filter.cpp  -  description
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

#include <QRegExp>

#include "simgui/ballonmsg.h"
#include "unquot.h"
#include "msgedit.h"
#include "msgview.h"
#include "userwnd.h"

#include "profile.h"
#include "profilemanager.h"

#include "filter.h"
#include "filtercfg.h"
#include "contacts/contact.h"

using namespace SIM;

Plugin *createFilterPlugin(unsigned base, bool, Buffer *cfg)
{
    return new FilterPlugin(base, cfg);
}

static PluginInfo info =
    {
        I18N_NOOP("Filter"),
        I18N_NOOP("Plugin provides message filter"),
        VERSION,
        createFilterPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef filterUserData[] =
    {
        { "SpamList", DATA_UTF, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static FilterPlugin *filterPlugin = NULL;

static QWidget *getFilterConfig(QWidget *parent, PropertyHubPtr data)
{
    return new FilterConfig(parent, data, filterPlugin, false);
}

FilterPlugin::FilterPlugin(unsigned base, Buffer *cfg)
  : QObject(), Plugin(base)
  , EventReceiver(HighPriority - 1)
{
    filterPlugin = this;

    m_propertyHub = SIM::PropertyHub::create("filter");
    //load_data(filterData, &data, cfg);
    user_data_id = getContacts()->registerUserData(info.title, filterUserData);

    CmdIgnoreList	= registerType();
    CmdIgnore		= registerType();
    CmdIgnoreText	= registerType();

    Command cmd;
    cmd->id          = CmdIgnoreList;
    cmd->text        = I18N_NOOP("Ignore list");
    cmd->menu_id     = MenuContactGroup;
    cmd->menu_grp    = 0x8080;
    cmd->flags		 = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    cmd->id          = CmdIgnore;
    cmd->text        = I18N_NOOP("Ignore user");
    cmd->icon		 = "ignorelist";
    cmd->menu_id     = 0;
    cmd->menu_grp    = 0;
    cmd->bar_id		 = ToolBarContainer;
    cmd->bar_grp	 = 0x7001;
    cmd->flags		 = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    cmd->id          = CmdIgnoreText;
    cmd->text        = I18N_NOOP("Ignore this phrase");
    cmd->icon		 = QString::null;
    cmd->menu_id     = MenuTextEdit;
    cmd->menu_grp    = 0x7000;
    cmd->bar_id		 = 0;
    cmd->bar_grp	 = 0;
    cmd->flags		 = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    cmd->menu_id     = MenuMsgView;
    EventCommandCreate(cmd).process();

    cmd->id			 = user_data_id;
    cmd->text		 = I18N_NOOP("&Filter");
    cmd->icon		 = "filter";
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->param		 = (void*)getFilterConfig;
    EventAddPreferences(cmd).process();
}

FilterPlugin::~FilterPlugin()
{
    EventCommandRemove(CmdIgnoreList).process();
    EventRemovePreferences(user_data_id).process();
    getContacts()->unregisterUserData(user_data_id);
}

QByteArray FilterPlugin::getConfig()
{
    return QByteArray();
}

bool FilterPlugin::processEvent(Event *e)
{
    switch (e->type()) {
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            break;
        Contact *contact = ec->contact();
        if (contact->getGroup()){
            Command cmd;
            cmd->id		= CmdIgnore;
            cmd->flags	= BTN_HIDE;
            cmd->param  = (void*)(contact->id());
            EventCommandShow(cmd).process();
        }
        break;
    }
    case eEventPluginLoadConfig:
    {
        setPropertyHub( ProfileManager::instance()->getPropertyHub("filter") );
        break;
    }
    case eEventMessageReceived: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (!msg || (msg->type() == MessageStatus))
            return false;
        Contact *contact = getContacts()->contact(msg->contact());
        PropertyHubPtr data = contact->getUserData("filter");
        // check if we accept only from users on the list
        if (((contact == NULL) || contact->getFlags() & CONTACT_TEMPORARY) &&
                        ((value("FromList").toBool() &&
			  msg->type() != MessageAuthRequest &&
			  msg->type() != MessageAuthGranted &&
			  msg->type() != MessageAuthRefused) ||
                (value("AuthFromList").toBool() && msg->type() <= MessageContacts))) {
            delete msg;
            delete contact;
            return msg;
        }
        if (!contact)
            return false;
        // check if the user is a ignored user
        if (contact->getIgnore()){
            delete msg;
            return true;
        }

        // get filter-data
		if (data && !data->value("SpamList").toString().isEmpty() && (!contact || (contact->getFlags() & CONTACT_TEMPORARY) )) {
            if (checkSpam(msg->getPlainText(), data->value("SpamList").toString())){
                delete msg;
                return true;
            }
		}
        break;
    }
    case eEventCheckCommandState: {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdIgnore){
            cmd->flags &= ~BTN_HIDE;
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact && contact->getGroup())
                cmd->flags |= BTN_HIDE;
            return true;
        }
        if (cmd->id == CmdIgnoreText){
            cmd->flags &= ~COMMAND_CHECKED;
            if (cmd->menu_id == MenuMsgView){
                MsgViewBase *edit = (MsgViewBase*)(cmd->param);
                if (edit->textCursor().hasSelection())
                    return true;
            } else
            /*if (cmd->menu_id == MenuTextEdit){
                TextEdit *edit = ((MsgEdit*)(cmd->param))->m_edit;
                if (edit->textCursor().hasSelection())
                    return true;
            }*/							//Fixme Block (crashing on rightclick in msgedit from container)
            return false;
        }
        if (cmd->menu_id == MenuContactGroup){
            if (cmd->id == CmdIgnoreList){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return false;
                cmd->flags &= COMMAND_CHECKED;
                if (contact->getIgnore())
                    cmd->flags |= COMMAND_CHECKED;
                return true;
            }
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdIgnore){
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact){
                QString text = i18n("Add %1 to ignore list?") .arg(contact->getName());
                Command cmd;
                cmd->id		= CmdIgnore;
                cmd->param	= (void*)(contact->id());
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                QWidget *w = eWidget.widget();
                BalloonMsg::ask((void*)(contact->id()), text, w, SLOT(addToIgnore(void*)), NULL, NULL, this);
            }
            return true;
        }
        if (cmd->id == CmdIgnoreText){
            QString text;
            unsigned id = 0;
            if (cmd->menu_id == MenuMsgView){
                MsgViewBase *view = (MsgViewBase*)(cmd->param);
                if (view->textCursor().hasSelection()){
                    text = view->textCursor().selectedText();
                    text = unquoteText(text);
                    id = view->m_id;
                }
            }else if (cmd->menu_id == MenuTextEdit){
                MsgEdit *medit = (MsgEdit*)(cmd->param);
                TextEdit *edit = medit->m_edit;
                if (edit->textCursor().hasSelection()){
                    text = edit->textCursor().selectedText();
                    text = unquoteText(text);
                    id = medit->m_userWnd->id();
                }
            }
            
            Contact *contact = getContacts()->contact(id);
            PropertyHubPtr data = contact->getUserData("filter");

            QString s = data->value("SpamList").toString();
            while (!text.isEmpty()){
                QString line = getToken(text, '\n');
                line = line.remove('\r');
                if (line.isEmpty())
                    continue;
                bool bSpace = false;
                for (int i = 0; i < (int)(line.length()); i++)
                    if (line[i] == ' '){
                        bSpace = true;
                        break;
                    }
                if (bSpace)
                    line = '\"' + line + '\"';
                if (!s.isEmpty())
                    s += ' ';
                s += line;
            }
            data->setValue("SpamList", s);
            return false;
        }
        if (cmd->menu_id == MenuContactGroup)
        {
            if (cmd->id == CmdIgnoreList)
            {
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (!contact)
                    return false;
                contact->setIgnore((cmd->flags & COMMAND_CHECKED) == 0);
                EventContact(contact, EventContact::eChanged).process();
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

QWidget *FilterPlugin::createConfigWindow(QWidget *parent)
{
    PropertyHubPtr data = getContacts()->getUserData("filter");
    return new FilterConfig(parent, data, this, true);
}

static bool match(const QString &text, const QString &pat)
{
    int i;
    for (i = 0; (i < (int)(text.length())) && (i < (int)(pat.length())); i++){
        QChar c = pat[i];
        if (c == '?')
            continue;
        if (c == '*'){
            int n;
            for (n = i; n < (int)(pat.length()); n++)
                if (pat[n] != '*')
                    break;
            QString p = pat.mid(n);
            if (p.isEmpty())
                return true;
            for (n = i; n < (int)(text.length()); n++){
                QString t = text.mid(n);
                if (match(text, p))
                    return true;
            }
            return false;
        }
        if (text[i] != c)
            return false;
    }
    return (i == (int)(text.length())) && (i == (int)(pat.length()));
}

bool FilterPlugin::checkSpam(const QString &text, const QString &_filter)
{
    QString filter = _filter;
    QStringList wordsText;
    getWords(text, wordsText, false);
    bool bQuota = false;
    while (!filter.isEmpty()){
        QString filterPart = getToken(filter, '\"');
        QStringList wordsFilter;
        getWords(filterPart, wordsFilter, true);
        if (wordsFilter.count()){
            if (bQuota){
                for (QStringList::Iterator it = wordsText.begin(); it != wordsText.end(); ++it){
                    if (!match(*it, wordsFilter[0]))
                        continue;
                    QStringList::Iterator it1 = it;
                    QStringList::Iterator itFilter = wordsFilter.begin();
                    for (; (it1 != wordsText.end()) && (itFilter != wordsFilter.end()); ++it1, ++itFilter){
                        if (!match(*it1, *itFilter))
                            break;
                    }
                    if (itFilter == wordsFilter.end())
                        return true;
                }
            }else{
                for (QStringList::Iterator it = wordsText.begin(); it != wordsText.end(); ++it){
                    for (QStringList::Iterator itFilter = wordsFilter.begin(); itFilter != wordsFilter.end(); ++itFilter){
                        if (match(*it, *itFilter))
                            return true;
                    }
                }
            }
        }
        bQuota = !bQuota;
    }
    return false;
}

void FilterPlugin::getWords(const QString &text, QStringList &words, bool bPattern)
{
    QString word;
    for (int i = 0; i < (int)(text.length()); i++){
        QChar c = text[i];
        if (c.isLetterOrNumber()){
            word += c;
            continue;
        }
        if (bPattern && ((c == '?') || (c == '*'))){
            word += c;
            continue;
        }
        if (word.isEmpty())
            continue;
        words.append(word);
        word = QString::null;
    }
    if (!word.isEmpty())
        words.append(word);
}

void FilterPlugin::addToIgnore(void *p)
{
    Contact *contact = getContacts()->contact((unsigned long)p);
    if (contact && !contact->getIgnore()){
        contact->setIgnore(true);
        EventContact(contact, EventContact::eChanged).process();
    }
}

void FilterPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr FilterPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant FilterPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void FilterPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

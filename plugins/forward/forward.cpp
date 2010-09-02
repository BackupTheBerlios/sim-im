/***************************************************************************
                          forward.cpp  -  description
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

#include "forward.h"
#include "forwardcfg.h"
#include "core.h"

#include "profile.h"
#include "profilemanager.h"

#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/client.h"

using namespace SIM;

Plugin *createForwardPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new ForwardPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Forward"),
        I18N_NOOP("Plugin provides messages forwarding on cellular"),
        VERSION,
        createForwardPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef forwardUserData[] =
    {
        { "Phone", DATA_UTF, 1, 0 },
        { "Send1st", DATA_BOOL, 1, 0 },
        { "Translit", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static ForwardPlugin *forwardPlugin = NULL;

static QWidget *getForwardSetup(QWidget *parent, PropertyHubPtr data)
{
    return new ForwardConfig(parent, data, forwardPlugin);
}

ForwardPlugin::ForwardPlugin(unsigned base)
    : QObject(), Plugin(base)
    , EventReceiver(DefaultPriority - 1)
{
    m_propertyHub = SIM::PropertyHub::create("forward");
    forwardPlugin = this;
    user_data_id = getContacts()->registerUserData(info.title, forwardUserData);
    Command cmd;
    cmd->id		  = user_data_id;
    cmd->text	  = I18N_NOOP("&Forward");
    cmd->icon	  = "cell";
    cmd->param	 = (void*)getForwardSetup;
    EventAddPreferences(cmd).process();
}

ForwardPlugin::~ForwardPlugin()
{
    EventRemovePreferences(user_data_id).process();
    getContacts()->unregisterUserData(user_data_id);
}

bool ForwardPlugin::processEvent(Event *e)
{
    if (e->type() == eEventMessageReceived){
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->type() == MessageStatus)
            return false;
        QString text = msg->getPlainText();
        if (text.isEmpty())
            return false;
        if (msg->type() == MessageSMS){
            SMSMessage *sms = static_cast<SMSMessage*>(msg);
            QString phone = sms->getPhone();
            bool bMyPhone;
            SIM::PropertyHubPtr data = getContacts()->getUserData("forward");
            bMyPhone = ContactList::cmpPhone(phone, data->value("Phone").toString());
            if (!bMyPhone){
                Group *grp;
                ContactList::GroupIterator it;
                while ((grp = ++it) != NULL){
                    data = grp->getUserData("forward", false);
                    if (data && !data->value("Phone").toString().isEmpty()){
                        bMyPhone = ContactList::cmpPhone(phone, data->value("Phone").toString());
                        break;
                    }
                }
            }
            if (!bMyPhone){
                Contact *contact;
                ContactList::ContactIterator it;
                while ((contact = ++it) != NULL){
                    data = contact->getUserData("forward", false);
                    if (data && !data->value("Phone").toString().isEmpty())
                    {
                        bMyPhone = ContactList::cmpPhone(phone, data->value("Phone").toString());
                        break;
                    }
                }
            }
            if (bMyPhone){
                int n = text.indexOf(": ");
                if (n > 0){
                    QString name = text.left(n);
                    QString msg_text = text.mid(n + 2);
                    Contact *contact;
                    ContactList::ContactIterator it;
                    while ((contact = ++it) != NULL){
                        if (contact->getName() == name){
                            Message *msg = new Message(MessageGeneric);
                            msg->setContact(contact->id());
                            msg->setText(msg_text);
                            void *data;
                            ClientDataIterator it(contact->clientData);
                            while ((data = ++it) != NULL){
                                if (it.client()->send(msg, data))
                                    break;
                            }
                            if (data == NULL)
                                delete msg;
                            return true;
                        }
                    }
                }
            }
        }
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            return false;
        SIM::PropertyHubPtr data = contact->getUserData("forward");
        if (!data || data->value("Key").toString().isEmpty())
            return false;
        CorePlugin *core = GET_CorePlugin();
        unsigned status = core->getManualStatus();
        if ((status == STATUS_AWAY) || (status == STATUS_NA)){
            text = contact->getName() + ": " + text;
            unsigned flags = MESSAGE_NOHISTORY;
            if (data->value("Send1st").toBool())
                flags |= MESSAGE_1ST_PART;
            if (data->value("Translit").toBool())
                flags |= MESSAGE_TRANSLIT;
            SMSMessage *m = new SMSMessage;
            m->setPhone(data->value("Phone").toString());
            m->setText(text);
            m->setFlags(flags);
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (client->send(m, NULL))
                    break;
            }
            if (i >= getContacts()->nClients())
                delete m;
        }
    }
    return false;
}

QWidget *ForwardPlugin::createConfigWindow(QWidget *parent)
{
    return new ForwardConfig(parent, getContacts()->getUserData("forward"), this);
}

void ForwardPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr ForwardPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant ForwardPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void ForwardPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}
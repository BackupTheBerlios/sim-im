/***************************************************************************
    sound.cpp  -  description
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

#include <QDir>
#include "simapi.h"

#include "exec.h"
#include "log.h"
#include "core.h"

#include "sound.h"
#include "soundconfig.h"
#include "sounduser.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "profile.h"
#include "profilemanager.h"

using namespace std;
using namespace SIM;

const unsigned CHECK_SOUND_TIMEOUT	= 200;
const unsigned WAIT_SOUND_TIMEOUT	= 1000;

Plugin *createSoundPlugin(unsigned base, bool bFirst, Buffer *config)
{
    return new SoundPlugin(base, bFirst, config);
}

Plugin *createSoundPluginObject()
{
    return new SoundPlugin(0, 0, 0);
}

static PluginInfo info =
{
    I18N_NOOP("Sound"),
    I18N_NOOP("Plugin provides sounds on any events"),
    VERSION,
    createSoundPlugin,
    PLUGIN_DEFAULT,
    createSoundPluginObject
};

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static SoundPlugin *soundPlugin = NULL;

static QWidget *getSoundSetup(QWidget *parent, SIM::PropertyHubPtr data)
{
    return new SoundUserConfig(parent, data, soundPlugin);
}

SoundPlugin::SoundPlugin(unsigned base, bool bFirst, Buffer *config)
: QObject(), Plugin(base)
{
    m_propertyHub = SIM::PropertyHub::create("sound");
    soundPlugin = this;
    m_media = Phonon::createPlayer(Phonon::NotificationCategory);

    CmdSoundDisable = 1000022; // FIXME

    Command cmd;
    cmd->id       = 0;
    cmd->flags    = COMMAND_CONTACT;
    cmd->text	  = I18N_NOOP("&Sound");
    cmd->icon	  = "sound";
    cmd->accel    = "sound";
    cmd->icon_on  = QString();
    cmd->param	  = (void*)getSoundSetup;
    EventAddPreferences(cmd).process();

    cmd->id       = CmdSoundDisable;
    cmd->text	  = I18N_NOOP("&Sound");
    cmd->icon	  = "nosound";
    cmd->icon_on  = "sound";
    cmd->bar_id   = ToolBarMain;
    cmd->bar_grp  = 0;
    cmd->menu_id  = 0;
    cmd->menu_grp = 0;
    cmd->flags	  = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();
}

SoundPlugin::~SoundPlugin()
{
    soundPlugin = NULL;
}

QWidget *SoundPlugin::createConfigWindow(QWidget *parent)
{
    return new SoundConfig(parent, this);
}

bool SoundPlugin::processEvent(SIM::Event *e)
{
    switch (e->type())
    {
    case eEventLoginStart:
        {
            playSound(value("StartUp").toString());
            break;
        }
    case eEventPluginLoadConfig:
        {
            PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("sound");
            if(!hub.isNull())
                setPropertyHub(hub);
            if(!value("StartUp").isValid())
                setValue("StartUp", "sounds/startup.ogg");
            if(!value("MessageSent").isValid())
                setValue("MessageSent", "sounds/msgsent.ogg");
            if(!value("FileDone").isValid())
                setValue("FileDone", "sounds/filedone.ogg");
            break;
        }
    case eEventContact:
        {
            EventContact *ec = static_cast<EventContact*>(e);
            if(ec->action() != EventContact::eOnline)
                break;
            Contact *contact = ec->contact();
            bool disable = contact->getUserData()->root()->value("sound/Disable").toBool();
            QString alert = contact->getUserData()->root()->value("sound/Alert").toString();
            if(alert.isEmpty())
                alert = getContacts()->userdata()->value("sound/Alert").toString();
            if (!alert.isEmpty() && !disable)
                EventPlaySound(alert).process();
            break;
        }
    case eEventMessageSent:
        {
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            QString err = msg->getError();
            if (!err.isEmpty())
                return false;
            QString sound;
            if (msg->type() == MessageFile)
                sound = value("FileDone").toString();
            else if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0)
            {
                if ((msg->getFlags() & MESSAGE_MULTIPLY) && ((msg->getFlags() & MESSAGE_LAST) == 0))
                    return false;
                sound = value("MessageSent").toString();
            }
            if (!sound.isEmpty())
                EventPlaySound(sound).process();
            break;
        }
    case eEventMessageReceived:
        {
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            if(msg->type() == MessageStatus)
                return false;
            Contact *contact = getContacts()->contact(msg->contact());
            bool nosound, disable;
            if(contact)
            {
                nosound = contact->getUserData()->root()->value("sound/NoSoundIfActive").toBool();
                disable = contact->getUserData()->root()->value("sound/Disable").toBool();
            }
            else
            {
                nosound = getContacts()->userdata()->value("sound/NoSoundIfActive").toBool();
                disable = getContacts()->userdata()->value("sound/Disable").toBool();
            }
            if(!disable && nosound)
            {
                EventActiveContact e;
                e.process();
                if (e.contactID() == contact->id())
                    disable = true;
            }
            if(!disable)
            {
                QString sound = messageSound(msg->baseType(), contact->id());
                playSound(sound);
            }
            break;
        }
    case eEventPlaySound:
        {
            EventPlaySound *s = static_cast<EventPlaySound*>(e);
            playSound(s->sound());
            return true;
        }
    default:
        break;
    }
    return false;
}

void SoundPlugin::playSound(const QString& path)
{
    QString snd;
    log(L_DEBUG, "Sound: %s", qPrintable(path));
    QDir d(path);
    if(d.isRelative())
        snd = app_file(path);
    else
        snd = path;
    m_media->setCurrentSource(Phonon::MediaSource(snd));
    Phonon::State state = m_media->state();
    if( state == Phonon::ErrorState ) {
        QString sError = m_media->errorString();
        log(L_DEBUG, "Sound playing error: %s", qPrintable(sError));
        return;
    }
    m_media->play();
}

QString SoundPlugin::messageSound(unsigned type, unsigned long contact_id)
{
    SIM::PropertyHubPtr data;
    Contact *c = getContacts()->contact(contact_id);
    if(!contact_id)
        data = getContacts()->userdata();
    else if(c)
    {
        data = c->getUserData()->root();
        if(!data->value("sound/override").toBool())
        {
            Group* g = getContacts()->group(c->getGroup(), false);
            if(g->userdata()->value("sound/override").toBool())
                data = g->userdata();
            else
                data = getContacts()->userdata();
        }
    }
    QString sound;
    if(data)
        sound = data->value("sound/Receive" + QString::number(type)).toString();
    if(sound == "(nosound)")
        return QString();
    return sound;
}

void SoundPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr SoundPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant SoundPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void SoundPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

// vim : expandtab


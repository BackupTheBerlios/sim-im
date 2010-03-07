/***************************************************************************
                          sound.h  -  description
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

#ifndef _SOUND_H
#define _SOUND_H

#include "simapi.h"

#include "cfg.h"
#include "event.h"
#include "plugins.h"
#include "propertyhub.h"

#include <QObject>
#include <QProcess>
#include <QByteArray>
#include "phonon/mediaobject.h"
#include "phonon/audiooutput.h"


class CorePlugin;
class QTimer;
class QSound;

class SoundPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    SoundPlugin(unsigned, bool, Buffer*);
    virtual ~SoundPlugin();

	void playSound(const QString& path);

    SIM::SIMEvent EventSoundChanged;

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
protected:
	QString messageSound(unsigned type, unsigned long contact_id);
    virtual bool processEvent(SIM::Event *e);
    virtual QWidget *createConfigWindow(QWidget *parent);
    friend class SoundConfig;
    friend class SoundUserConfig;

private:
	Phonon::MediaObject* m_media;
	unsigned long CmdSoundDisable;
    SIM::PropertyHubPtr m_propertyHub;

};

#endif


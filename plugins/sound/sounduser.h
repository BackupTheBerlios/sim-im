/***************************************************************************
                          sounduser.h  -  description
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

#ifndef _SOUNDUSER_H
#define _SOUNDUSER_H

#include "ui_sounduserbase.h"
#include "propertyhub.h"

class SoundPlugin;

class SoundUserConfig : public QWidget, public Ui::SoundUserConfigBase
{
    Q_OBJECT
public:
    SoundUserConfig(QWidget *parent, SIM::PropertyHubPtr data, SoundPlugin *plugin);

public slots:
    void apply(SIM::PropertyHubPtr data, bool override);
    void toggled(bool);

protected:
    void resizeEvent(QResizeEvent*);

private:
    SoundPlugin* m_plugin;
};

#endif


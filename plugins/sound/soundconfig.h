/***************************************************************************
                          soundconfig.h  -  description
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

#ifndef _SOUNDCONFIG_H
#define _SOUNDCONFIG_H

#include "ui_soundconfigbase.h"

class SoundUserConfig;
class SoundPlugin;

class SoundConfig : public QWidget, public Ui::SoundConfigBase
{
    Q_OBJECT
public:
    SoundConfig(QWidget *parent, SoundPlugin *plugin);
    ~SoundConfig();
signals:
    void addTab(const char *text, QWidget *w);
public slots:
    void apply();
protected slots:
    void artsToggled(bool);
protected:
    SoundPlugin		*m_plugin;
    SoundUserConfig *user_cfg;
};

#endif


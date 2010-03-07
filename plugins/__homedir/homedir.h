/***************************************************************************
                          homedir.h  -  description
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

#ifndef _HOMEDIR_H
#define _HOMEDIR_H

#include "event.h"
#include "plugins.h"

class HomeDirPlugin : public SIM::Plugin, public SIM::EventReceiver
{
public:
    HomeDirPlugin(unsigned base);
    QString defaultPath();
protected:
    bool processEvent(SIM::Event *e);
    QString buildFileName(const QString &name);
#ifdef WIN32
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QByteArray getConfig();
    friend class HomeDirConfig;

    bool m_bDefault;
    bool m_bSave;
#endif
    QString m_homeDir;
};

#endif


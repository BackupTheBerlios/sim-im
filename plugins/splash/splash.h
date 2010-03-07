/***************************************************************************
                          splash.h  -  description
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

#ifndef _SPLASH_H
#define _SPLASH_H

#include <QTimer>

#include "event.h"
#include "plugins.h"

class QWidget;

class SplashPlugin : public QObject, public SIM::Plugin
{
    Q_OBJECT
public:
    SplashPlugin(unsigned base, bool bStart);
    virtual ~SplashPlugin();

protected:
    
    QWidget *splash;
    bool m_bStart;
    QTimer		*m_timer;
protected slots:
    void timeout();

};

#endif


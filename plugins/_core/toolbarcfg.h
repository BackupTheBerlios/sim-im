/***************************************************************************
                          toolbarcfg.h  -  description
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

#ifndef _TOOLBARCFG_H
#define _TOOLBARCFG_H

#include "qobject.h"

#include <QEvent>

class ToolbarsCfg : public QObject
{
    Q_OBJECT
public:
    ToolbarsCfg();
    virtual ~ToolbarsCfg();
protected slots:
    void popupActivated();
protected:
    bool eventFilter(QObject *o, QEvent *e);
};

#endif


/***************************************************************************
                          toolbarcfg.cpp  -  description
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

#include "misc.h"

#include "toolbarcfg.h"

#include <QApplication>
#include <QMenu>
#include <QEvent>

ToolbarsCfg::ToolbarsCfg()
{
    qApp->installEventFilter(this);
}

ToolbarsCfg::~ToolbarsCfg()
{
}

bool ToolbarsCfg::eventFilter(QObject *o, QEvent *e)
{
    if((e->type() == QEvent::Show) && o->inherits("CMenu"))
	{
        QObject *parent = o->parent();
        if(parent && (parent->inherits("MainWindow") || parent->inherits("CToolBar")))
        {
            QMenu *popup = static_cast<QMenu*>(o);
            popup->addAction(i18n("Customize toolbar..."), this, SLOT(popupActivated()));
        }
    }
    return QObject::eventFilter(o, e);
}

void ToolbarsCfg::popupActivated()
{
}


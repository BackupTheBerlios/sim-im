/***************************************************************************
                          QChildWidget  -  description
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

#ifndef _QCHILDWIDGET_H
#define _QCHILDWIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QChildEvent>
#include <QEvent>

#include "simapi.h"

class EXPORT QChildWidget : public QWidget
{
    Q_OBJECT
public:
    QChildWidget(QWidget *parent, const char *name = NULL);
    virtual void childEvent(QChildEvent*);
    virtual void paintEvent(QPaintEvent*);
    virtual bool eventFilter(QObject*, QEvent*);
    QRect rcChild;
    bool m_bInit;
};

#endif


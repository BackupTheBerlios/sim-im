/***************************************************************************
                          qkeybutton.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _QKEYBUTTON_H_
#define _QKEYBUTTON_H_

#include "simapi.h"

#include <QPushButton>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>

class EXPORT QKeyButton: public QPushButton
{
    Q_OBJECT
public:
    QKeyButton( QWidget *parent = 0, const char *name = 0 );
signals:
    void changed();
protected slots:
    void click();
protected:
    void keyPressEvent(QKeyEvent*);
    void keyReleaseEvent(QKeyEvent*);
    void focusInEvent(QFocusEvent*);
    void focusOutEvent(QFocusEvent*);
    void mousePressEvent(QMouseEvent*);
    void startGrab();
    void endGrab();
    void setKey(QKeyEvent*, bool bPress);
    bool m_bGrab;
};

#endif

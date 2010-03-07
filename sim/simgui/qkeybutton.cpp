/***************************************************************************
                          qkeybutton.cpp  -  description
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

#include "qkeybutton.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QCursor>
#include <QStringList>
#include <QKeySequence>

#include "log.h"



QKeyButton::QKeyButton(QWidget *parent, const char *name)
        :	QPushButton( parent )
{
    setObjectName( name );
    m_bGrab = false;
    connect(this, SIGNAL(clicked()), this, SLOT(click()));
}

void QKeyButton::focusInEvent(QFocusEvent *e)
{
    QPushButton::focusInEvent(e);
    startGrab();
}

void QKeyButton::focusOutEvent(QFocusEvent *e)
{
    endGrab();
    QPushButton::focusOutEvent(e);
}

void QKeyButton::startGrab()
{
    if (m_bGrab) return;
    m_bGrab = true;
    grabKeyboard();
    grabMouse(QCursor(Qt::IBeamCursor));
}

void QKeyButton::endGrab()
{
    if (!m_bGrab) return;
    m_bGrab = false;
    releaseKeyboard();
    releaseMouse();
}

void QKeyButton::click()
{
    if (hasFocus()) {
        clearFocus();
    } else {
        setFocus();
    }
}

void QKeyButton::mousePressEvent(QMouseEvent *e)
{
    QPushButton::mousePressEvent(e);
    endGrab();
}

void QKeyButton::keyPressEvent(QKeyEvent *e)
{
    setKey(e, true);
}

void QKeyButton::keyReleaseEvent(QKeyEvent *e)
{
    setKey(e, false);
}

void QKeyButton::setKey(QKeyEvent *e, bool bPress)
{
    if (!m_bGrab) return;
    QStringList btns;
    Qt::KeyboardModifiers state = e->modifiers();
    Qt::KeyboardModifiers key_state = 0;
    QString keyName;
    QString name;
    SIM::log(SIM::L_DEBUG, "-> %X %X", e->key(), (unsigned)e->modifiers());
    switch (e->key()){
    case Qt::Key_Shift:
        key_state = Qt::ShiftModifier;
        break;
    case Qt::Key_Control:
        key_state = Qt::ControlModifier;
        break;
    case Qt::Key_Alt:
        key_state = Qt::AltModifier;
        break;
    case Qt::Key_Meta:
        key_state = Qt::MetaModifier;
        break;
    default:
        name = (QString)QKeySequence(e->key());
        if ((name[0] == '<') && (name[(int)(name.length()-1)] == '>'))
            return;
    }
    if (bPress){
        state |= key_state;
    }else{
        state &= ~key_state;
    }
    if (state & Qt::AltModifier) keyName += "Alt+";
    if (state & Qt::ControlModifier) keyName += "Ctrl+";
    if (state & Qt::ShiftModifier) keyName += "Shift+";
    if (state & Qt::MetaModifier) keyName += "Meta+";
    setText(keyName + name);
    if (name.length()){
        endGrab();
        emit changed();
    }
}

/*
#ifndef NO_MOC_INCLUDES
#include "qkeybutton.moc"
#endif
*/


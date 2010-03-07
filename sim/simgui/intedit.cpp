/***************************************************************************
                          intedit.cpp  -  description
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

#include "simgui/intedit.h"
#include <QKeyEvent>

IntLineEdit::IntLineEdit(QWidget *parent)
    : QLineEdit(parent), id(0)
{}

void IntLineEdit::keyPressEvent(QKeyEvent *e)
{
    QLineEdit::keyPressEvent(e);
    if (e->key() == Qt::Key_Escape)
        emit escape();
}

EMailValidator::EMailValidator(QWidget *parent)
    : QRegExpValidator(QRegExp("[A-Za-z0-9\\.\\-_\\+]+@[A-Za-z0-9\\-_]+\\.[A-Za-z0-9\\.\\-_]+"), parent)
{}

PhoneValidator::PhoneValidator(QWidget *parent)
    : QRegExpValidator(QRegExp("\\+?[0-9 ]+(\\([0-9]+\\))?([0-9 ]+\\-)*[0-9 ]+"), parent)
{}

RadioGroup::RadioGroup(QWidget *parent)
    : QGroupBox(parent)
    , m_bLock( false )
{
    setCheckable( true );
    setChecked( false );
    connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
}

RadioGroup::~RadioGroup()
{
}

void RadioGroup::slotToggled( bool checked )
{
    if( m_bLock )
        return;

    if( !checked ) {
        setChecked( true );
        return;
    }

    QList<RadioGroup*> l = parentWidget()->findChildren<RadioGroup*>();
    foreach( RadioGroup *obj, l ) {
        if( obj != this ) {
            obj->setLock( true );
            obj->setChecked( !checked );
            obj->setLock( false );
        }
    }
}

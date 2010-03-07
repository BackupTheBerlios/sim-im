/***************************************************************************
                          intedit.h  -  description
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

#ifndef _INTEDIT_H
#define _INTEDIT_H

#include "simapi.h"

#include <QLineEdit>
#include <QGroupBox>
#include <QValidator>

class EXPORT IntLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    IntLineEdit(QWidget *parent);
    unsigned id;
signals:
    void escape();
protected:
    void keyPressEvent(QKeyEvent*);
};

class EXPORT EMailValidator : public QRegExpValidator
{
public:
    EMailValidator(QWidget *parent);
};

class EXPORT PhoneValidator : public QRegExpValidator
{
public:
    PhoneValidator(QWidget *parent);
};

class EXPORT RadioGroup : public QGroupBox
{
    Q_OBJECT
public:
    RadioGroup(QWidget *parent);
    virtual ~RadioGroup();
protected:
    void setLock( bool bLock ) { m_bLock = bLock; }
    bool m_bLock;

protected slots:
    void slotToggled( bool checked );
};

#endif


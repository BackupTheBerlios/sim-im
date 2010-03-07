/***************************************************************************
                          nonim.h  -  description
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

#ifndef _NONIM_H
#define _NONIM_H

#include "ui_nonimbase.h"
#include "event.h"

#include <QShowEvent>
#include <QWidget>

class NonIM : public QWidget, public Ui::NonIM
{
    Q_OBJECT
public:
    NonIM(QWidget *parent);
signals:
    void setAdd(bool);
    void showError(const QString&);
protected slots:
    void add(SIM::Contact *&contact);
    void createContact(unsigned tmpFlags, SIM::Contact *&contact);
protected:
    void showEvent(QShowEvent*);
};

#endif


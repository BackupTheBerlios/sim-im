/***************************************************************************
                          arcfg.h  -  description
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

#ifndef _ARCFG_H
#define _ARCFG_H

#include <QWidget>
#include "contacts.h"

#include "ui_arcfgbase.h"

class ARConfig : public QWidget, public Ui::ARConfigBase
{
    Q_OBJECT
public:
    ARConfig(QWidget *parent, unsigned status, const QString &name, SIM::Contact *contact);
public slots:
    void apply();
    void applyGlobal();

    void applyForSpecialUser();

    void toggled(bool);
    void help();
protected:
    unsigned m_status;
    SIM::Contact *m_contact;
};

#endif


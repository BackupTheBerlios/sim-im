/***************************************************************************
                          editphone.h  -  description
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

#ifndef _EDITPHONE_H
#define _EDITPHONE_H

#include "ui_editphonebase.h"

class PhoneDetails;
class PagerDetails;

class EditPhone : public QDialog, public Ui::EditPhone
{
    Q_OBJECT
public:
    EditPhone(QWidget *parent, const QString &number, const QString &type, unsigned icon, bool bPublish, bool bShowPublish);
    QString number;
    QString type;
    unsigned icon;
    bool publish;
protected slots:
    void numberChanged(const QString&, bool);
    void nameChanged(const QString&);
    void typeChanged(int);
protected:
    void accept();
    void changed();
    bool m_ok;
    PhoneDetails *m_phone;
    PagerDetails *m_pager;
};

#endif


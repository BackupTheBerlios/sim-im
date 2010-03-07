/***************************************************************************
                          phonedetails.h  -  description
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

#ifndef _PHONEDETAILS_H
#define _PHONEDETAILS_H

#include "ui_phonebase.h"
#include "event.h"

class PhoneDetails : public QWidget, public Ui::PhoneDetailsBase
{
    Q_OBJECT
public:
    PhoneDetails(QWidget *p, const QString &oldNumber);
    void setExtensionShow(bool bShow);
    void getNumber();
signals:
    void numberChanged(const QString &str, bool bOK);
protected slots:
    void countryChanged(int);
    void textChanged(const QString&);
protected:
    bool m_bExt;
};

#endif


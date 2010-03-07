/***************************************************************************
                          editmail.h  -  description
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

#ifndef _EDITMAIL_H
#define _EDITMAIL_H

#include "ui_editmailbase.h"

class EditMail : public QDialog, public Ui::EditMailBase
{
    Q_OBJECT
public:
    EditMail(QWidget *parent, const QString &mail, bool bPublish, bool bShowPublish);
    QString res;
    bool publish;
protected slots:
    void textChanged(const QString&);
protected:
    void accept();
};

#endif


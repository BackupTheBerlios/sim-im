/***************************************************************************
                          verifydlg.h  -  description
                             -------------------
    begin                : Sun Nov 27 2005
    copyright            : (C) 2005 by Andrey Rahmatullin
    email                : wrar@altlinux.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VERIFYDLG_H
#define _VERIFYDLG_H

#include "event.h"
#include "ui_verifydlgbase.h"
#include <QLineEdit>
#include <QPixmap>

class QPixmap;

class VerifyDlg : public QDialog, public Ui::VerifyDlgBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    VerifyDlg(QWidget *parent, const QPixmap& picture);
    ~VerifyDlg();
    const QString getVerifyString() const { return edtVerify->text(); };
protected slots:
    void changed();
    void changed(const QString& text);
};

#endif


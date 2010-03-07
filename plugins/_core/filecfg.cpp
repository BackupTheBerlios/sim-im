/***************************************************************************
                          filecfg.cpp  -  description
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

#include "filecfg.h"
#include "simgui/editfile.h"
#include "smscfg.h"
#include "core.h"

#include <QCheckBox>
#include <QRadioButton>

using namespace SIM;

FileConfig::FileConfig(QWidget *parent, SIM::PropertyHubPtr data)
  : QWidget(parent)
{
    setupUi(this);
    edtPath->setDirMode(true);
    edtPath->setText(user_file(data->value("IncomingPath").toString()));
    switch (data->value("AcceptMode").toUInt())
    {
    case 0:
        btnDialog->setChecked(true);
        break;
    case 1:
        btnAccept->setChecked(true);
        chkOverwrite->setEnabled(true);
        break;
    case 2:
        btnDecline->setChecked(true);
        edtDecline->setEnabled(true);
        break;
    }
    chkOverwrite->setChecked(data->value("OverwriteFiles").toBool());
    edtDecline->setPlainText(data->value("DeclineMessage").toString());
}

void FileConfig::apply(SIM::PropertyHubPtr data)
{
    QString def = edtPath->text().isEmpty() ? "Incoming Files" : edtPath->text();
    data->setValue("IncomingPath", def);
    edtPath->setText(user_file(data->value("IncomingPath").toString()));
    data->setValue("AcceptMode", 0);
    if (btnAccept->isChecked())
    {
        data->setValue("AcceptMode", 1);
        data->setValue("OverwriteFiles", chkOverwrite->isChecked());
    }
    if (btnDecline->isChecked())
    {
        data->setValue("AcceptMode", 2);
        data->setValue("DeclineMessage", edtDecline->toPlainText());
    }
}

// vim: set expandtab:


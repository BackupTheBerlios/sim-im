/***************************************************************************
                          smscfg.cpp  -  description
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

#include "smscfg.h"
#include "core.h"

#include <QCheckBox>

SMSConfig::SMSConfig(QWidget *parent, SIM::PropertyHubPtr data) : QWidget(parent)
{
	setupUi(this);
    edtBefore->setPlainText(data->value("SMSSignatureBefore").toString());
    edtAfter->setPlainText(data->value("SMSSignatureAfter").toString());
}

void SMSConfig::apply(SIM::PropertyHubPtr data)
{
    data->setValue("SMSSignatureBefore", edtBefore->toPlainText());
    data->setValue("SMSSignatureAfter", edtAfter->toPlainText());
}

// vim: set expandtab:


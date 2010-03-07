/***************************************************************************
                          userhistorycfg.cpp  -  description
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

#include "userhistorycfg.h"
#include "core.h"

#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

UserHistoryCfg::UserHistoryCfg(QWidget *parent, SIM::PropertyHubPtr data) : QWidget(parent)
        //: UserHistoryCfgBase(parent)
{
	setupUi(this);
    chkDays->setChecked(data->value("CutDays").toBool());
    chkSize->setChecked(data->value("CutSize").toBool());
    edtDays->setValue(data->value("Days").toUInt());
    edtSize->setValue(data->value("MaxSize").toUInt());
    toggledDays(chkDays->isChecked());
    toggledSize(chkSize->isChecked());
    connect(chkDays, SIGNAL(toggled(bool)), this, SLOT(toggledDays(bool)));
    connect(chkSize, SIGNAL(toggled(bool)), this, SLOT(toggledSize(bool)));
}

UserHistoryCfg::~UserHistoryCfg()
{
}

void UserHistoryCfg::apply(SIM::PropertyHubPtr data)
{
    data->setValue("CutDays", chkDays->isChecked());
    data->setValue("CutSize", chkSize->isChecked());
    data->setValue("Days", edtDays->text().toUInt());
    data->setValue("MaxSize", edtSize->text().toUInt());
}

void UserHistoryCfg::toggledDays(bool bState)
{
    lblDays->setEnabled(bState);
    lblDays1->setEnabled(bState);
    edtDays->setEnabled(bState);
}

void UserHistoryCfg::toggledSize(bool bState)
{
    lblSize->setEnabled(bState);
    lblSize1->setEnabled(bState);
    edtSize->setEnabled(bState);
}

/*
#ifndef NO_MOC_INCLUDES
#include "userhistorycfg.moc"
#endif
*/


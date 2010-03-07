/***************************************************************************
                          ontopcfg.cpp  -  description
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

#include "ontopcfg.h"
#include "ontop.h"

#include <QCheckBox>

OnTopCfg::OnTopCfg(QWidget *parent, OnTopPlugin *plugin) : QWidget(parent)
{
	setupUi(this);
    m_plugin = plugin;
    chkInTask->setChecked(m_plugin->value("InTask").toBool());
    chkContainer->setChecked(m_plugin->value("ContainerOnTop").toBool());
}

void OnTopCfg::apply()
{
    m_plugin->setValue("InTask", chkInTask->isChecked());
    m_plugin->setValue("ContainerOnTop", chkContainer->isChecked());
    m_plugin->setState();
}


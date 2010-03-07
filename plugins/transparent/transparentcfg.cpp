/***************************************************************************
                          transparentcfg.cpp  -  description
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

#include "transparentcfg.h"
#include "transparent.h"

#include <QCheckBox>
#include <QSlider>

TransparentCfg::TransparentCfg(QWidget *parent, TransparentPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);
    sldTransparency->setValue(m_plugin->value("Transparency").toUInt());
    chkInactive->setChecked(m_plugin->value("IfInactive").toBool());
    chkMainWindow->setChecked(m_plugin->value("IfMainWindow").toBool());
    chkFloatings->setChecked (m_plugin->value("IfFloatings").toBool());
}

void TransparentCfg::apply()
{
    m_plugin->setValue("Transparency", sldTransparency->value());
    m_plugin->setValue("IfInactive", chkInactive->isChecked());
    m_plugin->setValue("IfMainWindow", chkMainWindow->isChecked());
    m_plugin->setValue("IfFloatings", chkFloatings->isChecked());
    m_plugin->setState();
}



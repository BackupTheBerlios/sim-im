/***************************************************************************
                          autoawaycfg.cpp  -  description
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

#include "autoawaycfg.h"
#include "autoaway.h"

#include <QCheckBox>
#include <QSpinBox>

AutoAwayConfig::AutoAwayConfig(QWidget *parent, AutoAwayPlugin *plugin) : QWidget(parent)
        //: AutoAwayConfigBase(parent)
{
	setupUi(this);
    m_plugin = plugin;
    chkAway->setChecked(m_plugin->getEnableAway());
    chkNA->setChecked(m_plugin->getEnableNA());
    chkOff->setChecked(m_plugin->getEnableOff());
    spnAway->setValue(m_plugin->getAwayTime());
    spnNA->setValue(m_plugin->getNATime());
    spnOff->setValue(m_plugin->getOffTime());
    chkAlert->setChecked(m_plugin->getDisableAlert());
    connect(chkAway, SIGNAL(toggled(bool)), this, SLOT(toggledAway(bool)));
    connect(chkNA, SIGNAL(toggled(bool)), this, SLOT(toggledNA(bool)));
    connect(chkOff, SIGNAL(toggled(bool)), this, SLOT(toggledOff(bool)));
    toggledAway(chkAway->isChecked());
    toggledNA(chkNA->isChecked());
    toggledOff(chkOff->isChecked());
}

void AutoAwayConfig::toggledAway(bool bState)
{
    spnAway->setEnabled(bState);
}

void AutoAwayConfig::toggledNA(bool bState)
{
    spnNA->setEnabled(bState);
}

void AutoAwayConfig::toggledOff(bool bState)
{
    spnOff->setEnabled(bState);
}

void AutoAwayConfig::apply()
{
    m_plugin->setDisableAlert(chkAlert->isChecked());
    m_plugin->setEnableAway(chkAway->isChecked());
    m_plugin->setEnableNA(chkNA->isChecked());
    m_plugin->setEnableOff(chkOff->isChecked());
    if (m_plugin->getEnableAway())
        m_plugin->setAwayTime(spnAway->text().toULong());
    if (m_plugin->getEnableNA())
        m_plugin->setNATime(spnNA->text().toULong());
    if (m_plugin->getEnableOff())
        m_plugin->setOffTime(spnOff->text().toULong());
}


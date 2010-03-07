/***************************************************************************
                          homedircfg.cpp  -  description
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

#include "homedircfg.h"
#include "homedir.h"

#include "simgui/editfile.h"

#include <QRegExp>
#include <QCheckBox>
#include <QDir>

HomeDirConfig::HomeDirConfig(QWidget *parent, HomeDirPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);
#ifdef WIN32 // ER
    chkDefault->setChecked(plugin->m_bDefault);
#endif    
    connect(chkDefault, SIGNAL(toggled(bool)), this, SLOT(defaultToggled(bool)));
    defaultToggled(chkDefault->isChecked());
    edtPath->setText(QDir::convertSeparators(plugin->m_homeDir));
    edtPath->setDirMode(true);
#ifdef WIN32 // ER
    chkDefault->setChecked(m_plugin->m_bDefault);
#endif    
}

void HomeDirConfig::apply()
{
    bool bDefault;
    QString d;
    QString defPath = m_plugin->defaultPath();

    if (chkDefault->isChecked()){
        bDefault = true;
        d = defPath;
    }else{
        bDefault = false;
        d = edtPath->text();
    }
    if (d.isEmpty()) {
        d = defPath;
    }
    QDir dir(d);
    if (!dir.exists()) {
        d = defPath;
        bDefault = true;
    }

    if (d.endsWith("/") || d.endsWith("\\"))
        d = d.left(d.length() - 1);

    edtPath->setText(QDir::convertSeparators(d));
    m_plugin->m_bDefault = bDefault;
    m_plugin->m_homeDir  = d;
    m_plugin->m_bSave    = true;
}

void HomeDirConfig::defaultToggled(bool bState)
{
    edtPath->setEnabled(!bState);
    if (bState)
        edtPath->setText(m_plugin->defaultPath());
}


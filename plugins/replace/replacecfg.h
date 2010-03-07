/***************************************************************************
                          replacecfg.h  -  description
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

#ifndef _REPLACECFG_H
#define _REPLACECFG_H

#include "ui_replacecfgbase.h"

class ReplacePlugin;

class ReplaceCfg : public QWidget, public Ui::ReplaceCfgBase
{
    Q_OBJECT

public:
    ReplaceCfg(QWidget *parent, ReplacePlugin *plugin);
    virtual ~ReplaceCfg();

public slots:
    void apply();
    void cellChanged( int row, int column );

protected:
    bool string( int row, QString &sKey, QString &sValue );
    void addString( QString sKey, QString sValue );
    void autosizeColumns();

    ReplacePlugin *m_plugin;
};

#endif


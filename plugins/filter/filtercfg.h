/***************************************************************************
                          filtercfg.h  -  description
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

#ifndef _FILTERCFG_H
#define _FILTERCFG_H

#include "ui_filtercfgbase.h"
#include "propertyhub.h"

using namespace SIM;

struct FilterUserData;
class FilterPlugin;
class IgnoreList;

class FilterConfig : public QWidget, public Ui::FilterConfigBase
{
    Q_OBJECT
public:
    FilterConfig(QWidget *parent, PropertyHubPtr data, FilterPlugin *plugin, bool bMain);
    ~FilterConfig();
public slots:
    void apply();
    void apply(PropertyHubPtr);
protected:
    PropertyHubPtr			m_data;
    FilterPlugin			*m_plugin;
    IgnoreList				*m_ignore;
};

#endif


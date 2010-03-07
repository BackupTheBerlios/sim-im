/***************************************************************************
                          weathercfg.h  -  description
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

#ifndef _WEATHERCFG_H
#define _WEATHERCFG_H

#include "event.h"
#include "fetch.h"

#include "ui_weathercfgbase.h"

class WeatherPlugin;
class WIfaceCfg;

class WeatherCfg : public QWidget, public Ui::WeatherCfgBase, public SIM::EventReceiver, public FetchClient
{
    Q_OBJECT
public:
    WeatherCfg(QWidget *parent, WeatherPlugin*);
    ~WeatherCfg();
public slots:
    void apply();
    void search();
    void activated(int index);
    void textChanged(const QString&);
protected:
    bool done(unsigned code, Buffer &data, const QString &headers);
    virtual bool processEvent(SIM::Event *e);
    void fill();
    WeatherPlugin *m_plugin;
    WIfaceCfg	  *m_iface;
    QStringList		m_ids;
    QStringList		m_names;
};

#endif


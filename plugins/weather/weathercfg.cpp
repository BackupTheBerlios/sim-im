/***************************************************************************
                          weathercfg.cpp  -  description
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

#include "weathercfg.h"

#include "wifacecfg.h"
#include "weather.h"

#include "log.h"
#include "misc.h"

#include "simgui/ballonmsg.h"

#include <QToolBar>
#include <QTimer>
#include <QDomDocument>

using namespace SIM;

WeatherCfg::WeatherCfg(QWidget *parent, WeatherPlugin *plugin)
        : QWidget(parent), m_iface(0)
{
    setupUi(this);
    m_plugin = plugin;
    lblLnk->setUrl("http://www.weather.com/?prod=xoap&par=1004517364");
    lblLnk->setText(QString("Weather data provided by weather.com") + QChar((unsigned short)174));
    connect(btnSearch, SIGNAL(clicked()), this, SLOT(search()));
    connect(cmbLocation->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbLocation, SIGNAL(activated(int)), this, SLOT(activated(int)));
    textChanged(QString());
    fill();
    for (QObject *p = parent; p != NULL; p = p->parent()){
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if(!tab)
            continue;
        m_iface = new WIfaceCfg(tab, plugin);
        tab->addTab(m_iface, i18n("Interface"));
        tab->adjustSize();
        break;
    }
}

WeatherCfg::~WeatherCfg()
{
// do not delete - it gets deleted when QTabWidget (=parent) goes away
//    delete m_iface;
}

void WeatherCfg::textChanged(const QString &text)
{
    btnSearch->setEnabled(!text.isEmpty() && isDone());
}

void WeatherCfg::search()
{
    if (!isDone()){
        stop();
        btnSearch->setText(i18n("&Search"));
        textChanged(cmbLocation->lineEdit()->text());
        return;
    }
    if (cmbLocation->lineEdit()->text().isEmpty())
        return;
    btnSearch->setText(i18n("&Cancel"));
    QString url = "http://xoap.weather.com/search/search?where=";
    url += toTranslit(cmbLocation->lineEdit()->text());
    fetch(url);
}

bool WeatherCfg::done(unsigned, Buffer &data, const QString&)
{
    m_ids.clear();
    m_names.clear();
    QDomDocument doc;
    if( !doc.setContent(data) ) {
        log(L_WARN, "XML parse error");
    }
    QDomElement el = doc.firstChildElement( "search" );
    if( !el.isNull() ) {
        el = el.firstChildElement( "loc" );
        while( !el.isNull() ) {
            QString sId = el.attribute( "id" );
            QString sLocation = el.text();
            m_ids.append(sId);
            m_names.append(sLocation);
            el = el.nextSiblingElement( "loc" );
        }
    }
    btnSearch->setText(i18n("&Search"));
    QString oldText = cmbLocation->lineEdit()->text();
    cmbLocation->clear();
    if (m_ids.empty()){
        cmbLocation->lineEdit()->setText(oldText);
        BalloonMsg::message(i18n("Location %1 not found") .arg(oldText), btnSearch, false);
    }else{
        cmbLocation->addItems(m_names);
        cmbLocation->setCurrentIndex(0);
        activated(0);
    }
    textChanged(cmbLocation->lineEdit()->text());
    return false;
}

bool WeatherCfg::processEvent(Event *e)
{
    if (e->type() == m_plugin->EventWeather)
        fill();
    return false;
}

void WeatherCfg::fill()
{
    edtID->setText(m_plugin->value("ID").toString());
    cmbUnits->setCurrentIndex(m_plugin->value("Units").toBool() ? 1 : 0);
    cmbLocation->lineEdit()->setText(m_plugin->value("Location").toString());
    edtDays->setValue(m_plugin->value("Forecast").toUInt());
}

void WeatherCfg::activated(int n)
{
    if (n >= m_ids.size())
        return;
    edtID->setText(m_ids[n]);
}

void WeatherCfg::apply()
{
    m_plugin->setValue("Units", cmbUnits->currentIndex() != 0);
    m_plugin->setValue("Forecast", (unsigned int)edtDays->text().toULong());
    m_plugin->setValue("ID", edtID->text());
    m_plugin->setValue("Location", cmbLocation->lineEdit()->text());
    m_iface->apply();
    if (!m_plugin->value("ID").toString().isEmpty()){
        m_plugin->showBar();
        m_plugin->updateButton();
        if (m_plugin->m_bar)
            m_plugin->m_bar->show();
        m_plugin->setValue("Time", 0);
        m_plugin->setValue("ForecastTime", 0);
        QTimer::singleShot(0, m_plugin, SLOT(timeout()));
    }else{
        m_plugin->hideBar();
    }
}

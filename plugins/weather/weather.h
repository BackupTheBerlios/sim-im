/***************************************************************************
                          weather.h  -  description
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

#ifndef _WEATHER_H
#define _WEATHER_H

#include "propertyhub.h"
#include "plugins.h"
#include "fetch.h"
#include "event.h"

class QToolBar;
class QDomDocument;
class QDomElement;

namespace SIM
{
class IconSet;
}

class WeatherPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver, public FetchClient
{
    Q_OBJECT
public:
    WeatherPlugin(unsigned, bool, Buffer*);
    virtual ~WeatherPlugin();
    QString getButtonText();
    QString getTipText();
    QString getForecastText();
    void updateButton();
    void showBar();
    void hideBar();
    SIM::SIMEvent EventWeather;
    QToolBar *m_bar;

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
protected slots:
    void timeout();
    void barDestroyed();
protected:
    QString replace(const QString&);
    QString forecastReplace(const QString&, int iDay);
    unsigned long BarWeather;
    unsigned long CmdWeather;
    char   m_bForecast;
    virtual QByteArray getConfig();
    bool isDay();
    bool parseTime(const QString &str, int &h, int &m);
    bool parseDateTime(const QString &str, QDateTime &dt);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool done(unsigned code, Buffer &data, const QString &headers);
    virtual bool processEvent(SIM::Event *e);
    SIM::IconSet *m_icons;

    bool parse(QDomDocument document);
    QString GetSubElementText(
        QDomElement element,
        QString sSubElement,
        QString sDefault = QString()
    );
private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


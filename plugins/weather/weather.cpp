/***************************************************************************
                          weather.cpp  -  description
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

#include "weather.h"
#include "weathercfg.h"

#include "log.h"
#include "misc.h"
#include "icons.h"
#include "mainwin.h"
#include "unquot.h"
#include "core.h"

#include "socket/socket.h"
#include "socket/socketfactory.h"
#include "simgui/toolbtn.h"

#include "profile.h"
#include "profilemanager.h"

#include <QDomDocument>
#include <QTimer>

using namespace SIM;

const unsigned CHECK1_INTERVAL = 30 * 60;
const unsigned CHECK2_INTERVAL = 120 * 60;

Plugin *createWeatherPlugin(unsigned base, bool bInit, Buffer *config)
{
    return new WeatherPlugin(base, bInit, config);
}

static PluginInfo info =
    {
        I18N_NOOP("Weather"),
        I18N_NOOP("Plugin provides show weather"),
        VERSION,
        createWeatherPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

WeatherPlugin::WeatherPlugin(unsigned base, bool bInit, Buffer *config)
    : Plugin (base)
    , m_bar  ( NULL)
{
    m_propertyHub = SIM::PropertyHub::create("weather");
    BarWeather = registerType();
    CmdWeather = registerType();
    EventWeather = (SIM::SIMEvent)registerType();
    m_icons = getIcons()->addIconSet("icons/weather.jisp", true);
    EventToolbar(BarWeather, EventToolbar::eAdd).process();

    Command cmd;
    cmd->id = CmdWeather;
    cmd->text = I18N_NOOP("Not connected");
    cmd->icon = "weather";
    cmd->bar_id = BarWeather;
    cmd->bar_grp = 0x1000;
    cmd->flags = BTN_PICT | BTN_DIV;
    EventCommandCreate(cmd).process();
   
}

WeatherPlugin::~WeatherPlugin()
{
    delete m_bar;
    EventCommandRemove(CmdWeather).process();
    EventToolbar(BarWeather, EventToolbar::eRemove).process();
    getIcons()->removeIconSet(m_icons);
}

QByteArray WeatherPlugin::getConfig()
{
    return QByteArray();
}

void WeatherPlugin::timeout()
{
    if (!getSocketFactory()->isActive() || !isDone() || value("ID").toString().isEmpty())
        return;
    QDateTime now = QDateTime::currentDateTime();
    if ( now < QDateTime::fromTime_t(value("Time").toUInt()).addSecs(CHECK1_INTERVAL) )
        return;
    m_bForecast = false;
    if ( now >= QDateTime::fromTime_t(value("ForecastTime").toUInt()).addSecs(CHECK2_INTERVAL) )
        m_bForecast = true;
    QString url = "http://xoap.weather.com/weather/local/";
    url += value("ID").toString();
    url += "?cc=*&link=xoap&prod=xoap&par=1004517364&key=a29796f587f206b2&unit=";
    url += value("Units").toBool() ? "s" : "m";
    if (m_bForecast && value("Forecast").toUInt()){
        url += "&dayf=";
        url += QString::number(value("Forecast").toUInt());
    }
    fetch(url);
}

bool WeatherPlugin::processEvent(Event *e)
{
    if (e->type() == eEventLanguageChanged)
        updateButton();
    if (e->type() == eEventInit)
        showBar();
    if (e->type() == eEventCommandExec)
    {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdWeather) && !value("ID").toString().isEmpty()){
            QString url = "http://www.weather.com/outlook/travel/local/";
            url += value("ID").toString();
            EventGoURL(url).process();
            return true;
        }
    }
    if(e->type() == eEventPluginLoadConfig)
    {
        PropertyHubPtr hub = ProfileManager::instance()->getPropertyHub("weather");
        if(!hub.isNull())
            setPropertyHub(hub);
        showBar();
        if (m_bar) 
        {
            m_bar->setIconSize( QSize( 30, 30 ) );
            m_bar->show();
        }
    }
    return false;
}

bool WeatherPlugin::done(unsigned code, Buffer &data, const QString&)
{
    if (code != 200)
        return false;
    QDomDocument document;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!document.setContent(data,false,&errorMsg,&errorLine,&errorColumn)){
        log(L_WARN, "Weather XML parse error '" + errorMsg + "' at line: " + QString::number(errorLine) + ", column:" + QString::number(errorColumn) + ")");
        return false;
    }
    if( !parse( document ) ) {
        return false;
    }
    unsigned int now = QDateTime::currentDateTime().toTime_t();
    setValue("Time", now);
    if (m_bForecast)
        setValue("ForecastTime", now);
    updateButton();
    Event eUpdate(EventWeather);
    eUpdate.process();
    return false;
}

void WeatherPlugin::barDestroyed()
{
    m_bar = NULL;
}

void WeatherPlugin::hideBar()
{
    if (m_bar){
        delete m_bar;
        m_bar = NULL;
    }
}

bool WeatherPlugin::parseTime(const QString &str, int &h, int &m)
{
    QString s = str;
    h = getToken(s, ':').toLong();
    m = getToken(s, ' ').toLong();
    if ((getToken(s, ' ') == "PM") && (h < 12))
        h += 12;
    if (h == 24)
        h = 0;
    return true;
}

bool WeatherPlugin::parseDateTime(const QString &str, QDateTime &dt)
{
    int h, m, D, M, Y;
    QString daytime;

    QString s = str;
    /* MM/DD/YY/ hh:mm */
    M = getToken(s, '/').toLong();
    D = getToken(s, '/').toLong();
    Y = getToken(s, ' ').toLong();
    h = getToken(s, ':').toLong();
    m = getToken(s, ' ').toLong();

    if (getToken(s, ' ') == "PM"  && (h < 12))
        h += 12;
    /* 12:20 PM is 00:20 and 12:30 AM is 12:20
       but what date is 12:20 pm 4/7/04? */
    if (h == 24)
        h = 0;
    if (Y < 70)
        Y += 2000;
    dt.setDate(QDate(Y,M,D));
    dt.setTime(QTime(h,m,0,0));
    return true;
}

bool WeatherPlugin::isDay()
{
    int raise_h = 0, raise_m = 0;
    int set_h = 0, set_m = 0;
    if (!parseTime(value("Sun_raise").toString(), raise_h, raise_m) || !parseTime(value("Sun_set").toString(), set_h, set_m))
        return false;
    QDateTime now = QDateTime::currentDateTime().toLocalTime();
    if ((now.time().hour() > raise_h) && (now.time().hour() < set_h))
        return true;
    if ((now.time().hour() == raise_h) && (now.time().minute() >= raise_m))
        return true;
    if ((now.time().hour() == set_h) && (now.time().minute() <= set_m))
        return true;
    return false;
}

void WeatherPlugin::showBar()
{
    if (m_bar || value("ID").toString().isEmpty())
        return;
 
    CorePlugin *core = GET_CorePlugin();
    MainWindow *main= core->getMainWindow();
    if (main == NULL)
        return;
    
    EventToolbar e(BarWeather, main);
    e.process();
    m_bar = e.toolBar();
    m_bar->setObjectName("WeatherToolbar");
    main->addToolBar(Qt::BottomToolBarArea, m_bar);
    connect(m_bar, SIGNAL(destroyed()), this, SLOT(barDestroyed()));
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    QTimer::singleShot(0, this, SLOT(timeout()));
    timer->start( min( CHECK1_INTERVAL, CHECK2_INTERVAL ) * 1000 );
    updateButton();
}

void WeatherPlugin::updateButton()
{
    if ((value("Time").toUInt() == 0) || (m_bar == NULL))
        return;
    Command cmd;
    cmd->id      = CmdWeather;
    cmd->text    = I18N_NOOP("Not connected");
    cmd->icon    = "weather" + QString::number(value("Icon").toUInt());
    cmd->bar_id  = BarWeather;
    cmd->bar_grp = 0x1000;
    cmd->flags   = BTN_PICT | BTN_DIV;
    EventCommandChange(cmd).process();

    QString text = unquoteText(getButtonText());
    QString tip  = "<center><h1>%l</h1></center>\n<table><tr><td>" + getTipText() + "</td>";
    QString ftip = getForecastText();
    text = replace(text);
    tip  = replace(tip);
    if (value("Forecast").toUInt())
        tip = tip + "<td><h2>" + i18n("weather","Forecast") + ":</h2><table><tr><td>";
    unsigned n = (value("Forecast").toUInt() + 1) / 2;
    if (n < 3)
        n = value("Forecast").toUInt();
    for (unsigned int iDay = 1; iDay <= value("Forecast").toUInt(); iDay++){
        tip += forecastReplace(ftip,iDay);
        if (--n == 0){
            tip += "</td><td>";
            n = (value("Forecast").toUInt() + 1) / 2;
        }
    }
    if (value("Forecast").toUInt())
        tip += "</td></tr></table></td>";
    tip += "</tr></table><br>\n";
    tip += "<i><h3>"+i18n("weather", "Weather data provided by weather.com&reg;") + "</h3></i>";
    Command cmdw;
    cmdw->id	= CmdWeather;
    cmdw->param	= m_bar;
    EventCommandWidget eWidget(cmdw);
    eWidget.process();
    CToolButton *btn = qobject_cast<CToolButton*>(eWidget.widget());
    if (btn == NULL)
        return;
    btn->setText(text);
    btn->repaint();
    btn->setToolTip(tip);
}

#if 0
i18n("Monday")
i18n("Tuesday")
i18n("Wednesday")
i18n("Thursday")
i18n("Friday")
i18n("Saturday")
i18n("Sunday")

i18n("weather", "Squalls")
i18n("weather", "Overcast")
i18n("weather", "Fog")
i18n("weather", "Foggy")
i18n("weather", "Mist")
i18n("weather", "Haze")
i18n("weather", "Storm")
i18n("weather", "T-Storm")
i18n("weather", "T-Storms")
i18n("weather", "Scattered T-Storms")
i18n("weather", "Thunder")
i18n("weather", "Light Rain with Thunder")
i18n("weather", "Thunder in the Vicinity")
i18n("weather", "Rain")
i18n("weather", "Light Rain")
i18n("weather", "Heavy Rain")
i18n("weather", "Freezing Rain")
i18n("weather", "Rain to Snow")
i18n("weather", "Snow")
i18n("weather", "Light Snow")
i18n("weather", "Few Snow")
i18n("weather", "Scattered Snow")
i18n("weather", "Ice Crystals")
i18n("weather", "Sleet")
i18n("weather", "Clear")
i18n("weather", "Clearing")
i18n("weather", "Showers")
i18n("weather", "Showers in the Vicinity")
i18n("weather", "Mostly Clear")
i18n("weather", "Sunny")
i18n("weather", "Fair")
i18n("weather", "Cloudy")
i18n("weather", "Clouds")
i18n("weather", "Mostly Cloudy")
i18n("weather", "Partly Cloudy")
i18n("weather", "Wind")
i18n("weather", "Windy")
i18n("weather", "Drizzle")
i18n("weather", "Heavy Drizzle")
i18n("weather", "Freezing Drizzle")
i18n("weather", "Freezing Rain")
i18n("weather", "Light Drizzle")
i18n("weather", "Drifting Snow")
i18n("weather", "Snow Grains")
i18n("weather", "Light Snow Grains")
i18n("weather", "Scattered")
i18n("weather", "Smoke")
i18n("weather", "steady")
i18n("weather", "rising")
i18n("weather", "falling")
i18n("weather", "Unlimited")

i18n("weather", "N")
i18n("weather", "NNW")
i18n("weather", "NW")
i18n("weather", "WNW")
i18n("weather", "W")
i18n("weather", "WSW")
i18n("weather", "SW")
i18n("weather", "SSW")
i18n("weather", "S")
i18n("weather", "SSE")
i18n("weather", "SE")
i18n("weather", "ESE")
i18n("weather", "E")
i18n("weather", "ENE")
i18n("weather", "NE")
i18n("weather", "NNE")
i18n("weather", "VAR")
i18n("weather", "CALM")

i18n("weather", "km")
i18n("weather", "mi")
i18n("weather", "km/h")
i18n("weather", "mph")
i18n("weather", "mmHg");
i18n("weather", "inHg");

i18n("weather", "Low")
i18n("weather", "Moderate")
i18n("weather", "High")

i18n("moonphase", "New")
i18n("moonphase", "Waxing Crescent")
i18n("moonphase", "First Quarter")
i18n("moonphase", "Waxing Gibbous")
i18n("moonphase", "Full")
i18n("moonphase", "Waning Gibbous")
i18n("moonphase", "Last Quarter")
i18n("moonphase", "Waning Crescent")
#endif

static QString i18n_conditions(const QString &str)
{
    if (str.isEmpty())
        return QString();
    int n = str.indexOf(" / ");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " / " + i18n_conditions(str.mid(n + 3));
    n = str.indexOf(" and ");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " " + i18n("and") + " " + i18n_conditions(str.mid(n + 5));
    n = str.indexOf(" Early");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " " + i18n("weather", "Early");
    n = str.indexOf(" Late");
    if (n >= 0)
        return i18n_conditions(str.left(n)) + " " + i18n("weather", "Late");
    QString s = str;
    s = s.remove(" Showers");
    s = s.remove(" Shower");
    return i18n("weather", s);
}

QString WeatherPlugin::replace(const QString &text)
{
    QString res = text;
    QString sun_set, sun_raise, updated;
    QTime tmp_time;
    QDateTime dt;
    int h,m;

    parseTime(value("Sun_set").toString(),h,m);
    tmp_time.setHMS(h,m,0,0);
    sun_set = tmp_time.toString(Qt::LocalDate);
    sun_set = sun_set.left(sun_set.length() - 3);

    parseTime(value("Sun_raise").toString(),h,m);
    tmp_time.setHMS(h,m,0,0);
    sun_raise = tmp_time.toString(Qt::LocalDate);
    sun_raise = sun_raise.left(sun_raise.length() - 3);

    parseDateTime(value("Updated").toString(),dt);
    updated = dt.toString(Qt::LocalDate);
    updated = updated.left(updated.length() - 3);
    /* double Expressions *before* single or better RegExp ! */
    res = res.replace(QRegExp("\\%mp"), i18n("moonphase", value("MoonPhase").toString()));
    res = res.replace(QRegExp("\\%mi"), QString::number(value("MoonIcon").toInt()));
    res = res.replace(QRegExp("\\%pp"), QString::number(value("Precipitation").toInt()));
    res = res.replace(QRegExp("\\%ut"), i18n("weather", value("UV_Description").toString()));
    res = res.replace(QRegExp("\\%ui"), QString::number(value("UV_Intensity").toUInt()));
    res = res.replace(QRegExp("\\%t"), QString::number((int)value("Temperature").toInt()) + QChar((unsigned short)176) + value("UT").toString());
    res = res.replace(QRegExp("\\%f"), QString::number((int)value("FeelsLike").toInt()) + QChar((unsigned short)176) + value("UT").toString());
    res = res.replace(QRegExp("\\%d"), QString::number((int)value("DewPoint").toInt()) + QChar((unsigned short)176) + value("UT").toString());
    res = res.replace(QRegExp("\\%h"), QString::number(value("Humidity").toInt()) + "%");
    res = res.replace(QRegExp("\\%w"), QString::number(value("Wind_speed").toInt()) + " " + i18n("weather",value("US").toString()));
    res = res.replace(QRegExp("\\%x"), QString::number(value("Wind_speed").toInt() * 10 / 36) + " " + i18n("m/s"));
    res = res.replace(QRegExp("\\%g"), value("WindGust").toInt() ? QString("(") + i18n("gust") + QString(" ") + QString::number(value("WindGust").toUInt()) + i18n("weather",value("US").toString()) + QString(")") : QString(""));
    res = res.replace(QRegExp("\\%y"), value("WindGust").toInt() ? QString("(") + i18n("gust") + QString(" ") + QString::number(value("WindGust").toInt() * 10 / 36) + QString(" ") + i18n("m/s") + QString(")") : QString(""));
    res = res.replace(QRegExp("\\%p"), QString::number(value("Pressure").toInt()) + " " + i18n("weather", value("UP").toString()));
    res = res.replace(QRegExp("\\%a"), QString::number(value("Pressure").toInt() * 75 / 100)); // deprecated!
    res = res.replace(QRegExp("\\%q"), i18n("weather", value("PressureD").toString()));
    res = res.replace(QRegExp("\\%l"), value("Location").toString());
    res = res.replace(QRegExp("\\%b"), i18n("weather", value("Wind").toString()));
    res = res.replace(QRegExp("\\%u"), updated);
    res = res.replace(QRegExp("\\%r"), sun_raise);
    res = res.replace(QRegExp("\\%s"), sun_set);
    res = res.replace(QRegExp("\\%c"), i18n_conditions(value("Conditions").toString()));
    res = res.replace(QRegExp("\\%v"), i18n("weather", value("Visibility").toString()) + (value("Visibility").toUInt() ? ' ' + i18n("weather",value("UD").toString()) : QString()));
    res = res.replace(QRegExp("\\%i"), QString::number(value("Icon").toUInt()));
        res = res.replace(QRegExp("\\%o"), value("Obst").toString());
    return res;
}

QString WeatherPlugin::forecastReplace(const QString &text, int iDay)
{
    QString sDay = QString::number(iDay);
    if (value("Day").toMap().value(sDay).toString().isEmpty())
        return QString();
    QString res = text;
    QString temp;
    int minT = value("MinT").toMap().value(sDay).toInt();
    int maxT = value("MaxT").toMap().value(sDay).toInt();
    temp += QString::number(minT);
    temp += QChar((unsigned short)176);
    temp += value("UT").toString();
    if ((value("MaxT").toMap().value(sDay).toString() != QLatin1String("N/A")) && (maxT != -255)) {
        temp += '/';
        temp += QString::number(maxT);
        temp += QChar((unsigned short)176);
        temp += value("UT").toString();
    }
    QString dd = value("Day").toMap().value(sDay).toString();
    QString mon = getToken(dd, ' ');
    QString day = dd;
    day += ". ";
    day += i18n(mon);
    res = res.replace(QRegExp("\\%n"), value("DayIcon").toMap().value(sDay).toString());
    res = res.replace(QRegExp("\\%t"), temp);
    res = res.replace(QRegExp("\\%c"), i18n_conditions(value("DayConditions").toMap().value(sDay).toString()));
    res = res.replace(QRegExp("\\%w"), i18n(value("WDay").toMap().value(sDay).toString()));
    res = res.replace(QRegExp("\\%d"), day);
    return res;
}

QString WeatherPlugin::getButtonText()
{
    QString str = value("Text").toString();
    if (str.isEmpty())
        str = i18n("%t | %c");
    return str;
}

#define WIATHER_ICON_SIZE (93)

QString WeatherPlugin::getTipText()
{
    QString str = value("Tip").toString();
    if (str.isEmpty())
        str =
        "<h2>"+i18n("weather","Current Weather")+":</h2><p>\n"
        "<img height=\"" + QString::number( WIATHER_ICON_SIZE ) +
            "\" width=\"" + QString::number( WIATHER_ICON_SIZE ) + "\" " +
            "src=\"sim:icons/weather%i\"></p><br>%c<br>\n"+
		i18n("weather","Temperature")+": <b>%t</b> ("+i18n("weather","feels like")+": <b>%f</b>)<br>\n"+
		i18n("weather","Humidity")+": <b>%h</b><br>\n"+
		i18n("weather","Chance of Precipitation")+": <b>%pp%</b><br>\n"+
		i18n("weather","Pressure")+": <b>%p</b> (%q)<br>\n"+
		i18n("weather","Wind")+": <b>%b</b> <b>%w %g</b><br>\n"+
		i18n("weather","Visibility")+": <b>%v</b><br>\n"+
		i18n("weather","Dew Point")+": <b>%d</b><br>\n"+
		i18n("weather","Sunrise")+": %r<br>\n"+
		i18n("weather","Sunset")+": %s<br>\n"+
		i18n("weather","UV-Intensity is <b>%ut</b> with value <b>%ui</b> (of 11)")+"<br>\n"
		"<b>"+i18n("weather","Moonphase")+": </b>%mp<br>\n"
        "<img src=\"sim:icons/moon%mi\"><br>\n"
		"<br>\n"+
		i18n("weather","Updated")+": %u<br>\n";
    return str;
}

QString WeatherPlugin::getForecastText()
{
    QString str = value("ForecastTip").toString();
    if (str.isEmpty())
        str = i18n("<nobr><h3>%d %w</h3></nobr><br>\n"
                   "<img height=\"" + QString::number( (int)(WIATHER_ICON_SIZE / 2) ) + "\" "
                        "width=\"" + QString::number( (int)(WIATHER_ICON_SIZE / 2) ) + "\" "
                        "src=\"sim:icons/weather%n\"><br>%c<br>\n"
                   "Temperature: <b>%t</b><br>\n");

    return str;
}

QWidget *WeatherPlugin::createConfigWindow(QWidget *parent)
{
    return new WeatherCfg(parent, this);
}

QString WeatherPlugin::GetSubElementText(
    QDomElement element,
    QString sSubElement,
    QString sDefault /*= QString()*/
) {
    QString sResult = sDefault;

    do {
        if( element.isNull() )
            break;

        QDomNodeList list = element.elementsByTagName(sSubElement);
        if( list.count() <= 0 )
            break;

        //QDomNode::NodeType t = list.item(0).nodeType(); //unused
        QDomElement subElement = list.item(0).toElement();
        sResult = subElement.firstChild().toCharacterData().data();
    } while( false );

    return sResult;
}

bool WeatherPlugin::parse(QDomDocument document)
{
    QDomElement weatherElement = document.documentElement();

// Parsing head element
    QDomElement headElement = weatherElement.elementsByTagName("head").item(0).toElement();
    setValue("UT", GetSubElementText( headElement, "ut", "### Failed ###" ) );
    QString sUp = GetSubElementText( headElement, "up", "### Failed ###" );
    if( sUp == "in" ) {
        setValue("UP", "inHg");
    }
    else {
        setValue("UP", sUp);
    }
    setValue("US", GetSubElementText( headElement, "us", "### Failed ###" ) );
    setValue("UD", GetSubElementText( headElement, "ud", "### Failed ###" ) );

// Parsing loc element
    QDomElement locElement = weatherElement.elementsByTagName("loc").item(0).toElement();
    setValue("Location", GetSubElementText( locElement, "dnam", "### Failed ###" ) );
    setValue("Sun_raise", GetSubElementText( locElement, "sunr", "### Failed ###" ) );
    setValue("Sun_set", GetSubElementText( locElement, "suns", "### Failed ###" ) );

// Parsing cc element
    QDomElement ccElement = weatherElement.elementsByTagName("cc").item(0).toElement();
    setValue("Obst", GetSubElementText( ccElement, "obst", "### Failed ###" ) );
    setValue("Updated", GetSubElementText( ccElement, "lsup", "### Failed ###" ) );
    setValue("Temperature", (int)GetSubElementText( ccElement, "tmp", "-10000" ).toLong() );
    setValue("FeelsLike", (int)GetSubElementText( ccElement, "flik", "-10000" ).toLong() );
    setValue("Visibility", GetSubElementText( ccElement, "vis", "### Failed ###" ) );
    setValue("DewPoint", (int)GetSubElementText( ccElement, "dewp", "-10000" ).toLong() );
    setValue("Humidity", (int)GetSubElementText( ccElement, "hmid", "-10000" ).toLong() );
    setValue("Conditions", GetSubElementText( ccElement, "t", "### Failed ###" ) );
    setValue("Icon", (unsigned int)GetSubElementText( ccElement, "icon", "0" ).toLong() );
    // Parsing cc/moon element
    {
        QDomElement subElement = ccElement.elementsByTagName("moon").item(0).toElement();
        setValue("MoonPhase", GetSubElementText( subElement, "t", "### Failed ###" ) );
        setValue("MoonIcon", (int)GetSubElementText( subElement, "icon", "0" ).toLong() );
    }
    // Parsing cc/bar element
    {
        QDomElement subElement = ccElement.elementsByTagName("bar").item(0).toElement();
        float v = GetSubElementText( subElement, "r", "-10000" ).toFloat();
        if ( QString(value("UP").toString()) == "mb" ){
            v=v * 75 / 100;
            setValue("Pressure", v);
            setValue("UP", "mmHg");
        } else{
            setValue("Pressure", v);
        }
        setValue("PressureD", GetSubElementText( subElement, "d", "### Failed ###" ) );
    }
    // Parsing cc/wind element
    {
        QDomElement subElement = ccElement.elementsByTagName("wind").item(0).toElement();
        setValue("Wind", GetSubElementText( subElement, "t", "### Failed ###" ) );
        setValue("WindGust", (int)GetSubElementText( subElement, "gust", "-10000" ).toLong() );
        setValue("Wind_speed", (int)GetSubElementText( subElement, "s", "-10000" ).toLong() );
        // wind/d dropped for now !
    }
    // Parsing cc/uv element
    {
        QDomElement subElement = ccElement.elementsByTagName("uv").item(0).toElement();
        setValue("UV_Description", GetSubElementText( subElement, "t", "### Failed ###" ) );
        setValue("UV_Intensity", (int)GetSubElementText( subElement, "i", "-10000" ).toLong() );
    }

// Parsing dayf element
    QDomElement dayfElement = weatherElement.elementsByTagName("dayf").item(0).toElement();
    QDomNodeList list = dayfElement.elementsByTagName("day");
    for( int iDay = 0 ; iDay < list.count() ; iDay++ ) {
        QDomElement dayElement = list.item(iDay).toElement();
//        dayElement.attribute("d").toLong();
        QVariantMap day = value("Day").toMap();
        day.insert(QString::number(iDay), dayElement.attribute("dt"));
        setValue("Day", day);

        QVariantMap wday = value("WDay").toMap();
        wday.insert(QString::number(iDay), dayElement.attribute("t"));
        setValue("WDay", wday);

        QVariantMap mint = value("MinT").toMap();
        mint.insert(QString::number(iDay), GetSubElementText(dayElement, "low", "### Failed ###"));
        setValue("MinT", mint);

        QVariantMap maxt = value("MaxT").toMap();
        maxt.insert(QString::number(iDay), GetSubElementText(dayElement, "hi", "### Failed ###"));
        setValue("MaxT", maxt);

        QDomNodeList listParts = dayElement.elementsByTagName("part");
        for( int iPart = 0 ; iPart < listParts.count() ; iPart++ ) {
            QDomElement partElement = listParts.item(iPart).toElement();
            if( partElement.attribute("p") == "d" ) {
                QVariantMap dayc = value("DayConditions").toMap();
                dayc.insert(QString::number(iDay), GetSubElementText( partElement, "t", "### Failed ###" ));
                setValue("DayConditions", dayc);

                QVariantMap dayi = value("DayIcon").toMap();
                dayi.insert(QString::number(iDay), GetSubElementText(partElement, "icon", "na"));
                setValue("DayIcon", dayi);
            }
        }
    }

    return true;
}

void WeatherPlugin::setPropertyHub(SIM::PropertyHubPtr hub)
{
	m_propertyHub = hub;
}

SIM::PropertyHubPtr WeatherPlugin::propertyHub()
{
	return m_propertyHub;
}

QVariant WeatherPlugin::value(const QString& key)
{
	return m_propertyHub->value(key);
}

void WeatherPlugin::setValue(const QString& key, const QVariant& v)
{
	m_propertyHub->setValue(key, v);
}

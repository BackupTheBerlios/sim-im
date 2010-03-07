/***************************************************************************
                          splash.cpp  -  description
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

#include <QWidget>
#include <QPixmap>
#include <QApplication>
#include <QFile>
#include <QPainter>
#include <QSplashScreen>
#include <QBitmap>
#include <QDesktopWidget>

#include "aboutdata.h"
#include "misc.h"
#include "log.h"

#include "splash.h"


using namespace SIM;

Plugin *createSplashPlugin(unsigned base, bool bStart, Buffer*)
{
    Plugin *plugin = new SplashPlugin(base, bStart);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Splash"),
        I18N_NOOP("Plugin provides splash screen"),
        VERSION,
        createSplashPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

SplashPlugin::SplashPlugin(unsigned base, bool bStart) : QObject(NULL), Plugin(base)
{
	splash = NULL;
	m_bStart = bStart;
	if (m_bStart)
	{
		QPixmap pict(app_file("pict/splash.png"));
		// FIXME: better use QSplash with QSplashScreen::drawContents()
		if(!pict.isNull())
		{
			//KAboutData *about_data = getAboutData();
			QString text = "";// about_data->programName();
			text += " ";
			//text += about_data->version();
			QPainter p(&pict);
			QFont f = qApp->font();
			f.setBold(true);
			p.setFont(f);
			QRect rc = p.boundingRect(0, 0, pict.width(), pict.height(), Qt::AlignLeft | Qt::AlignTop, text);
			int x = pict.width() - 7 - rc.width();
			int y = 7 + rc.height();
			p.setPen(QColor(0x80, 0x80, 0x80));
			p.drawText(x, y, text);
			x -= 2;
			y -= 2;
			p.setPen(QColor(0xFF, 0xFF, 0xE0));
			p.drawText(x, y, text);
			splash = new QWidget(NULL, Qt::SplashScreen);
            splash->setObjectName("splash");

			QDesktopWidget *desktop =  qApp->desktop();  //QApplication::desktop();
			int desk_width = desktop->geometry().width();
			int desk_height = desktop->geometry().height();
			if ((desk_width/desk_height)==2) //widescreen or double screen
				splash->move((desktop->width()/2 - pict.width()) / 2, (desktop->height() - pict.height()) / 2);
			else //normal screen 
				splash->move((desktop->width() - pict.width()) / 2, (desktop->height() - pict.height()) / 2);
                        QPalette palette = splash->palette();
                        palette.setBrush(splash->backgroundRole(), QBrush(pict));
                        splash->setPalette(palette);
			splash->resize(pict.width(), pict.height());
			splash->repaint();
			const QBitmap mask = pict.mask();
			p.end();
			if (!mask.isNull())
				splash->setMask(mask);
			splash->show();
            m_timer = new QTimer(this);
            connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
            m_timer->start(5000);
		}
	}
}

SplashPlugin::~SplashPlugin() 
{
    delete splash;
}

void SplashPlugin::timeout() //Anywhere is definitly a bug. Splash is going to be unloaded and not fulltime displayed...
{
    splash->hide();
}


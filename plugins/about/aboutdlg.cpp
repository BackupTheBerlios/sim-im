/***************************************************************************
                          aboutdlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "simapi.h"

#ifndef USE_KDE

#include <QLabel>
#include <QPushButton>
#include <QRegExp>
#include <QTabWidget>
#include <QFile>
#include <QLineEdit>
#include <QList>
#include <QCloseEvent>

#include "aboutdata.h"
#include "icons.h"
#include "misc.h"
#include "simgui/textshow.h"

#include "aboutdlg.h"
#include "about.h"

using namespace SIM;

KAboutApplication::KAboutApplication( const KAboutData *aboutData, QWidget *parent, const char *name, bool modal)
  : QDialog(parent)
{
	setupUi(this);
	SET_WNDPROC("about");
	setButtonsPict(this);
        setObjectName(name);
        setModal(modal);

	connect(btnOK, SIGNAL(clicked()), this, SLOT(close()));
    setWindowIcon(SIM::Icon("SIM"));
	QIcon icon = SIM::Icon("SIM");
        lblIcon->setPixmap(SIM::Pict("SIM"));
	edtVersion->setText(i18n("%1 Version: %2") .arg(aboutData->appName()) .arg(aboutData->version()));
	edtVersion->setReadOnly(true);
	txtAbout->setText((QString("<center><br>%1<br><br>%2<br><br>") +
				"<a href=\"%3\">%4</a><br><br>" +
				i18n("Bug report") + ": <a href=\"%5\">%6</a><br>" +
				i18n("Note: This is an english mailing list") +
				"</center>")
			.arg(quote(aboutData->shortDescription()))
			.arg(quote(aboutData->copyrightStatement()))
			.arg(quote(aboutData->homepage()))
			.arg(quote(aboutData->homepage()))
			.arg(quote(aboutData->bugAddress()))
			.arg(quote(aboutData->bugAddress())));
	QString txt;
    KAboutPerson person;
    foreach( person, aboutData->authors() )
	{
        txt += addPerson(&person);
		txt += "<br>";
	}
	txtAuthors->setText(txt);
	txt = QString::null;
    QList<KAboutTranslator> translators = aboutData->translators();
    QList<KAboutTranslator>::iterator itt;
	if (!translators.isEmpty())
	{
		for (itt = translators.begin(); itt != translators.end(); ++itt)
		{
			const KAboutTranslator &t = *itt;
			txt += QString("<br><center>%1<br>&lt;<a href=\"mailto:%2\">%3</a>&gt;")
				.arg(quote(t.name()))
				.arg(quote(t.emailAddress()))
				.arg(quote(t.emailAddress()));
			txt += "</center>";
		}
		txtTranslations->setText(txt);
	}
	else
	{
        tabMain->removeTab(tabMain->indexOf(tabTranslation));
	}
	QString license = aboutData->license();
	license += "\n\n";
	QFile f(SIM::app_file("COPYING"));
	if (f.open(QIODevice::ReadOnly))
	{
		for (;;)
		{
			QString s = QString(f.readLine(512));
			if(s.isEmpty() || s.isNull())
				break;
			license += s;
		}
	}
	txtLicence->setText(quote(license));
	this->setFixedSize(this->width()+50,this->height());
}

KAboutApplication::~KAboutApplication()
{
}

void KAboutApplication::closeEvent(QCloseEvent *e)
{
    QDialog::closeEvent(e);
    emit finished();
}

QString KAboutApplication::addPerson(const KAboutPerson *p)
{
    QString res;
    if (!p->task().isEmpty()){
        res += quote(p->task());
        res += ":<br>";
    }
    res += QString("%1 &lt;<a href=\"mailto:%2\">%3</a>&gt;<br>")
           .arg(quote(p->name()))
           .arg(quote(p->emailAddress()))
           .arg(quote(p->emailAddress()));
    if (!p->webAddress().isEmpty())
        res += QString("<a href=\"%1\">%2</a><br>")
               .arg(quote(p->webAddress()))
               .arg(quote(p->webAddress()));
    return res;
}

QString KAboutApplication::quote(const QString &s)
{
    QString res = s;
    res.replace('&',  "&amp;");
    res.replace('\"', "&quot;");
    res.replace('<',  "&lt;");
    res.replace('>',  "&gt;");
    res.replace('\n', "<br>");
    return res;
}

#endif


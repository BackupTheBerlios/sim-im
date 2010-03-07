/***************************************************************************
                          yahoosearch.cpp  -  description
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

#include <cstdio>

#include <QLabel>
#include <QComboBox>
#include <QByteArray>
#include <QShowEvent>

#include "contacts/contact.h"
#include "contacts/group.h"
#include "simgui/intedit.h"

#include "country.h"
#include "log.h"
#include "misc.h"

#include "yahoosearch.h"
#include "yahooclient.h"

using namespace SIM;

const ext_info ages[] =
    {
        { "13-18", 1 },
        { "18-25", 2 },
        { "25-35", 3 },
        { "35-50", 4 },
        { "50-70", 5 },
        { "> 70",  6 },
        { "", 0 }
    };

const ext_info genders[] =
    {
        { I18N_NOOP("Male"), 1 },
        { I18N_NOOP("Female"), 2 },
        { "", 0 }
    };

#if 0
i18n("male")
i18n("female")
#endif

YahooSearch::YahooSearch(YahooClient *client, QWidget *parent) : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtID->setValidator(new QRegExpValidator(QRegExp("[0-9A-Za-z \\.\\-_]+"), this));
    initCombo(cmbAge, 0, ages);
    initCombo(cmbGender, 0, genders);
	emit setAdd(true);
}

void YahooSearch::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    emit setAdd(true);
}

void YahooSearch::search()
{
	/*
    if (grpID->isChecked()){
        search(edtID->text(), 1);
    }else if (grpName->isChecked()){
        search(edtName->text(), 2);
    }else if (grpKeyword->isChecked()){
        search(edtKeyword->text(), 0);
    }
	*/
	log(L_DEBUG, "YahooSearch::search() FIXME!!!");
}

void YahooSearch::search(const QString &text, int type)
{
    QString url;
    url = "http://members.yahoo.com/interests?.oc=m&.kw=";
    QByteArray kw = getContacts()->fromUnicode(NULL, text);
    for (const char *p = kw; *p; p++){
        if ((*p <= ' ') || (*p == '&') || (*p == '=')){
            char b[5];
            sprintf(b, "%%%02X", *p & 0xFF);
            url += b;
            continue;
        }
        url += *p;
    }
    url += "&.sb=";
    url += QString::number(type);
    url += "&.g=";
    url += QString::number(getComboValue(cmbGender, genders));
    url += "&.ar=";
    url += QString::number(getComboValue(cmbAge, ages));
    url += "&.pg=y";
    fetch(url);
}

void YahooSearch::searchStop()
{
    stop();
}

void YahooSearch::searchMail(const QString&)
{
    emit searchDone(this);
}

void YahooSearch::searchName(const QString &first, const QString &last, const QString &nick)
{
    QString s = first;
    if (s.isEmpty())
        s = last;
    if (s.isEmpty())
        s = nick;
    if (s.isEmpty()){
        emit searchDone(this);
        return;
    }
    search(s, 2);
}

bool YahooSearch::done(unsigned code, Buffer &b, const QString &)
{
    if (code == 200){
        QStringList l;
        l.append("");
        l.append(i18n("Yahoo! ID"));
        l.append("gender");
        l.append(i18n("Gender"));
        l.append("age");
        l.append(i18n("Age"));
        l.append("location");
        l.append(i18n("Location"));
        emit setColumns(l, 0, this);
        QByteArray data;
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        while (b.readPos() < b.writePos()){
            b.scan("\x04", data);
            if (data.length() < 2)
                break;
            QByteArray id;
            id = data.mid(2);
            b.scan("\x04", data);
            QByteArray gender, age, location;
            b.scan("\x04", gender);
            b.scan("\x04", age);
            b.scan("\x04", location);
            b.scan("\x04", data);

            log(L_DEBUG, "%s %s", id.data(), data.data());

            QStringList l;
            l.append("Yahoo!_online");
            l.append(id);
            l.append(id);
            l.append(i18n(gender.constData()));
            l.append(age);
            l.append(getContacts()->toUnicode(NULL, location));
            addItem(l, this);
        }
    }
    emit searchDone(this);
    return false;
}

void YahooSearch::createContact(unsigned tmpFlags, Contact *&contact)
{
    QString resource;
    QString name = edtID->text();
	createContact(name, tmpFlags, contact);
}

void YahooSearch::createContact(const QString &id, unsigned tmpFlags, Contact *&contact)
{
    if (m_client->findContact(id.toUtf8(), NULL, contact, false, false))
        return;
    QString grpName;
    Group *grp = NULL;
    ContactList::GroupIterator it;
    while ((grp = ++it) != NULL){
        if (grp->id())
            break;
    }
    if (grp)
        grpName = grp->getName();
    m_client->findContact(id.toUtf8(), getContacts()->fromUnicode(NULL, grpName), contact, false, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
}


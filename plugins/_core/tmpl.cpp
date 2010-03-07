/***************************************************************************
                          tmpl.cpp  -  description
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

#include <time.h>

#include "log.h"
#include "contacts/clientdataiterator.h"
#include "contacts/contact.h"
#include "contacts/client.h"

#include "core.h"

#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <QTimer>
#include <QDateTime>
#include <QProcess>

#include "tmpl.h"
#include "socket/ip.h"

using namespace std;
using namespace SIM;

Tmpl::Tmpl(QObject *parent)
        : QObject(parent)
{
}

Tmpl::~Tmpl()
{
}

bool Tmpl::processEvent(Event *e)
{
    if (e->type() == eEventTemplateExpand){
        EventTemplate *et = static_cast<EventTemplate*>(e);
        EventTemplate::TemplateExpand *t = et->templateExpand();
        TmplExpand tmpl;
        tmpl.tmpl = *t;
        tmpl.process = NULL;
        tmpl.bReady = false;
        if (!process(tmpl))
            tmpls.push_back(tmpl);
        return true;
    }
    return false;
}

void Tmpl::clear()
{
    for (QList<TmplExpand>::iterator it = tmpls.begin(); it != tmpls.end();){
        if (it->bReady && it->process){
            delete it->process;
            it->process = NULL;
            it->bReady = false;
            if (process(*it)){
                tmpls.erase(it);
                it = tmpls.begin();
                continue;
            }
            ++it;
        } else {
            ++it;
        }
    }
}

void Tmpl::ready()
{
    for (QList<TmplExpand>::iterator it = tmpls.begin(); it != tmpls.end(); ++it){
        QProcess *p = it->process;
        if (p && p->state() == QProcess::NotRunning){
            if (p->exitStatus() == QProcess::NormalExit){
                it->bReady = true;
                p->setReadChannel(QProcess::StandardOutput);
                it->res += QString::fromLocal8Bit(p->readAll());
                QTimer::singleShot(0, this, SLOT(clear()));
                return;
            }
        }
    }
}

bool Tmpl::process(TmplExpand &t)
{
    QString head = getToken(t.tmpl.tmpl, '`', false);
    t.res += process(t, head);
    if (t.tmpl.tmpl.isEmpty()){
        t.tmpl.tmpl = t.res;
        EventTemplateExpanded e(&t.tmpl);
        t.tmpl.receiver->processEvent(&e);
        e.setNoProcess();
        return true;
    }
    QString prg = getToken(t.tmpl.tmpl, '`', false);
    prg = process(t, prg);
    t.process = new QProcess(parent());
    connect(t.process, SIGNAL(processExited()), this, SLOT(ready()));
    t.process->start(prg);
    return false;
}

QString Tmpl::process(TmplExpand &t, const QString &str)
{
    QString res;
    QString s = str;
    while (!s.isEmpty()){
        res += getToken(s, '&');
        if(s.isEmpty())
            break;
        QString tag = getToken(s, ';');
        if (tag.isEmpty()) {
            res += tag;
            log(L_WARN, "Found '&' without ';' while parsing %s", qPrintable(str));
            continue;
        }
        Contact *contact;
        if (tag.startsWith("My")){
            contact = getContacts()->owner();
            tag = tag.mid(2);
        }else{
            contact = t.tmpl.contact;
        }

        if (contact == NULL)
            continue;

        if (tag == "TimeStatus"){
            QDateTime dt;
            dt.setTime_t(CorePlugin::instance()->value("StatusTime").toUInt());
            res += dt.toString("hh:mm");
            continue;
        }

        if (tag == "IntervalStatus"){
            res += QString::number(time(NULL) - CorePlugin::instance()->value("StatusTime").toUInt());
            continue;
        }

        if (tag == "IP"){
            EventGetContactIP e(contact);
            struct in_addr addr;
            e.process();
            if (e.ip())
                addr.s_addr = e.ip()->ip();
            else
                addr.s_addr = 0;
            res += inet_ntoa(addr);
            continue;
        }

        if (tag == "Mail"){
            QString mails = contact->getEMails();
            QString mail = getToken(mails, ';', false);
            res += getToken(mail, '/');
            continue;
        }

        if (tag == "Phone"){
            QString phones = contact->getPhones();
            QString phone_item = getToken(phones, ';', false);
            phone_item = getToken(phone_item, '/', false);
            res += getToken(phone_item, ',');
            continue;
        }

        if (tag == "Unread"){
            unsigned nUnread = 0;
            for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); ++it){
                if (it->contact == contact->id())
                    nUnread++;
            }
            res += QString::number(nUnread);
            continue;
        }

//        if (getTag(tag, &(contact->getGroup()), contact->dataDef(), res))
//            continue;

        clientData *data;
        ClientDataIterator itc(contact->clientData);
        while ((data = ++itc) != NULL){
            if (getTag(tag, &(data->Sign), itc.client()->protocol()->userDataDef(), res))
                break;
        }
        if (data)
            continue;

//        UserDataDef *def;
//        ContactList::UserDataIterator it;
//        while ((def = ++it) != NULL){
//            SIM::Data *data = (SIM::Data*)contact->getUserData(def->id);
//            if (data == NULL)
//                continue;
//            if (getTag(tag, data, def->def, res)){
//                break;
//            }
//        }
    }
    return res;
}

bool Tmpl::getTag(const QString &name, SIM::Data *data, const DataDef *def, QString &res)
{
    const DataDef *d;
    for (d = def; d->name; d++){
        if (name == d->name)
            break;
        data += d->n_values;
    }
    if (d->name == NULL)
        return false;

    switch (d->type){
    case DATA_BOOL:
        res += data->toBool() ? i18n("yes") : i18n("no");
        break;
    case DATA_ULONG:
        res += QString::number(data->toULong());
        break;
    case DATA_LONG:
        res += QString::number(data->toLong());
        break;
    case DATA_STRING:
    case DATA_UTF:
        if(data->str().isEmpty())
            return false;   // mabye we get a better one in the next contact
        res += data->str();
        break;
    case DATA_CSTRING:
        if(data->cstr().isEmpty())
            return false;   // mabye we get a better one in the next contact
        // this is not encoded correct, but no other way atm
        res += QString::fromLocal8Bit(data->cstr());
        break;
    default:
        break;
    }
    return true;
}

/*
#ifndef NO_MOC_INCLUDES
#include "tmpl.moc"
#endif
*/



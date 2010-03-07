/***************************************************************************
                          tmpl.h  -  description
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

#ifndef _TMPL_H
#define _TMPL_H

#include <QObject>
#include <QList>

#include "event.h"
#include "misc.h"
#include "core_events.h"

class QProcess;

class Tmpl : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    Tmpl(QObject *parent);
    ~Tmpl();
protected slots:
    void ready();
    void clear();
protected:
    struct TmplExpand
    {
        EventTemplate::TemplateExpand	tmpl;
        QProcess		*process;
        bool			bReady;
        QString			res;
    };
    virtual bool processEvent(SIM::Event*);
    bool process(TmplExpand &t);
    QString process(TmplExpand &t, const QString &str);
    bool getTag(const QString &name, SIM::Data *data, const SIM::DataDef *def, QString &res);
    QList<TmplExpand> tmpls;
};

#endif


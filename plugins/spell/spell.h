/***************************************************************************
                          spell.h  -  description
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

#ifndef _SPELL_H
#define _SPELL_H

#include "cfg.h"
#include "event.h"
#include "misc.h"
#include "plugins.h"
#include "propertyhub.h"

#include <QObject>
#include <QStringList>
#include <QEvent>
#include <QByteArray>

class QTextEdit;
class QSyntaxHighlighter;
class SpellerBase;
class Speller;

typedef QMap<SIM::my_string, bool> MAP_BOOL;

class SpellPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    SpellPlugin(unsigned, Buffer*);
    virtual ~SpellPlugin();

    void reset();

    unsigned CmdSpell;

    QStringList suggestions(const QString &word);
    void add(const QString &word);
    bool check(const QString &word);
    void ignore(const QString &word);

    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);

signals:
    void misspelling(const QString &word);
    void configChanged();

protected slots:
    void textEditFinished(QTextEdit*);
    void tempChildDestroyed(QObject*);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    virtual bool event( QEvent *e );
    virtual bool processEvent(SIM::Event *e);
    virtual QByteArray getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    void activate();
    void deactivate();
    bool            m_bActive;
    SpellerBase     *m_base;
    QList<Speller*> m_spellers;
    QList<QObject*> m_listTempChilds;
    QStringList m_listIgnore;

private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


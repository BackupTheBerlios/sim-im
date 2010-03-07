/***************************************************************************
                          spellhighlight.h  -  description
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

#ifndef _SPELLHIGHLIGHT_H
#define _SPELLHIGHLIGHT_H

#include "spell.h"

#include <QString>
#include <QSyntaxHighlighter>

class SpellHighlighter
    : public QSyntaxHighlighter
    , public SIM::EventReceiver
{
    Q_OBJECT
public:
    SpellHighlighter(QTextEdit *edit, SpellPlugin *m_plugin);
    virtual ~SpellHighlighter();

    virtual void highlightBlock( const QString &sText );

protected:
    virtual bool processEvent(SIM::Event *e);
    SpellPlugin *m_plugin;
    QStringList m_listSuggestions;
};

#endif


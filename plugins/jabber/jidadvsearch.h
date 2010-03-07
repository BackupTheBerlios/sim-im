/***************************************************************************
                          jidadvsearch.h  -  description
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

#ifndef _JIDADVSEARCH_H
#define _JIDADVSEARCH_H

#include "ui_jidadvsearchbase.h"

#include <QShowEvent>

class JIDAdvSearch : public QWidget, public Ui::JIDAdvSearch
{
    Q_OBJECT
public:
    JIDAdvSearch(QWidget *parent);
signals:
    void enableOptions(bool);
protected:
    void showEvent(QShowEvent*);
};

#endif


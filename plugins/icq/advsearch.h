/***************************************************************************
                          advsearch.h  -  description
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

#ifndef _ADVSEARCH_H
#define _ADVSEARCH_H

#include "ui_advsearchbase.h"
#include <QShowEvent>

class AdvSearch : public QWidget, public Ui::AdvSearch
{
    Q_OBJECT
public:
    AdvSearch();
signals:
    void enableOptions(bool);
protected:
    void showEvent(QShowEvent*);
};

#endif


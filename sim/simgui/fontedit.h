/***************************************************************************
                          fontedit.h  -  description
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

#ifndef _FONTEDIT_H
#define _FONTEDIT_H

#include "simapi.h"
#include <QFrame>
#include <QFont>
#include <QHBoxLayout>

class QLabel;

class EXPORT FontEdit : public QFrame
{
    Q_OBJECT
public:
    FontEdit(QWidget *parent);
    QString getFont();
    void setFont(const QString&);
    void setWinFont(const QFont&);
    const QFont &winFont() { return f; }
    static QFont str2font(const QString &font, const QFont &defFont);
    static QString font2str(const QFont &font, bool use_tr);
protected slots:
    void chooseFont();
protected:
    QFont f;
    QLabel *lblFont;
	QHBoxLayout *lay;
};

#endif


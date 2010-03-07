/***************************************************************************
                          transtop.h  -  description
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

#ifndef _TRANSTOP_H
#define _TRANSTOP_H


#include <qwidget.h>
#include <qpixmap.h>

class KRootPixmap;

class TransparentTop : public QObject
{
    Q_OBJECT
public:
    TransparentTop(QWidget *parent, unsigned transparent);
    ~TransparentTop();
    static void setTransparent(QWidget*, bool isTransparent, unsigned long transparency);
    static bool bCanTransparent;
    static TransparentTop *getTransparent(QWidget*);
    QPixmap background(const QColor &c);
    KRootPixmap *rootpixmap;
    void setTransparent(unsigned transparent);
    void transparentChanged();
protected slots:
    void backgroundUpdated( const QPixmap &pm );
protected:
    QPixmap bg;
    double m_transparent;
};

#endif


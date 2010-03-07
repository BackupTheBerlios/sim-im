/***************************************************************************
                          linklabel.h  -  description
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

#ifndef _LINKLABEL_H
#define _LINKLABEL_H

#include "simapi.h"

#include <QtGui/QLabel>
#include <QtCore/QUrl>

class EXPORT LinkLabel : public QLabel
{
    Q_OBJECT
public:
    LinkLabel(QWidget *parent = NULL);
    void setUrl(const QString &url);
protected:
    virtual void mouseReleaseEvent(QMouseEvent * e);
    QString m_url;
};

#endif


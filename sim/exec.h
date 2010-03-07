/***************************************************************************
                          exec.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _EXEC_H
#define _EXEC_H

#include <QObject>

#include "simapi.h"

class QTimer;

class EXPORT ExecManager : public QObject
{
    Q_OBJECT
public:
    ExecManager();
    ~ExecManager();
    static ExecManager *manager;
#ifndef WIN32
    QTimer *m_timer;
#endif
signals:
    void childExited(int, int);
public slots:
    void checkChilds();
};

#endif


/***************************************************************************
                          aboutdlg.h  -  description
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

#ifndef _ABOUTDLG_H
#define _ABOUTDLG_H

#include "simapi.h"
#include <QDialog>

#ifndef USE_KDE

#include "ui_aboutdlgbase.h"

class KAboutData;
class KAboutPerson;

class KAboutApplication : public QDialog, public Ui::AboutDlgBase
{
    Q_OBJECT
public:
    KAboutApplication(const KAboutData *aboutData, QWidget *parent=0, const char *name=0, bool modal=true);
    ~KAboutApplication();
signals:
    void finished();
protected:
    void closeEvent(QCloseEvent*);
    QString addPerson(const KAboutPerson*);
    QString quote(const QString &s);
};

#endif
#endif


/***************************************************************************
                          gpggen.h  -  description
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

#ifndef _GPGGEN_H
#define _GPGGEN_H

#include "ui_gpggenbase.h"

class QProcess;
class GpgCfg;

class GpgGen : public QDialog, public Ui::GpgGenBase
{
    Q_OBJECT
public:
    GpgGen(GpgCfg *parent);
    ~GpgGen();
protected slots:
    void textChanged(const QString&);
    void genKeyReady();
protected:
    void accept();
    QProcess *m_process;
    GpgCfg   *m_cfg;
};

#endif


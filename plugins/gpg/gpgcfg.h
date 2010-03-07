/***************************************************************************
                          gpgcfg.h  -  description
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

#ifndef _GPGCFG_H
#define _GPGCFG_H

#include "ui_gpgcfgbase.h"

class GpgPlugin;
class GpgAdvanced;
class QProcess;

#ifdef WIN32
class GpgFind;
#endif

class GpgCfg : public QWidget, public Ui::GpgCfgBase
{
    Q_OBJECT
public:
    GpgCfg(QWidget *parent, GpgPlugin *plugin);
    ~GpgCfg();
public slots:
    void apply();
    void refresh();
    void textChanged(const QString&);
    void find();
    void findFinished();
    void secretReady();
    void selectKey(int);
protected:
    void fillSecret(const QByteArray &ba = QByteArray());
    bool        m_bNew;
    QProcess   *m_process;
    GpgPlugin  *m_plugin;
#ifdef WIN32
    GpgFind    *m_find;
#endif
    GpgAdvanced *m_adv;
};

#endif


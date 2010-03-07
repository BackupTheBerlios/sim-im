/***************************************************************************
                          gpgfind.h  -  description
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

#ifndef _GPGFIND_H
#define _GPGFIND_H

#include "ui_gpgfindbase.h"

#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStack>

class EditFile;

class GpgFind : public QDialog, public Ui::GpgFindBase
{
    Q_OBJECT
public:
    GpgFind(EditFile *edt);
    ~GpgFind();
signals:
    void finished();
protected slots:
    void next();
protected:
    bool checkPath();
    QString                  m_path;
    QStack<QStringList> m_tree;
    QStack<unsigned>    m_pos;
    QFileInfoList            m_drives;
    QFileInfo               *m_drive;
    EditFile                *m_edit;
};

#endif


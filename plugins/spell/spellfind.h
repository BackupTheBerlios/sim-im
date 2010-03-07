/***************************************************************************
                          spellfind.h  -  description
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

#ifndef _SPELLFIND_H
#define _SPELLFIND_H

#include <stack>
#include <qdir.h>
#include "ui_spellfindbase.h"

class EditFile;

class SpellFind : public QDialog, public Ui::SpellFindBase
{
    Q_OBJECT
public:
    SpellFind(EditFile *edt);
    ~SpellFind();
signals:
    void finished();
protected slots:
    void next();
protected:
    bool checkPath();
    QString				 m_path;
    std::stack<QStringList>	m_tree;
    std::stack<unsigned>	m_pos;
    QFileInfoList		 m_drives;
    QFileInfo			 m_drive;
    EditFile			 *m_edit;
};

#endif


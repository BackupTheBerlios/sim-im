/***************************************************************************
                          gpgfind.cpp  -  description
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

#include <windows.h>

#include <QPixmap>
#include <QPushButton>
#include <QDir>
#include <QLabel>
#include <QTimer>
#include <QRegExp>

#include "simgui/editfile.h"
#include "icons.h"
#include "misc.h"

#include "gpgfind.h"

using namespace SIM;

GpgFind::GpgFind(EditFile *edt) : QDialog(NULL)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowIcon(Icon("find"));
    setButtonsPict(this);
    m_edit = edt;
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    //m_drives = *QDir::drives(); //portme
    //m_drive  = m_drives.first(); //portme
    QTimer::singleShot(0, this, SLOT(next()));
}

GpgFind::~GpgFind()
{
    emit finished();
}

void GpgFind::next()
{
    if (!m_tree.empty()){
        QStringList &subDirs = m_tree.top();
        unsigned pos = m_pos.top();
        if (pos >= (unsigned)subDirs.count()){
            m_tree.pop();
            m_pos.pop();
            m_path = m_path.left(m_path.length() - 1);
            m_path = m_path.left(m_path.lastIndexOf('\\') + 1);
            QTimer::singleShot(0, this, SLOT(next()));
            return;
        }
        QString subDir = subDirs[pos++];
        m_pos.pop();
        m_pos.push(pos);
        if (!subDir.startsWith(".")){
            m_path += subDir;
            m_path += '\\';
            if (checkPath())
                return;
        }
        QTimer::singleShot(0, this, SLOT(next()));
        return;
    }
    if (m_drive == NULL){
        close();
        return;
    }
    m_path = m_drive->absoluteFilePath();
    m_path = m_path.replace('/', '\\');
    if ((GetDriveTypeW((LPCWSTR)m_path.utf16()) == DRIVE_FIXED) && checkPath())
        return;
    //m_drive = m_drives.next(); //portme
    QTimer::singleShot(0, this, SLOT(next()));
}

bool GpgFind::checkPath()
{
    QDir d(m_path);
    if (!d.exists())
        return false;
    QString p = m_path;
    if (p.length() > 40){
        p = "...";
        p += m_path.mid(m_path.length() - 38);
    }
    lblPath->setText(p);
    QFile f(m_path + "gpg.exe");
    if (f.exists()){
        m_edit->setText(m_path + "gpg.exe");
        QTimer::singleShot(0, this, SLOT(close()));
        return true;
    }
    QStringList subDirs = d.entryList(QDir::Dirs);
    if (!subDirs.isEmpty()){
        m_tree.push(subDirs);
        m_pos.push(0);
    }else{
        m_path = m_path.left(m_path.lastIndexOf('\\'));
    }
    return false;
}


/***************************************************************************
                          maininfo.h  -  description
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

#ifndef _MAININFO_H
#define _MAININFO_H

#include "ui_maininfobase.h"
#include "event.h"
class ListViewItem;

// Lets do this later whole in one, because at the moment it only breaks linking of all other plugins...
#ifndef CORE_EXPORT
#   ifdef CORE_EXPORTS
#       define CORE_EXPORT Q_DECL_EXPORT
#   else // CORE_EXPORTS
#       define CORE_EXPORT Q_DECL_IMPORT
#   endif // CORE_EXPORTS
#endif // CORE_EXPORT

class CORE_EXPORT MainInfo : public QWidget, public Ui::MainInfo, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MainInfo(QWidget *parent, SIM::Contact *contact);
protected slots:
    void apply();
    void mailSelectionChanged();
    void phoneSelectionChanged();
    void deleteMail(QTreeWidgetItem *item);
    void deletePhone(QTreeWidgetItem *item);
    void editMail(QTreeWidgetItem *item);
    void editPhone(QTreeWidgetItem *item);
    void addMail();
    void editMail();
    void deleteMail();
    void addPhone();
    void editPhone();
    void deletePhone();
protected:
    void fill();
    virtual bool processEvent(SIM::Event*);
    void fillPhoneItem(QTreeWidgetItem *item, const QString &number, const QString &type, unsigned icon, const QString &proto);
    void fillCurrentCombo();
    void fillEncoding();
    void getEncoding(bool SendContactChangedEvent = true);
    bool m_bInit;
    SIM::Contact *m_contact;
};

#endif


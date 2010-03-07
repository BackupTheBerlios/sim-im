/***************************************************************************
                          usercfg.h  -  description
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

#ifndef _USERCFG_H
#define _USERCFG_H

#include "ui_cfgdlgbase.h"
#include "event.h"
#include <QResizeEvent>

class CorePlugin;

class UserConfig : public QDialog, public Ui::ConfigureDialogBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    UserConfig(SIM::Contact *conatct, SIM::Group *group);
    ~UserConfig();
    SIM::Contact *m_contact;
    SIM::Group   *m_group;
    bool raisePage(unsigned id);
    bool raiseDefaultPage();
    void showUpdate(bool);
signals:
    void applyChanges();
protected slots:
    void apply();
    void itemSelected(QTreeWidgetItem*, QTreeWidgetItem*);
    void updateInfo();
protected:
    virtual void accept();
    virtual bool processEvent(SIM::Event*);
    void resizeEvent(QResizeEvent*);
    void setTitle();
    void fill();
    unsigned m_nUpdates;
    unsigned m_defaultPage;
    bool raisePage(unsigned id, QTreeWidgetItem*);
    void removeCommand(unsigned id);
    bool removeCommand(unsigned id, QTreeWidgetItem*);
private:
	QTreeWidgetItem		*m_parentItem;
};

#endif


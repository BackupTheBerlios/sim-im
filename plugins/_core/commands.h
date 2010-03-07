/***************************************************************************
                          commands.h  -  description
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

#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <QObject>
#include <QEvent>

#include "event.h"

#include <map>

class CorePlugin;
class CMenu;

struct MenuDef
{
    SIM::CommandsDef *def;
    CMenu			*menu;
    void			*param;
};

typedef std::map<unsigned, SIM::CommandsDef*> CMDS_MAP;
typedef std::map<unsigned, MenuDef>		MENU_MAP;

class Commands : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    Commands();
    ~Commands();
    void set(SIM::CommandsDef*, const char *str);
    void clear();
protected slots:
    void popupActivated();
protected:
    bool eventFilter(QObject *o, QEvent *e);
    virtual bool processEvent(SIM::Event*);
    SIM::CommandsDef *createBar(unsigned id);
    void removeBar(unsigned id);
    SIM::CommandsDef *createMenu(unsigned id);
    void removeMenu(unsigned id);
    CToolBar *show(unsigned id, QMainWindow *parent);
    CMenu *get(SIM::CommandDef *cmd);
    SIM::CommandsDef *getDef(unsigned id);
    CMenu *processMenu(unsigned id, void *param, int key);
    void customize(SIM::CommandsDef *def);
    void customizeMenu(unsigned long id);
    QObject* getParent(QObject *o);
    unsigned cur_id;
    CMDS_MAP bars;
    MENU_MAP menus;
};

#endif


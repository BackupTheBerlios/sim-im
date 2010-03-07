/***************************************************************************
                          cmenu.h  -  description
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

#ifndef _CMENU_H
#define _CMENU_H

#include "simapi.h"

#include <vector>

#include "event.h"
#include <QMenu>

struct CMD
{
    unsigned	id;
    unsigned	base_id;
};

class CMenu : public QMenu
{
    Q_OBJECT
public:
    CMenu(SIM::CommandsDef *def);
    ~CMenu();
    void setParam(void *param);
protected slots:
    void showMenu();
    void hideMenu();
    void menuActivated(QAction *action);
    void clearMenu();
protected:
    void processItem(SIM::CommandDef *s, bool &bSeparator, bool &bFirst, unsigned base_id);
    QSize sizeHint() const;
    void initMenu();
    std::vector<CMD> m_cmds;
    SIM::CommandsDef *m_def;
    QMenu	*m_wrk;
    void *m_param;
    bool		m_bInit;
};

#endif


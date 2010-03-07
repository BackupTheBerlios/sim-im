/***************************************************************************
                              toolbar_history.cpp

     This file contains subroutine that creates toolbar for history window
                             -------------------
    begin                : Tue Nov 26 2008
    based on             : core.cpp of Sim-IM by Vladimir Shutoff
                           and Sim-IM team
    copyright            : (C) 2002 - 2004 Vladimir Shutoff
                           (C) 2004 - 2008 Sim-IM Development Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "core.h"

using namespace SIM;

void CorePlugin::createHistoryToolbar()
{
  EventToolbar(ToolBarHistory, EventToolbar::eAdd).process();
  Command cmd;

  cmd->id      = CmdHistoryDirection;
  cmd->text    = I18N_NOOP("&Direction");
  cmd->icon    = "1uparrow";
  cmd->icon_on = "1downarrow";
  cmd->bar_id  = ToolBarHistory;
  cmd->bar_grp = 0x2000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdHistoryFind;
  cmd->text    = I18N_NOOP("&Filter");
  cmd->icon    = "filter";
  cmd->icon_on = "filter";
  cmd->bar_id  = ToolBarHistory;
  cmd->bar_grp = 0x3000;
  cmd->flags   = BTN_COMBO_CHECK;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdHistoryPrev;
  cmd->text    = I18N_NOOP("&Previous page");
  cmd->icon    = "1leftarrow";
  cmd->icon_on = QString::null;
  cmd->bar_id  = ToolBarHistory;
  cmd->bar_grp = 0x5000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdHistoryNext;
  cmd->text    = I18N_NOOP("&Next page");
  cmd->icon    = "1rightarrow";
  cmd->bar_id  = ToolBarHistory;
  cmd->bar_grp = 0x5001;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdHistorySave;
  cmd->text    = I18N_NOOP("&Save as text");
  cmd->icon    = "filesave";
  cmd->accel   = "Ctrl+S";
  cmd->bar_id  = ToolBarHistory;
  cmd->bar_grp = 0x6000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdChangeEncoding; 
  cmd->text     = I18N_NOOP("Change &encoding");
  cmd->icon     = "encoding";
  cmd->menu_id  = 0;
  cmd->bar_id   = ToolBarHistory;
  cmd->bar_grp   = 0x8080;
  cmd->popup_id	 = MenuEncoding;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

}

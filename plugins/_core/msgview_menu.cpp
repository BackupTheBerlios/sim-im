/***************************************************************************
                              msgview_menu.cpp

  This file contains subroutine that creates MenuMsgView menu and add some
  of it's items. Items that created in other places are mentioned as a comments.
  MenuMsgView used as context menu for chat history in contaner window
  (chat-window) and as a context menu in history window. (Some menu items are
  visible only when used in history window (CmdDeleteMessage, CmdCutHistory))
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

void CorePlugin::createMenuMsgView()
{
  EventMenu(MenuMsgView, EventMenu::eAdd).process();

  Command cmd;

  cmd->id       = CmdMsgOpen;
  cmd->text     = I18N_NOOP("&Open message");
  cmd->icon     = "message";
  cmd->menu_id  = MenuMsgView;
  cmd->menu_grp = 0x1000;
  cmd->bar_id   = 0;
  cmd->bar_grp  = 0;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdMsgSpecial;
  cmd->text     = "_";
  cmd->icon     = QString::null;
  cmd->menu_id  = MenuMsgView;
  cmd->menu_grp = 0x1001;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdCopy;
  cmd->text     = I18N_NOOP("&Copy");
  cmd->accel    = "Ctrl+C";
  cmd->icon     = "editcopy";
  cmd->menu_id  = MenuMsgView;
  cmd->menu_grp = 0x2000;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdDeleteMessage;
  cmd->text     = I18N_NOOP("&Delete message");
  cmd->accel    = QString::null;
  cmd->icon     = "remove";
  cmd->menu_id  = MenuMsgView;
  cmd->menu_grp = 0x3000;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdCutHistory;
  cmd->text     = I18N_NOOP("&Cut history");
  cmd->icon     = "remove";
  cmd->menu_id  = MenuMsgView;
  cmd->menu_grp = 0x3001;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  /***** Ignore this phrase (CmdIgnoreText, grp=0x7000) *****/
  // This item should be created by filter plugin by FilterPlugin constructor but for 
  // some reason it is not seen at menu list
  // FIXME: Find out why 'Ignore this phrase' is not seen

  /***** Copy location (CmdCopyLocation, grp=0x7010) *****/
  // This menu item is created in plugins/navigate/navigate.cpp by NavigatePlugin constructor

}

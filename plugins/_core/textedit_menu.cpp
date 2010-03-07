/***************************************************************************
                              textedit_menu.cpp

  This file contains subroutine that creates MenuTextEdit menu and adds some
  of it's items. Items that created in other places are mentioned here as a
  comments.  MenuTextEdit is used as context menu in any multiline text input
  areas, including MsgEdit area, and also in some non-editable text view
  areas, witch is also based on such TextShow class, such as log view area in
  networkmonitor, etc.
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
#include "kdeisversion.h" // for KDE_IS_VERSION

using namespace SIM;

void CorePlugin::createMenuTextEdit()
{
  EventMenu(MenuTextEdit, EventMenu::eAdd).process();

  Command cmd;

  /***** ????????? (CmdSpell, grp=0x0100) *****/
  // Some strange menu item is created here by constructor of spell plugin
  // What does it do you should find out yourself

  cmd->id       = CmdUndo;
  cmd->text     = I18N_NOOP("&Undo");
  cmd->accel    = "Ctrl+Z";
  cmd->icon     = "undo";
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x1000;
  cmd->bar_id   = 0;
  cmd->bar_grp  = 0;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();


  cmd->id       = CmdRedo;
  cmd->text     = I18N_NOOP("&Redo");
  cmd->accel    = "Ctrl+Y";
  cmd->icon     = "redo";
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x1001;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdCut;
  cmd->text     = I18N_NOOP("Cu&t");
  cmd->icon     = "editcut";
  cmd->accel    = "Ctrl+X";
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x2000;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdCopy;
  cmd->text     =  I18N_NOOP("&Copy");
  cmd->icon     = "editcopy";
  cmd->accel    = "Ctrl+C";
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x2001;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdPaste;
  cmd->text     = I18N_NOOP("&Paste");
  cmd->icon     = "editpaste";
  cmd->accel    = "Ctrl+V";
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x2002;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdClear;
  cmd->text     = I18N_NOOP("Clear");
  cmd->icon     = QString::null;
  cmd->accel    = QString::null;
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x3000;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdSelectAll;
  cmd->text     = I18N_NOOP("Select All");
  cmd->icon     = QString::null;
  cmd->accel    = "Ctrl+A";
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x3001;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

#ifdef USE_KDE
#if KDE_IS_VERSION(3,2,0)
  cmd->id       = CmdEnableSpell;
  cmd->text     = I18N_NOOP("Enable spell check");
  cmd->icon     = QString::null; // TODO: Add KDE spellcheck icon here... may be slightly modified
  cmd->accel    = QString::null;
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x4000;
  cmd->flags    = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdSpell;
  cmd->text     = I18N_NOOP("Spell check"); 
  cmd->icon     = QString::null;  // TODO: Add KDE spellcheck icon here...
  cmd->menu_id  = MenuTextEdit;
  cmd->menu_grp = 0x4001;
  cmd->flags    = COMMAND_DEFAULT;
  EventCommandCreate(cmd).process();
#endif
#endif

  /***** Ignore this phrase (CmdIgnoreText, grp=0x7000) *****/
  // This menu item should be created by constructor of filter plugin, but for some reason
  // it is not visible, where it should be.
  // TODO: findout why 'Ignore this phrase' is always hidden

  /***** Copy &location (CmdCopyLocation, grp=0x7010 *****/
  // This menu item is created by constructor of navigate plugin
  // TODO: findout why this 'Copy Location' item is always visible at msgedit box, even when there is no http link there

}

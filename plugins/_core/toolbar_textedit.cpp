/***************************************************************************
                              toolbar_textedit.cpp

  This file contains subroutine that creates ToolBarTextEdit toolbar and
  fills that toolbar with toolbar items. This toolbar is used as a default
  toolbar for RichTextEdit (sim/textshow.cpp), but I (shaplov) have not
  found any plase where this toolbar is really shown, so:
  TODO: Check if this toolbar is really used, and may be remove it as unused code.
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
#include "simgui/textshow.h" // for CmdBgColor and all others...

using namespace SIM;

void CorePlugin::createTextEditToolbar()
{
  EventToolbar(ToolBarTextEdit, EventToolbar::eAdd).process();

  Command cmd;

  cmd->id      = CmdBgColor;
  cmd->text    = I18N_NOOP("Back&ground color");
  cmd->icon    = "bgcolor";
  cmd->icon_on = QString::null;
  cmd->bar_id  = ToolBarTextEdit;
  cmd->bar_grp = 0x1000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdFgColor;
  cmd->text    = I18N_NOOP("Fo&reground color");
  cmd->icon    = "fgcolor";
  cmd->bar_id  = ToolBarTextEdit;
  cmd->bar_grp = 0x1010;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdBold;
  cmd->text    = I18N_NOOP("&Bold");
  cmd->icon    = "text_bold";
  cmd->icon_on = "text_bold";
  cmd->bar_id  = ToolBarTextEdit;
  cmd->bar_grp = 0x2000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdItalic;
  cmd->text    = I18N_NOOP("It&alic");
  cmd->icon    = "text_italic";
  cmd->icon_on = "text_italic";
  cmd->bar_id  = ToolBarTextEdit;
  cmd->bar_grp = 0x2010;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdUnderline;
  cmd->text    = I18N_NOOP("&Underline");
  cmd->icon    = "text_under";
  cmd->icon_on = "text_under";
  cmd->bar_id  = ToolBarTextEdit;
  cmd->bar_grp = 0x2020;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdFont;
  cmd->text    = I18N_NOOP("Select f&ont");
  cmd->icon    = "text";
  cmd->icon_on = "text";
  cmd->bar_id  = ToolBarTextEdit;
  cmd->bar_grp = 0x3000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();
}

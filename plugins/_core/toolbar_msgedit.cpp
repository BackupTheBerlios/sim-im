/***************************************************************************
                              toolbar_msgedit.cpp

  This file contains subroutine that creates toolbar for MsgEdit part of
  chat window.
  Note that not all items of that menu created here. Some items is a result
  of some other actions. Such items only mentions as a comments here, with
  specifing where the item is relly created.
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
#include "icons.h"    // for getIcons(), getSmiles()
using namespace SIM;

void CorePlugin::createMsgEditToolbar()
{
  Command cmd;

  EventToolbar(ToolBarMsgEdit, EventToolbar::eAdd).process();

  cmd->id      = CmdBgColor;
  cmd->text    = I18N_NOOP("Back&ground color");
  cmd->icon    = "bgcolor";
  cmd->icon_on = QString::null;
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1000;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdFgColor;
  cmd->text    = I18N_NOOP("Fo&reground color");
  cmd->icon    = "fgcolor";
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1001;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdBold;
  cmd->text    = I18N_NOOP("&Bold");
  cmd->icon    = "text_bold";
  cmd->icon_on = "text_bold";
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1002;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdItalic;
  cmd->text    = I18N_NOOP("It&alic");
  cmd->icon    = "text_italic";
  cmd->icon_on = "text_italic";
  cmd->bar_id  = ToolBarMsgEdit; 
  cmd->bar_grp = 0x1003;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdUnderline;
  cmd->text    = I18N_NOOP("&Underline");
  cmd->icon    = "text_under";
  cmd->icon_on = "text_under";
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1004;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdFont;
  cmd->text    = I18N_NOOP("Select f&ont");
  cmd->icon    = "text";
  cmd->icon_on = QString::null;
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1005;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdFileName;
  cmd->text    = I18N_NOOP("Select &file");
  cmd->icon    = "file";
  cmd->icon_on = QString::null;
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1010;
  cmd->flags   = BTN_EDIT | COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdPhoneNumber;
  cmd->text    = I18N_NOOP("&Phone number");
  cmd->icon    = "cell";
  cmd->icon_on = QString::null;
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x1020;
  cmd->flags   = BTN_COMBO | BTN_NO_BUTTON | COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  /***** URL (CmdUrlInput, grp=0x1030) *****/
  // Toolbar item 'URL' is created in plugins/icq/icqmessage.cpp in function ICQPlugin::registerMessages()
  // And this item will exist even if icq plugin is not loaded. Because it were loaded while initing Sim-IM
  // and 'URL' menu item were created. Then it was not destroed somehow.
  // FIXME: may be 'URL' toolbar item _should_ be destroed when icq plugin is unloaded. Check it.

  // **** Quote (CmdMsgQuote + CmdReceived,  grp=0x1041) *****/
  // Theoreticly another toolbar item should be created here... It is "Quote" item, and it is created while
  // creating MenuMsgCommand. But (FIXME:) for some reason this item is missing when looking at item list
  // when costumising toolbar. Why I do not know....

  /***** Forward (CmdMsgForward + CmdReceived, grp=0x1042) *****/
  // Same as Quote. (FIXME: see fixme note for Quote few lines beforee)

  /***** Grant (CmdGrantAuth, grp=0x1080) *****/
  // Toolbar item is defined at msgedit.cpp in authRequestCommands structure and created at MsgEdit::setupMessages
  // while creating MessageAuthRequest. So make sure MsgEdit::setupMessages is called after createMsgEditToolbar()

  /***** Refuse (CmdRefuseAuth, grp=0x1081) *****/
  // Same as for Grant.

  /***** Accept (CmdFileAccept, grp=0x1090) *****/
  // Toolbar item is defined at msgedit.cpp in fileCommands structure and created at MsgEdit::setupMessages
  // while creating MessageFile. So make sure MsgEdit::setupMessages is called after createMsgEditToolbar()

  /***** Decline (CmdFileDecline, grp=0x1091) *****/
  // Same as for Accept.

  cmd->id      = CmdSmile;
  cmd->text    = I18N_NOOP("I&nsert smile");
  cmd->icon    = QString::null;
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x7000;
  cmd->flags   = COMMAND_CHECK_STATE;
  // Now checking are there any smile icons... if yes use one as item icon, if no hide 'Insert Smile' toolbar item
  QStringList smiles;
  getIcons()->getSmiles(smiles);
  if (smiles.empty()) cmd->flags |= BTN_HIDE;
    else cmd->icon = smiles.front();
  EventCommandCreate(cmd).process();

  cmd->id      = CmdTranslit;
  cmd->text    = I18N_NOOP("Send in &translit");
  cmd->icon    = "translit";
  cmd->icon_on = "translit";
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x7010;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id      = CmdSendClose;
  cmd->text    = I18N_NOOP("C&lose after send");
  cmd->icon    = "fileclose";
  cmd->icon_on = "fileclose";
  cmd->bar_id  = ToolBarMsgEdit;
  cmd->bar_grp = 0x7020;
  cmd->flags   = COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdSend;
  cmd->text     = I18N_NOOP("&Send");
  cmd->icon     = "mail_generic";
  cmd->icon_on  = QString::null;
  cmd->bar_id   = ToolBarMsgEdit;
  cmd->bar_grp  = 0x8000;
  cmd->flags    = BTN_PICT | COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  cmd->id       = CmdNextMessage;
  cmd->text     = I18N_NOOP("&Next");
  cmd->icon     = "message";
  cmd->bar_id   = ToolBarMsgEdit;
  cmd->bar_grp  = 0x8000;
  cmd->flags    = BTN_PICT | COMMAND_CHECK_STATE;
  EventCommandCreate(cmd).process();

  /***** Answer (CmdMsgAnswer grp= 0x8000) *****/
  // This toolbar item is created while creating MenuMsgCommand

  cmd->id       = CmdMultiply;
  cmd->text     = I18N_NOOP("Multi&ply send");
  cmd->icon     = "1rightarrow";
  cmd->icon_on  = "1leftarrow";
  cmd->bar_id   = ToolBarMsgEdit;
  cmd->bar_grp  = 0xF010;
  cmd->flags    = COMMAND_DEFAULT;
  EventCommandCreate(cmd).process();
}


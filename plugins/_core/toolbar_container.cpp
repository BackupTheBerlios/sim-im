/***************************************************************************
                              toolbar_container.cpp

  This file contains subroutine that creates toolbar and it's submenu
  for container window (One might call it chat-window, but in Sim-IM
  terminology it is calld container).
  Note that only static items are created here. Some items of MenuMessage
  are created by MsgEdit::setupMessages(); (msgedit.cpp) while creating
  message types. Some items somewhere else (I (shaplov) did not explore this
  issue yet)
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

void CorePlugin::createContainerToolbar()
{
	Command cmd;

	EventToolbar(ToolBarContainer, EventToolbar::eAdd).process();

	cmd->id       = CmdMessageType;
	cmd->text     = I18N_NOOP("Message");
	cmd->icon     = "message";
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0x2000;
	cmd->menu_id  = 0;
	cmd->menu_grp = 0;
	cmd->popup_id = MenuMessage;
	cmd->flags    = BTN_PICT;
	EventCommandCreate(cmd).process();

	cmd->id       = CmdContainerContact;
	cmd->text     = I18N_NOOP("Contact");
	cmd->icon     = "empty";
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0x6000;
	cmd->popup_id = MenuContainerContact;
	cmd->flags    = BTN_PICT;
	EventCommandCreate(cmd).process();

	cmd->id       = CmdContactGroup;
	cmd->text     = I18N_NOOP("Group");
	cmd->icon     = "grp_on";
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0x7000;
	cmd->popup_id = MenuContactGroup;
	cmd->flags    = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	cmd->id       = CmdInfo;
	cmd->text     = I18N_NOOP("User &info");
	cmd->icon     = "info";
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0x8000;
	cmd->popup_id = 0;
	cmd->flags    = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	cmd->id       = CmdHistory;
	cmd->text     = I18N_NOOP("&History");
	cmd->icon     = "history";
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0x8010;
	cmd->popup_id = 0;
	cmd->flags    = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	cmd->id       = CmdChangeEncoding;
	cmd->text     = I18N_NOOP("Change &encoding");
	cmd->icon     = "encoding";
	cmd->menu_id  = 0;
	cmd->menu_grp = 0;
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0x8080;
	cmd->popup_id = MenuEncoding;
	cmd->flags    = COMMAND_CHECK_STATE;

	EventCommandCreate(cmd).process();
	cmd->id       = CmdClose;
	cmd->text     = I18N_NOOP("Close");
	cmd->icon     = "exit";
	cmd->bar_id   = ToolBarContainer;
	cmd->bar_grp  = 0xF000;
	cmd->accel    = "Esc";
	cmd->flags    = COMMAND_DEFAULT;
	cmd->popup_id = 0; 
	EventCommandCreate(cmd).process();


	// First menu of this tootbar: list of message types that can be sent to the contact
	// displayed in chat window, and also list of different contact entities of meta-contact
	// (if we have meta-contact) also with types of messages can be send to particular contact.

	// Some items ot this menu are added by MsgEdit::setupMessages(); (msgedit.cpp), witch is called at
	// CorePlugin constructor, right after this function. MsgEdit::setupMessages() creates message
	// types and creatins of message types creates menu item in MenuMessage (strange idea, isn't it?)
	// And there are also plases where this menu is filled. Please use 'grep' to find them

	EventMenu(MenuMessage, EventMenu::eAdd).process();

	cmd->id       = CmdContactClients;
	cmd->text     = "_";
	cmd->icon     = "NULL";
	cmd->bar_id   = 0;
	cmd->menu_id  = MenuMessage;
	cmd->menu_grp = 0x30FF;
	cmd->accel    = QString::null;
	cmd->flags    = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();


	// Second menu of this toolbar: list of chats that are opened in this container
	// To see where this menu is filled please use 'grep CmdContainerContacts *'
	EventMenu(MenuContainerContact, EventMenu::eAdd).process();

	cmd->id       = CmdContainerContacts;
	cmd->text     = "_";
	cmd->icon     = QString::null; 
	cmd->menu_id  = MenuContainerContact;
	cmd->menu_grp = 0x1000;
	cmd->bar_id   = 0;
	cmd->bar_grp  = 0;
	cmd->flags    = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

}

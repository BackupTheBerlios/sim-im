/***************************************************************************
                              toolbar_main.cpp

  This file contains subroutines for creating and processing main window
                          toolbar and it's pull-down menus.
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

#include <QMenu>
#include "core.h"
#include "log.h"
#include "commands/commandhub.h"
#include "commands/uicommand.h"

using namespace SIM;

void CorePlugin::createGroupModeMenu()
{
    UiCommandPtr dontshow = UiCommand::create(I18N_NOOP("Do&n't show groups"), "grp_off", "groupmode_dontshow", QStringList("groupmode_menu"));
    UiCommandPtr mode1 = UiCommand::create(I18N_NOOP("Group mode 1"), "grp_on", "groupmode_mode1", QStringList("groupmode_menu"));
    UiCommandPtr mode2 = UiCommand::create(I18N_NOOP("Group mode 2"), "grp_on", "groupmode_mode2", QStringList("groupmode_menu"));
    UiCommandPtr groups = UiCommand::create(I18N_NOOP("&Groups"), value("GroupMode").toUInt() ? "grp_on" : "grp_off",
                                            "groupmode_menu", QStringList("main_toolbar"));
    groups->addSubCommand(dontshow);
    groups->addSubCommand(mode1);
    groups->addSubCommand(mode2);
    groups->addSubCommand(getCommandHub()->command("show_offline"));

    UiCommandPtr showEmptyGroups = UiCommand::create(I18N_NOOP("Show &empty groups"), QString(), "show_empty_groups", QStringList("groupmode_menu"));
    groups->addSubCommand(showEmptyGroups);

    UiCommandPtr createGroup = UiCommand::create(I18N_NOOP("&Create group"), "grp_create", "create_group", QStringList("groupmode_menu"));
    groups->addSubCommand(createGroup);
    groups->setWidgetType(UiCommand::wtButton);
    getCommandHub()->registerCommand(groups);
}

void CorePlugin::createMainToolbar()
{
    log(L_DEBUG, "createMainToolbar()");
    UiCommandPtr showOffline = UiCommand::create(I18N_NOOP("Show &offline"), "online_on", "show_offline", QStringList("main_toolbar"));
    showOffline->setWidgetType(UiCommand::wtButton);
    showOffline->setCheckable(true);
    if (value("ShowOnLine").toBool())
        showOffline->setChecked(true);
    getCommandHub()->registerCommand(showOffline);

    createGroupModeMenu();

    return;

	Command cmd;

	// **** Main ToolBar ****
	EventToolbar(ToolBarMain, EventToolbar::eAdd).process();

	cmd->id          = CmdOnline;
	cmd->text        = I18N_NOOP("Show &offline");
	cmd->icon        = "online_off";
	cmd->icon_on     = "online_on";
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0x4000;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0;
	if (value("ShowOnLine").toBool()) cmd->flags |= COMMAND_CHECKED;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdGroupToolbarButton;
	cmd->text        = I18N_NOOP("&Groups");
	cmd->icon        = value("GroupMode").toUInt() ? "grp_on" : "grp_off";
	cmd->icon_on     = QString::null;
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0x4000;
	cmd->menu_id     = 0;
	cmd->popup_id    = MenuGroups;
	EventCommandCreate(cmd).process();

	// Status toolbar item is created at status.cpp line 197. May be it should be moved here too...

	cmd->id          = CmdMenu;
	cmd->text        = I18N_NOOP("&Menu");
	cmd->icon        = "1downarrow";
	cmd->icon_on     = QString::null;
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0x8000;
	cmd->menu_id     = 0;
	cmd->menu_grp    = 0;
	cmd->popup_id    = MenuMain;
	cmd->flags       = 0;
	EventCommandCreate(cmd).process();

	// **** Main Menu ****

	EventMenu(MenuMain, EventMenu::eAdd).process();

	cmd->id          = CmdSearch;
	cmd->text        = I18N_NOOP("Search / Add contact");
	cmd->icon        = "find";
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x2080;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdSendSMS;
	cmd->text        = I18N_NOOP("&Send SMS");
	cmd->icon        = "sms";
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x2081;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id        = CmdUnread; 
	cmd->text      = I18N_NOOP("Unread messages");
	cmd->icon      = "message";
	cmd->bar_id    = 0;
	cmd->bar_grp   = 0;
	cmd->menu_id   = MenuMain;
	cmd->menu_grp  = 0x3000;
	cmd->flags     = COMMAND_IMPORTANT | COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	// Status menu item is created at status.cpp line 56. May be it should be moved here too...

	cmd->id          = CmdGroup;
	cmd->text        = I18N_NOOP("&Groups");
	cmd->icon        = "grp_on";
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x6001;
	cmd->popup_id    = MenuGroups;
	cmd->flags       = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdPhones;
	cmd->text        = I18N_NOOP("&Phone service");
	cmd->icon        = "phone";
	cmd->bar_id      = 0;
	cmd->bar_grp     = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x60F0;
	cmd->popup_id    = MenuPhones;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	// "Always on top" menu item is created by "ontop" plugin

	// "Network monitor" menu item is created by "netmonitor" plugin

	// "Connections" or "Connection manager" menu item (whitch one depends on number of
	// client plugins loaded) is created by CorePlugin::loadMenu() in core.cpp
	// May be should moved here once...

	cmd->id          = CmdConfigure;
	cmd->text        = I18N_NOOP("Setup");
	cmd->icon        = "configure";
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x8080;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	// "About KDE" menu item is created by "about" plugin if KDE is enabled

	// "Bug report / Requests" menu item is created by "about" plugin

	// "About Sim-IM" menu item is created by "about" plugin

	cmd->id          = CmdProfileChange;
	cmd->text        = I18N_NOOP("Change profile");
	cmd->icon        = QString::null;
	cmd->bar_id      = 0;
	cmd->bar_grp     = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x10040;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_DEFAULT; // May be COMMAND_IMPORTANT? Do we need this menu item in tray menu by default?
	EventCommandCreate(cmd).process();

	cmd->id          = CmdQuit;
	cmd->text        = I18N_NOOP("Quit");
	cmd->icon        = "exit";
	cmd->bar_id      = ToolBarMain;
	cmd->bar_grp     = 0;
	cmd->menu_id     = MenuMain;
	cmd->menu_grp    = 0x10080;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_IMPORTANT;
	EventCommandCreate(cmd).process();

	// **** Groups menu ****

	EventMenu(MenuGroups, EventMenu::eAdd).process();

	cmd->id          = CmdGrpOff;
	cmd->text        = I18N_NOOP("Do&n't show groups");
	cmd->icon        = "grp_off";
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuGroups;
	cmd->menu_grp    = 0x1000;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdGrpMode1;
	cmd->text        = I18N_NOOP("Group mode 1");
	cmd->icon        = "grp_on"; 
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuGroups;
	cmd->menu_grp    = 0x1001;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdGrpMode2;
	cmd->text        = I18N_NOOP("Group mode 2");
	cmd->icon        = "grp_on";  //TODO: Make icon for GroupMode2 independant from icon for GroupMode1
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuGroups;
	cmd->menu_grp    = 0x1002;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdOnline;
	cmd->text        = I18N_NOOP("Show &offline");
	cmd->icon        = "online_off";
	cmd->icon_on     = "online_on";
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuGroups;
	cmd->menu_grp    = 0x8000;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdEmptyGroup;
	cmd->text        = I18N_NOOP("Show &empty groups");
	cmd->icon        = QString::null;
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuGroups;
	cmd->menu_grp    = 0x8001;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdGrpCreate;
	cmd->text        = I18N_NOOP("&Create group");
	cmd->icon        = "grp_create";
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuGroups;
	cmd->menu_grp    = 0xA000;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	// **** Phone service submenu ****

	EventMenu(MenuPhones, EventMenu::eAdd).process();

	cmd->id          = CmdPhoneLocation;
	cmd->text        = I18N_NOOP("&Location");
	cmd->icon        = QString::null;
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhones;
	cmd->menu_grp    = 0x1000;
	cmd->popup_id    = MenuPhoneLocation;
	cmd->flags       = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	EventMenu(MenuPhoneLocation, EventMenu::eAdd).process();

	cmd->id          = CmdPhoneLocation; //FIXME: Is it ok that we have two CmdPhoneLocation menu items in different menus?
	cmd->text        = "_";
	cmd->icon        = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhoneLocation;
	cmd->menu_grp    = 0x1000;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdPhoneState;
	cmd->text        = I18N_NOOP("&Status");
	cmd->icon        = QString::null;
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhones;
	cmd->menu_grp    = 0x1010;
	cmd->popup_id    = MenuPhoneState;
	cmd->flags       = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();

	EventMenu(MenuPhoneState, EventMenu::eAdd).process();

	cmd->id          = CmdPhoneNoShow;
	cmd->text        = I18N_NOOP("&No show");
	cmd->icon        = QString::null;
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhoneState;
	cmd->menu_grp    = 0x1000;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdPhoneAvailable;
	cmd->text        = I18N_NOOP("&Available");
	cmd->icon        = "phone";
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhoneState;
	cmd->menu_grp    = 0x1001;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdPhoneBusy;
	cmd->text        = I18N_NOOP("&Busy");
	cmd->icon        = "nophone";
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhoneState;
	cmd->menu_grp    = 0x1002;
	cmd->flags       = COMMAND_CHECK_STATE;
	EventCommandCreate(cmd).process();

	cmd->id          = CmdPhoneBook;
	cmd->text        = I18N_NOOP("&Phone book");
	cmd->icon        = QString::null;
	cmd->icon_on     = QString::null;
	cmd->bar_id      = 0;
	cmd->menu_id     = MenuPhones;
	cmd->menu_grp    = 0x1020;
	cmd->popup_id    = 0;
	cmd->flags       = COMMAND_DEFAULT;
	EventCommandCreate(cmd).process();
}

bool CorePlugin::updateMainToolbar(unsigned long commandID)
{
  bool bUpdateAll = (commandID == ~((unsigned long) 0));

  if ( ( commandID == CmdGroupToolbarButton ) || bUpdateAll )
  {
    Command cmd;
    cmd->id          = CmdGroupToolbarButton;
    cmd->text        = I18N_NOOP("&Groups");
    cmd->icon        = value("GroupMode").toUInt() ? "grp_on" : "grp_off";
    cmd->bar_id      = ToolBarMain;
    cmd->bar_grp     = 0x4000;
    cmd->menu_id     = 0;
    cmd->menu_grp    = 0;
    cmd->popup_id    = MenuGroups;
    EventCommandChange(cmd).process();
  }

  if ( ( commandID == CmdOnline ) || bUpdateAll )
  {
    Command cmd;
    cmd->id          = CmdOnline;
    cmd->text        = I18N_NOOP("Show &offline");
    cmd->icon        = "online_off";
    cmd->icon_on     = "online_on";
    cmd->bar_id      = ToolBarMain;
    cmd->bar_grp     = 0x4000;
    cmd->menu_id     = MenuGroups;
    cmd->menu_grp    = 0x8000;
    cmd->flags       = COMMAND_CHECK_STATE;
    if (value("ShowOnLine").toBool()) cmd->flags |= COMMAND_CHECKED;
    EventCommandChange(cmd).process();
  }
  return bUpdateAll;
}

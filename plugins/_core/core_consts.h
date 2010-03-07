/***************************************************************************
                               core_consts.h

    This file defines constants for Command IDs, Menu IDs and ToolBar
    IDs.

    Command ID (const named  CmdXXXXX) is an unic int number which is
    associated with certain command. In most cases commands are
    associtated with menu(or toolbar) items, so one may say that CmdXXXXXX
    is an unique id of menu(or toolbar) item.

    Menu ID (const named MenuXXXXX) is a unique number which is
    associated with each menu(main menu, context menu, etc... any menu in
    Sim-IM has it's own Menu ID). It is used for creating and showing
    certain menu, and, as well, to specify pull down menu for certain menu
    item.

    ToolBar ID (const  namedToolBarXXXXXX) - an unique number that
    identifies certain toolbar.

                             -------------------
    begin                : Tue Nov 30 2008
    based on             : core.h of Sim-IM by Vladimir Shutoff
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

#ifndef _CORE_CONSTS_H
#define _CORE_CONSTS_H

const unsigned long CmdBase                 = 0x00020000;
const unsigned long CmdInfo                 = (CmdBase + 0x001);
const unsigned long CmdSearch               = (CmdBase + 0x002);
const unsigned long CmdConnections          = (CmdBase + 0x003);
const unsigned long CmdCM                   = (CmdBase + 0x004);
const unsigned long CmdProfileChange        = (CmdBase + 0x005);
const unsigned long CmdShowPanel            = (CmdBase + 0x006);
const unsigned long CmdCommonStatus         = (CmdBase + 0x007);
const unsigned long CmdTitle                = (CmdBase + 0x008);
const unsigned long CmdSetup                = (CmdBase + 0x009);
const unsigned long CmdMainWindow           = (CmdBase + 0x00B);	// 11
const unsigned long CmdUserView             = (CmdBase + 0x00C);	// 12
const unsigned long CmdContainer            = (CmdBase + 0x00D);	// 13
const unsigned long CmdClose                = (CmdBase + 0x00E);	// 14
const unsigned long CmdContainerContact     = (CmdBase + 0x00F);	// 15
const unsigned long CmdContainerContacts    = (CmdBase + 0x010);	// 16
const unsigned long CmdSendMessage          = (CmdBase + 0x011);	// 17
const unsigned long CmdSend                 = (CmdBase + 0x012);	// 18
const unsigned long CmdStatusMenu           = (CmdBase + 0x014);	// 20
const unsigned long CmdStatusBar            = (CmdBase + 0x015);	// 21
const unsigned long CmdMenu                 = (CmdBase + 0x017);	// 23
const unsigned long CmdOnline               = (CmdBase + 0x018);	// 24 //FIXME: Rename to CmdShowOfflineContacts to make code more sensible
const unsigned long CmdGroup                = (CmdBase + 0x019);	// 25
const unsigned long CmdGrpOff               = (CmdBase + 0x01A);	// 26
const unsigned long CmdGrpMode1             = (CmdBase + 0x01B);	// 27
const unsigned long CmdGrpMode2             = (CmdBase + 0x01C);	// 28
const unsigned long CmdGrpCreate            = (CmdBase + 0x01D);	// 29
const unsigned long CmdGrpRename            = (CmdBase + 0x01E);	// 30
const unsigned long CmdGrpDelete            = (CmdBase + 0x01F);	// 31
const unsigned long CmdGrpTitle             = (CmdBase + 0x020);	// 32
const unsigned long CmdGrpUp                = (CmdBase + 0x021);	// 33
const unsigned long CmdGrpDown              = (CmdBase + 0x022);	// 34
const unsigned long CmdContactTitle         = (CmdBase + 0x023);	// 35
const unsigned long CmdContactRename        = (CmdBase + 0x024);	// 36
const unsigned long CmdContactDelete        = (CmdBase + 0x025);	// 37
const unsigned long CmdConfigure            = (CmdBase + 0x026);	// 38
const unsigned long CmdMessageType          = (CmdBase + 0x027);	// 39
const unsigned long CmdSendClose            = (CmdBase + 0x028);	// 40
const unsigned long CmdSmile                = (CmdBase + 0x029);	// 41
const unsigned long CmdMultiply             = (CmdBase + 0x02A);	// 42
const unsigned long CmdSendSMS              = (CmdBase + 0x02B);	// 43
const unsigned long CmdInvisible            = (CmdBase + 0x02C);	// 44
const unsigned long CmdHistory              = (CmdBase + 0x02D);	// 45
const unsigned long CmdHistorySave          = (CmdBase + 0x02E);	// 46
const unsigned long CmdHistoryDirection     = (CmdBase + 0x02F);	// 47
const unsigned long CmdHistoryNext          = (CmdBase + 0x030);	// 48
const unsigned long CmdHistoryPrev          = (CmdBase + 0x031);	// 49
const unsigned long CmdMsgOpen              = (CmdBase + 0x033);	// 51
const unsigned long CmdMsgQuote             = (CmdBase + 0x035);	// 53
const unsigned long CmdMsgAnswer            = (CmdBase + 0x036);	// 54
const unsigned long CmdMsgForward           = (CmdBase + 0x037);	// 55
const unsigned long CmdCopy                 = (CmdBase + 0x038);	// 56
const unsigned long CmdCut                  = (CmdBase + 0x039);	// 57
const unsigned long CmdPaste                = (CmdBase + 0x03A);	// 58
const unsigned long CmdSelectAll            = (CmdBase + 0x03B);	// 59
const unsigned long CmdUndo                 = (CmdBase + 0x03C);	// 60
const unsigned long CmdRedo                 = (CmdBase + 0x03D);	// 61
const unsigned long CmdClear                = (CmdBase + 0x03E);	// 62
const unsigned long CmdSeparate             = (CmdBase + 0x03F);	// 63
const unsigned long CmdNextMessage          = (CmdBase + 0x041);	// 65
const unsigned long CmdGrantAuth            = (CmdBase + 0x042);	// 66
const unsigned long CmdRefuseAuth           = (CmdBase + 0x043);	// 67
const unsigned long CmdPhones               = (CmdBase + 0x047);	// 71
const unsigned long CmdPhoneLocation        = (CmdBase + 0x048);	// 72
const unsigned long CmdPhoneState           = (CmdBase + 0x049);	// 73
const unsigned long CmdPhoneNoShow          = (CmdBase + 0x04A);	// 74
const unsigned long CmdPhoneAvailable       = (CmdBase + 0x04B);	// 75
const unsigned long CmdPhoneBusy            = (CmdBase + 0x04C);	// 76
const unsigned long CmdPhoneBook            = (CmdBase + 0x04D);	// 77
const unsigned long CmdShowAlways           = (CmdBase + 0x04E);	// 78
const unsigned long CmdFileAccept           = (CmdBase + 0x04F);	// 79
const unsigned long CmdFileDecline          = (CmdBase + 0x050);	// 80
const unsigned long CmdDeclineWithoutReason = (CmdBase + 0x051);	// 81
const unsigned long CmdDeclineReasonInput   = (CmdBase + 0x052);	// 82
const unsigned long CmdDeclineReasonBusy    = (CmdBase + 0x053);	// 83
const unsigned long CmdDeclineReasonLater   = (CmdBase + 0x054);	// 84
const unsigned long CmdHistoryFind          = (CmdBase + 0x055);	// 85
const unsigned long CmdFileName             = (CmdBase + 0x056);	// 86
const unsigned long CmdPhoneNumber          = (CmdBase + 0x057);	// 87
const unsigned long CmdTranslit             = (CmdBase + 0x058);	// 88
const unsigned long CmdUrlInput             = (CmdBase + 0x059);	// 89
const unsigned long CmdCutHistory           = (CmdBase + 0x05A);	// 90
const unsigned long CmdDeleteMessage        = (CmdBase + 0x05B);	// 91
const unsigned long CmdEditList             = (CmdBase + 0x05C);	// 92
const unsigned long CmdRemoveList           = (CmdBase + 0x05D);	// 93
const unsigned long CmdStatusWnd            = (CmdBase + 0x05E);	// 94
const unsigned long CmdEmptyGroup           = (CmdBase + 0x05F);	// 95
const unsigned long CmdEnableSpell          = (CmdBase + 0x060);	// 96
const unsigned long CmdSpell                = (CmdBase + 0x061);	// 97
const unsigned long CmdChangeEncoding       = (CmdBase + 0x062);	// 98
const unsigned long CmdAllEncodings         = (CmdBase + 0x063);	// 99
const unsigned long CmdSearchInfo           = (CmdBase + 0x065);	// 101
const unsigned long CmdSearchMsg            = (CmdBase + 0x066);	// 102
const unsigned long CmdSearchOptions        = (CmdBase + 0x067);	// 103
const unsigned long CmdFetchAway            = (CmdBase + 0x068);	// 104
const unsigned long CmdHistoryAvatar        = (CmdBase + 0x069);	// 105
const unsigned long CmdQuit                 = (CmdBase + 0x070);	// 106
const unsigned long CmdGroupToolbarButton   = (CmdBase + 0x071);	// 107

const unsigned long CmdContactGroup         = (CmdBase + 0x100);
const unsigned long CmdUnread               = (CmdBase + 0x200);
const unsigned long CmdContactClients       = (CmdBase + 0x300);
const unsigned long CmdMsgSpecial           = (CmdBase + 0x400);
const unsigned long CmdClient               = (CmdBase + 0x500);
const unsigned long CmdContactResource      = (CmdBase + 0x600);
const unsigned long CmdReceived             = 0x600;  //FIXME: Why it does not have CmdBase in it?

// These consts are really defined at textshow.h, but listed here
// as a comments, in order to have a full comand list in one file:
//
//const unsigned TextCmdBase  = 0x00030000;
//const unsigned CmdBgColor   = TextCmdBase;
//const unsigned CmdFgColor   = TextCmdBase + 1;
//const unsigned CmdBold      = TextCmdBase + 2;
//const unsigned CmdItalic    = TextCmdBase + 3;
//const unsigned CmdUnderline = TextCmdBase + 4;
//const unsigned CmdFont      = TextCmdBase + 5;

// *** Here we defines ids for all menus used in _core plugin ***

// Main menu and it's submenus
const unsigned MenuMain             = 0x0001;
const unsigned MenuStatus           = 0x0002;
const unsigned MenuGroups           = 0x0003;
const unsigned MenuPhones           = 0x0004;
const unsigned MenuPhoneLocation    = 0x0005;
const unsigned MenuPhoneState       = 0x0006;
const unsigned MenuConnections      = 0x0007;
// other menus
const unsigned MenuGroup            = 0x1001;  // Context menu for Group item in userlist
const unsigned MenuContact          = 0x1002;  // Context menu for Contact item in userlist
const unsigned MenuContactGroup     = 0x1003;
const unsigned MenuContainer        = 0x1004;
const unsigned MenuMessage          = 0x1005;
const unsigned MenuContainerContact = 0x1006;
const unsigned MenuMsgView          = 0x1007;
const unsigned MenuTextEdit         = 0x1008;
const unsigned MenuMsgCommand       = 0x1009;
const unsigned MenuFileDecline      = 0x100A;
const unsigned MenuMailList         = 0x100B;
const unsigned MenuPhoneList        = 0x100C;
const unsigned MenuStatusWnd        = 0x100D;
const unsigned MenuEncoding         = 0x100E;
const unsigned MenuSearchItem       = 0x100F;
const unsigned MenuSearchGroups     = 0x1010;
const unsigned MenuSearchOptions    = 0x1011;

// *** Here we defines ids for all toolbars used in _core plugin ***

const unsigned ToolBarMain          = 1;
const unsigned ToolBarContainer     = 2;
const unsigned ToolBarTextEdit      = 3;
const unsigned ToolBarMsgEdit       = 4;
const unsigned ToolBarHistory       = 5;
const unsigned ToolBarHistoryAvatar = 6;


#endif

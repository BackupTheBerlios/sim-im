/***************************************************************************
                          core.h  -  description
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

#ifndef _CORE_H
#define _CORE_H

#include <vector>
#include <map>
#include <list>

#include <QFont>
#include <QObject>
#include <QThread>
#include <QByteArray>

#include "misc.h"
#include "plugins.h"
#include "propertyhub.h"
#include "simapi.h"
#include "mainwin.h"
#include "core_api.h"

using namespace std;

//typedef map<QString, unsigned> MAP_TYPES;

//struct msg_id
//{
//    unsigned    id;
//    unsigned    contact;
//    unsigned    type;
//    QString     client;
//};

//class FileLock;
//class QWidget;
//class QTranslator;
//class QMimeSource;
//class Commands;
//class MainWindow;
//class UserView;
//class SearchDialog;
//class CommonStatus;
//class StatusWnd;
//class ConnectionManager;

//const unsigned CONTAINER_SIMPLE	= 0;
//const unsigned CONTAINER_NEW	= 1;
//const unsigned CONTAINER_GROUP	= 2;
//const unsigned CONTAINER_ALL	= 3;

//const unsigned CONTAINER_GRP	= 0x80000000;

//const unsigned char SORT_NONE	= 0;
//const unsigned char SORT_STATUS	= 1;
//const unsigned char SORT_ACTIVE	= 2;
//const unsigned char SORT_NAME	= 3;

//const unsigned NEW_MSG_NOOPEN	= 0;
//const unsigned NEW_MSG_MINIMIZE	= 1;
//const unsigned NEW_MSG_RAISE	= 2;

//#include "core_events.h"

//const unsigned	MESSAGE_DEFAULT		= 0x0000;
//const unsigned	MESSAGE_SILENT		= 0x0001;
//const unsigned	MESSAGE_HIDDEN		= 0x0002;
//const unsigned	MESSAGE_SENDONLY	= 0x0004;
//const unsigned	MESSAGE_INFO		= 0x0008;
//const unsigned  MESSAGE_SYSTEM		= 0x0010;
//const unsigned  MESSAGE_ERROR		= 0x0020;
//const unsigned	MESSAGE_CHILD		= 0x0040;

//const unsigned  MIN_INPUT_BAR_ID = 0x1010;
//const unsigned  MAX_INPUT_BAR_ID = 0x1500;

//const unsigned  STYLE_UNDER		= 1;
//const unsigned  STYLE_ITALIC	= 2;
//const unsigned  STYLE_STRIKE	= 4;

//class MsgEdit;

//struct MessageDef
//{
//    const SIM::CommandDef	*cmdReceived;
//    const SIM::CommandDef	*cmdSent;
//    unsigned			flags;
//    const char			*singular;
//    const char			*plural;
//    SIM::Message*		(*create)(Buffer *cfg);
//    QObject*			(*generate)(MsgEdit *edit, SIM::Message *msg);
//    SIM::Message*		(*drag)(QMimeSource*);
//};

//struct clientContact
//{
//    SIM::IMContact	*data;
//    SIM::Client	*client;
//    bool		bNew;
//};


//class Tmpl;
//class XSL;
//class BalloonMsg;

//class HistoryThread : public QThread
//{
//public:
//    virtual void run();
//    void set_id(unsigned id) {m_id=id;}
//    void set_Viewer(const QString &Viewer) {m_Viewer=Viewer;}
//protected:
//    unsigned m_id;
//    QString m_Viewer;
//};

class CommonStatus;

class CORE_EXPORT CorePlugin : public QObject, public SIM::Plugin
{
    Q_OBJECT
public:
    CorePlugin();
    virtual ~CorePlugin();
//    void setManualStatus(unsigned long status);
//    unsigned long getManualStatus();
//    unsigned getContainerMode();
//    void setContainerMode(unsigned);
//    void setRegNew(bool p_new) {m_RegNew=p_new;}
//    bool getRegNew() const {return m_RegNew;}
//    MainWindow *getMainWindow() { return m_main; };

//    SIM::CommandsMap	preferences;
//    SIM::CommandsMap	messageTypes;
//    MAP_TYPES	types;

//    list<msg_id>	unread;

//    QFont editFont;
//    SIM::Message *createMessage(const char *type, Buffer *cfg);
//    QString clientName(SIM::Client *client);

//    XSL	*historyXSL;
//    static CorePlugin* instance();

//    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
//    QVariant value(const QString& key);
//    void setValue(const QString& key, const QVariant& v);

//    void changeClientStatus(SIM::Client* client, const SIM::IMStatusPtr& status);

//    ContainerManager* containerManager() const;
    bool init();

//signals:
//    void modeChanged(int);
//protected slots:
//    void dialogFinished();
//    void dialogDestroy();
//    void managerFinished();
//    void destroyManager();
//    void selectProfile();
//    void checkHistory();
//    void alertFinished();
//    void focusDestroyed();
//    void showMain();
//    void postInit();
//    void ignoreEvents(bool i);
protected:
    void subscribeToEvents();
    void createCommands();
    void createMainMenuCommand();
//    virtual bool processEvent(SIM::Event*);
//    virtual QByteArray getConfig();
//    virtual QWidget *createConfigWindow(QWidget *parent);
//    void createEventCmds();
//    void showInfo(SIM::CommandDef *cmd);
//    void destroy();
//    void loadDir();
//    void loadClients(SIM::ClientList&);
//    void loadClients(const QString& profilename, SIM::ClientList&);
//    void loadMenu();
//    QString tsFile(const QString &lang);
//    SIM::ClientPtr loadClient(const QString &name, Buffer *cfg);
//    bool adjustClientItem(unsigned id, SIM::CommandDef *cmd);
//    void showPanel();
//    void hideWindows();
//    void changeProfile(const QString& profilename);
//    void installTranslator();
//    void removeTranslator();
//    void initData();
//    void loadUnread();
//    void clearUnread(unsigned contact_id);
//    SIM::Client* getClient(unsigned i);
//    void getWays(vector<clientContact> &ways, SIM::Contact *contact);
//    QString typeName(const QString &name);
//    void setAutoReplies();
//    bool lockProfile(const QString &profile, bool bSend = false);

//    void createMainToolbar();
//    void createGroupModeMenu();
//    bool updateMainToolbar(unsigned long commandID);
//    void createHistoryToolbar();
//    void createContainerToolbar();
//    void createMsgEditToolbar();
//    void createTextEditToolbar();
//    void createMenuMsgView();		// in msgview_menu.cpp
//    void createMenuTextEdit();		// in textedit_menu.cpp

//    void startLogin();

//    void prepareConfig();

//    bool                m_bInit;
//    QStringList         m_profiles;
//    QWidget             *m_cfg;
//    QWidget             *m_focus;
//    SearchDialog        *m_search;
//    QTranslator         *m_translator;
//    ConnectionManager   *m_manager;
//    CommonStatus        *m_status;
//    StatusWnd           *m_statusWnd;
//    unsigned            m_nClients;
//    unsigned            m_nClientsMenu;
//    unsigned            m_nResourceMenu;
//    MainWindow          *m_main;
//    BalloonMsg          *m_alert;
//    FileLock            *m_lock;
//    bool                m_RegNew;
//    Tmpl                *m_tmpl;
//    Commands            *m_cmds;
//    HistoryThread      *m_HistoryThread;

private slots:
    void eventInit();
    void eventQuit();

private:
    SIM::PropertyHubPtr m_propertyHub;
    MainWindow* m_main;
    CommonStatus* m_commonStatus;
//    bool m_bIgnoreEvents;
//    ContainerManager* m_containerManager;

//    // Event handlers:
//    bool processEventIconChanged();
//    bool processEventJoinAlert();
//    bool processEventGroup(SIM::Event* e);
//    bool processEventDeleteMessage(SIM::Event* e);
//    bool processEventRewriteMessage(SIM::Event* e);
//    bool processEventTmplHelp(SIM::Event* e);
//    bool processEventTmplHelpList(SIM::Event* e);
//    bool processEventARRequest(SIM::Event* e);
//    bool processEventSaveState(SIM::Event* e);
//    bool processEventPluginChanged(SIM::Event* e);
//    bool processEventHomeDir(SIM::Event* e);
//    bool processEventGetProfile(SIM::Event* e);
//    bool processEventAddPreferences(SIM::Event* e);
//    bool processEventRemovePreferences(SIM::Event* e);
//    bool processEventClientChanged(SIM::Event* e);
//    bool processEventCreateMessageType(SIM::Event* e);
//    bool processEventRemoveMessageType(SIM::Event* e);
//    bool processEventContact(SIM::Event* e);
//    bool processEventMessageAcked(SIM::Event* e);
//    bool processEventMessageDeleted(SIM::Event* e);
//    bool processEventMessageReceived(SIM::Event* e);
//    bool processEventSent(SIM::Event* e);
//    bool processEventDefaultAction(SIM::Event* e);
//    bool processEventLoadMessage(SIM::Event* e);
//    bool processEventOpenMessage(SIM::Event* e);
//    bool processEventCheckCommandState(SIM::Event* e);
//    bool processEventCommandExec(SIM::Event* e);
//    bool processEventGoURL(SIM::Event* e);

//    bool processCheckCmdChangeEncoding(SIM::CommandDef* cmd);
//    bool processCheckCmdAllEncodings(SIM::CommandDef* cmd);
//    bool processCheckCmdEnableSpell(SIM::CommandDef* cmd);
//    bool processCheckCmdSendClose(SIM::CommandDef* cmd);
//    bool processCheckCmdContactClients(SIM::CommandDef* cmd);
//    bool processCheckMenuContainer(SIM::CommandDef* cmd);
//    bool processCheckMenuMessage(SIM::CommandDef* cmd);
//    bool processCheckMenuMsgCommand(SIM::CommandDef* cmd);
//    bool processCheckCmdPhoneLocation(SIM::CommandDef* cmd);
//    bool processCheckCmdUnread(SIM::CommandDef* cmd);
//    bool processCheckCmdSendSMS(SIM::CommandDef* cmd);
//    bool processCheckCmdShowPanel(SIM::CommandDef* cmd);
//    bool processCheckCmdCommonStatus(SIM::CommandDef* cmd);

//    bool processExecMenuEncoding(SIM::CommandDef* cmd);
//    bool processExecMenuMessage(SIM::CommandDef* cmd);
//    bool processExecMenuMsgCommand(SIM::CommandDef* cmd);
//    bool processExecCmdGrantAuth(SIM::CommandDef* cmd);
//    bool processExecCmdRefuseAuth(SIM::CommandDef* cmd);
//    bool processExecCmdSeparate(SIM::CommandDef* cmd);
//    bool processExecCmdSendSMS(SIM::CommandDef* cmd);
//    bool processExecCmdHistory(SIM::CommandDef* cmd);
//    bool processExecCmdConfigure(SIM::CommandDef* cmd);
//    bool processExecCmdSearch(SIM::CommandDef* cmd);
//    bool processExecMenuPhoneState(SIM::CommandDef* cmd);
//    bool processExecMenuPhoneLocation(SIM::CommandDef* cmd);
//    bool processExecCmdSetup(SIM::CommandDef* cmd);
//    bool processExecCmdPhoneBook(SIM::CommandDef* cmd);
//    bool processExecCmdCommonStatus(SIM::CommandDef* cmd);

//    bool processStatusChange(int clientnum, SIM::CommandDef* cmd);

//    void createCommand(int id, const QString& text, const QString& icon, int menu_id,
//            int menu_grp, int bar_id, int bar_grp, int flags, const QString& accel);

//    friend class MainWindow;
//    friend class UserView;
//    friend class UserConfig;
//    friend class SearchDialog;
//    friend class LangConfig;
//    friend class CommonStatus;
//    friend class LoginDialog;
//    friend class StatusWnd;
//    friend class InterfaceConfig;
//    friend class ConnectionManager;
//    friend class Container;
//    friend class UserWnd;
//    friend class MsgEdit;
//    friend class HistoryWindow;
//    friend class Tmpl;
};

#define GET_CorePlugin() ( static_cast<CorePlugin*>(SIM::getPluginManager()->plugin("_core").data()) )

CorePlugin* getCorePlugin();

#endif

// vim: set expandtab:


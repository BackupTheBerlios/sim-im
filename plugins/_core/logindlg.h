/***************************************************************************
                          logindlg.h  -  description
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

#ifndef _LOGINDLG_H
#define _LOGINDLG_H

#include "buffer.h"
#include "clientlist.h"
#include "contacts/client.h"
#include "event.h"
#include "plugins.h"
#include "ui_logindlgbase.h"
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

class QLabel;
class QLineEdit;
class LinkLabel;

class LoginDialog : public QDialog, public Ui::LoginDialogBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    LoginDialog(bool bInit, SIM::ClientPtr client, const QString &msg, const QString &loginProfile);
    ~LoginDialog();
    bool isChanged() { return m_bProfileChanged; }
    SIM::ClientPtr client() { return m_client; }
	QString profile() { return m_profile; }
	bool isNewProfile() { return m_newProfile; }
    QString newProfileName() { return m_newProfileName; }

signals:
    void changeProfile(const QString& profilename);
    void loadClients();

protected slots:
    void saveToggled(bool);
    void profileChanged(int);
    void pswdChanged(const QString&);
    void profileDelete();
    void profileRename();
    void loginComplete();
    void adjust();
    void newNameChanged( const QString &text );

protected:
    virtual bool processEvent(SIM::Event*);
    virtual void closeEvent(QCloseEvent *e);
    virtual void accept();
    virtual void reject();
    void makeInputs(unsigned &row, SIM::ClientPtr client);
    void clearInputs();
    void fill();
    void startLogin();
    void stopLogin();
    void loadClients(const QString& profilename, SIM::ClientList&);
    SIM::ClientPtr loadClient(const QString &name, Buffer *cfg);

private:
    QString m_profile;
    QString m_loginProfile;
	bool m_newProfile;
    bool m_bLogin;
    bool m_bInit;
    bool m_bProfileChanged;
    QList<QLabel*>	picts;
    QList<QLabel*>	texts;
    QList<QLineEdit*>	passwords;
    QList<LinkLabel*>	links;
    QList<QFrame*>	lines;
    SIM::ClientPtr m_client;
    QString m_newProfileName;
    QList<SIM::PluginPtr> m_protocolPlugins;
	QLabel *m_pict;
	QVBoxLayout *m_vboxlayout;
	QHBoxLayout *m_hboxlayout;
	QLabel		*m_txt;
	QLineEdit	*m_edt;
	QFrame		*m_line;
	LinkLabel	*m_lnkHelp;
};

#endif

// vim: set expandtab:


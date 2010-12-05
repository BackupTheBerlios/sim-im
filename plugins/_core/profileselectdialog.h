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

#ifndef PROFILESELECTDIALOG_H
#define PROFILESELECTDIALOG_H

#include "buffer.h"
#include "contacts/client.h"
//#include "event.h"
#include "plugins.h"
#include "ui_profileselectdialog.h"
#include <QLabel>
#include <QLayout>
#include <QLineEdit>

class QLabel;
class QLineEdit;
class LinkLabel;

class ProfileSelectDialog : public QDialog
{
    Q_OBJECT
public:
    ProfileSelectDialog();
    ~ProfileSelectDialog();
	QString profile() { return m_profile; }

signals:
    void changeProfile(const QString& profilename);
    void loadClients();

protected slots:
    void saveToggled(bool);
    void profileChanged(int);
    void pswdChanged(const QString&);
    void profileDelete();
    void profileRename();
    void adjust();
    void newNameChanged( const QString &text );

protected:
    void makeInputs(const SIM::ClientPtr& client);
    void clearInputs();
    void updateProfilesList();

private:
    QString m_profile;

    struct ClientEntry
    {
        QLabel* picture;
        QLabel* text;
        QLineEdit* passwordEdit;
        QLabel* link;
    };
    QList<ClientEntry> m_clientEntries;

    QList<QFrame*> m_lines;
    QString m_newProfileName;
    QList<SIM::PluginPtr> m_protocolPlugins;

    Ui::ProfileSelectDialog* m_ui;
};

#endif

// vim: set expandtab:


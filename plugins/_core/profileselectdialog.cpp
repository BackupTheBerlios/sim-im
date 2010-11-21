/***************************************************************************
logindlg.cpp  -  description
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

#include "contacts/client.h"
#include "contacts/protocolmanager.h"
#include "clientmanager.h"
#include "core.h"
#include "imagestorage/imagestorage.h"
#include "log.h"
#include "profilemanager.h"
#include "simgui/ballonmsg.h"
#include "simgui/linklabel.h"
#include "profileselectdialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopWidget>
#include <QDir>
#include <QInputDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QTimer>

using namespace SIM;

ProfileSelectDialog::ProfileSelectDialog() 
	: QDialog(NULL)
	, m_ui ( new Ui::ProfileSelectDialog )
{
    m_ui->setupUi(this);
    QSettings settings;
    m_profile = settings.value("Profile").toString();

    setWindowTitle(i18n("Select profile"));
    setWindowIcon(getImageStorage()->icon("SIM"));

    updateProfilesList();

    m_ui->chkSave->setChecked(settings.value("SavePasswd").toBool());
    m_ui->chkNoShow->setChecked(settings.value("NoShow").toBool());
    saveToggled(settings.value("SavePasswd").toBool());

    m_ui->labelNew->hide();
    m_ui->e_newName->hide();
    profileChanged(m_ui->cmbProfile->currentIndex());

    m_ui->cmbProfile->setFocus();
}

ProfileSelectDialog::~ProfileSelectDialog()
{
}

void ProfileSelectDialog::updateProfilesList()
{
    QStringList profiles = ProfileManager::instance()->enumProfiles();
    m_ui->cmbProfile->addItems(profiles);

    for(int i = 0; i < m_ui->cmbProfile->count(); i++)
    {
        if(m_ui->cmbProfile->itemText(i) == m_profile)
        {
            m_ui->cmbProfile->setCurrentIndex(i);
            break;
        }
    }

    m_ui->cmbProfile->addItem(i18n("New profile"));
}

void ProfileSelectDialog::saveToggled(bool bState)
{
    if (!bState)
        m_ui->chkNoShow->setChecked(false);
    m_ui->chkNoShow->setEnabled(bState);
}

void ProfileSelectDialog::profileChanged(int index)
{
    if (index < 0)
    {
        clearInputs();
        m_ui->buttonOk->setEnabled(false);
        m_ui->btnDelete->setEnabled(false);
        m_ui->btnRename->hide();
        return;
    }
    m_ui->buttonOk->setEnabled(true);
    if (index >= m_ui->cmbProfile->count() - 1)
    {
        m_ui->groupBoxPasswords->hide();
        clearInputs();
        m_ui->buttonOk->setEnabled(true);
        m_ui->btnDelete->setEnabled(false);
        m_ui->btnRename->hide();
        m_ui->labelNew->show();
        m_ui->e_newName->show();
        newNameChanged(m_ui->e_newName->text());
    }
    else
    {
        m_ui->btnRename->show();
        m_ui->labelNew->hide();
        m_ui->e_newName->hide();
        clearInputs();
        ProfileManager::instance()->selectProfile(m_ui->cmbProfile->currentText());
        getClientManager()->load();
        QStringList clients = getClientManager()->clientList();
		
		for (int i=0;i<clients.count();++i)
			log(L_DEBUG, "printing client %s", qPrintable(clients.at(i)));

        m_ui->groupBoxPasswords->show();

        foreach(const QString& clientName, clients)
        {
            log(L_DEBUG, "building profileselect dialog for client: %s", qPrintable(clientName));
            ClientPtr client = getClientManager()->client(clientName);
            if (client->protocol()->flag(Protocol::flNoAuth))
                continue;
            makeInputs(client);
        }
    }
    QTimer::singleShot(0, this, SLOT(adjust()));
}

void ProfileSelectDialog::adjust()
{
    int h = minimumSizeHint().height();
    resize(width(), h);
    move(x(), (qApp->desktop()->height() - h) / 2);
}

void ProfileSelectDialog::makeInputs(const ClientPtr& client)
{
    ClientEntry entry;
    QHBoxLayout* layout = new QHBoxLayout();
    m_ui->inputsLayout->addLayout(layout);

    entry.picture = new QLabel(m_ui->groupBoxPasswords);
    entry.picture->setPixmap(getImageStorage()->pixmap(client->protocol()->iconId()));
    layout->addWidget(entry.picture);

    entry.text = new QLabel(m_ui->groupBoxPasswords);
    entry.text->setText(client->name());
    entry.text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(entry.text);

    entry.passwordEdit = new QLineEdit(m_ui->groupBoxPasswords);
    entry.passwordEdit->setText(client->password());
    entry.passwordEdit->setEchoMode(QLineEdit::Password);
    connect(entry.passwordEdit, SIGNAL(textChanged(const QString&)), this, SLOT(pswdChanged(const QString&)));
    layout->addWidget(entry.passwordEdit);

    QString retrievePasswordLink = client->retrievePasswordLink();
    if (!retrievePasswordLink.isEmpty())
    {
        entry.link = new LinkLabel(m_ui->groupBoxPasswords);
        entry.link->setTextFormat(Qt::RichText);
        entry.link->setText(QString("<a href=\"%1\">").arg(retrievePasswordLink) + i18n("Forgot password?") + QString("</a>"));
        layout->addWidget(entry.link);
    }
    else
    {
        entry.link = 0;
    }

    QFrame* line = new QFrame(m_ui->groupBoxPasswords);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line);
    m_clientEntries.append(entry);
    m_lines.append(line);
}

void ProfileSelectDialog::clearInputs()
{
    foreach(const ClientEntry& entry, m_clientEntries)
    {
        delete entry.picture;
        delete entry.text;
        delete entry.passwordEdit;
        delete entry.link;
    }
    m_clientEntries.clear();

    qDeleteAll(m_lines);
    m_lines.clear();
}

void ProfileSelectDialog::pswdChanged(const QString&)
{
    bool hasEmptyPasswordField = false;
    for(int i = 0; i < m_clientEntries.size(); i++)
    {
        if (m_clientEntries[i].passwordEdit->text().isEmpty())
        {
            hasEmptyPasswordField = true;
            break;
        }
    }
    m_ui->buttonOk->setEnabled(!hasEmptyPasswordField);
}

void ProfileSelectDialog::profileDelete()
{
    int n = m_ui->cmbProfile->currentIndex();
    if ((n < 0) || (n >= m_ui->cmbProfile->count() - 1))
        return;

    QString curProfile = m_ui->cmbProfile->currentText();
    ProfileManager::instance()->removeProfile(curProfile);
    clearInputs();
    m_ui->btnDelete->setEnabled(false);
    updateProfilesList();
}

void ProfileSelectDialog::profileRename()
{
    QString old_name = m_ui->cmbProfile->currentText();

    QString name = old_name;
    while(1) {
        bool ok = false;
        name = QInputDialog::getText(this,i18n("Rename Profile"), i18n("Please enter a new name for the profile."),
                                     QLineEdit::Normal, name, &ok);
        if(!ok)
            return;
        if(ProfileManager::instance()->profileExists(name)) {
            QMessageBox::information(this, i18n("Rename Profile"), i18n("There is already another profile with this name.  Please choose another."), QMessageBox::Ok);
            continue;
        }
        else if(!ProfileManager::instance()->renameProfile(old_name, name)) {
            QMessageBox::information(this, i18n("Rename Profile"), i18n("Unable to rename the profile.  Please do not use any special characters."), QMessageBox::Ok);
            continue;
        }
        break;
    }
    updateProfilesList();
}

void ProfileSelectDialog::newNameChanged(const QString &text)
{
    if(text.isEmpty()) {
        m_ui->buttonOk->setEnabled(false);
        return;
    }
    m_ui->buttonOk->setEnabled(!ProfileManager::instance()->profileExists(text));
}

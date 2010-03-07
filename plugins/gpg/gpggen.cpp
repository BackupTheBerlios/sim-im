/***************************************************************************
                          gpggen.cpp  -  description
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

#include "misc.h"
#include "icons.h"

#include "gpggen.h"
#include "gpgcfg.h"
#include "gpg.h"

#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"
#include "contacts/group.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFile>
#include <QProcess>

using namespace SIM;

GpgGen::GpgGen(GpgCfg *cfg) : QDialog()
{
    setupUi(this);
    setModal(true);
    SET_WNDPROC("genkey")
    setWindowIcon(Icon("encrypted"));
    setButtonsPict(this);
    cmbMail->setEditable(true);
    m_process = NULL;
    m_cfg  = cfg;
    connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtPass1, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtPass2, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbMail->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    Contact *owner = getContacts()->owner();
    if (owner){
        QString name;
        name = owner->getFirstName();
        QString firstName = getToken(name, '/');
        name  = owner->getLastName();
        QString lastName  = getToken(name, '/');

        if (firstName.isEmpty() || lastName.isEmpty()){
            name = firstName + lastName;
        }else{
            name = firstName + ' ' + lastName;
        }
        edtName->setText(name);
        QString mails = owner->getEMails();
        while (!mails.isEmpty()){
            QString item = getToken(mails, ';');
            QString mail = getToken(item, '/');
            cmbMail->addItem(mail);
        }
    }
}

GpgGen::~GpgGen()
{
    delete m_process;
}

void GpgGen::textChanged(const QString&)
{
    buttonOk->setEnabled(!edtName->text().isEmpty() &&
                         !cmbMail->lineEdit()->text().isEmpty() &&
                         (edtPass1->text() == edtPass2->text()));
}

#ifdef WIN32
	#define CRLF	"\r\n"
#else
	#define CRLF	"\n"
#endif

void GpgGen::accept()
{
    edtName->setEnabled(false);
    cmbMail->setEnabled(false);
    edtComment->setEnabled(false);
    buttonOk->setEnabled(false);
#ifdef WIN32
    QString gpg  = m_cfg->edtGPG->text();
#else
    QString gpg  = GpgPlugin::plugin->GPG();
#endif
    QString home = m_cfg->edtHome->text();
    if (gpg.isEmpty() || home.isEmpty())
        return;
    lblProcess->setText(i18n("Move mouse for generate random key"));
    if (home.endsWith("\\") || home.endsWith("/"))
        home = home.left(home.length() - 1);
    QString in =
        "Key-Type: 1" CRLF
        "Key-Length: 1024" CRLF
        "Expire-Date: 0" CRLF
        "Name-Real: ";
    in += edtName->text();
    in += CRLF;
    if (!edtComment->text().isEmpty()){
        in += "Name-Comment: ";
        in += edtComment->text();
        in += CRLF;
    }
    in += "Name-Email: ";
    in += cmbMail->lineEdit()->text();
    in += CRLF;
    if (!edtPass1->text().isEmpty()){
        in += "Passphrase: ";
        in += edtPass1->text();
        in += CRLF;
    }
    QString fname = user_file("keys/genkey.txt");
    QFile f(fname);
    f.open(QIODevice::WriteOnly | QIODevice::Truncate);
    f.write(in.toUtf8());
    f.close();

    QStringList sl;
    sl += gpg;
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += GpgPlugin::plugin->value("GenKey").toString().split(' ');
    sl += fname;

    delete m_process;	// to be sure...
    m_process = new QProcess(this);

    connect(m_process, SIGNAL(finished()), this, SLOT(genKeyReady()));

    m_process->start(sl.join(" "));
}

void GpgGen::genKeyReady()
{
	QFile::remove(user_file("keys/genkey.txt"));
	if(m_process->exitStatus() == QProcess::NormalExit && m_process->exitCode() == 0)
	{
		QDialog::accept();
	}
	else
	{
		QByteArray ba1, ba2;
		m_process->setReadChannel(QProcess::StandardError);
		ba1 = m_process->readAll();
		m_process->setReadChannel(QProcess::StandardOutput);
		ba2 = m_process->readAll();
		QString s(" (");
		if (!ba1.isEmpty())
			s += QString::fromLocal8Bit(ba1.data(), ba1.size());
		if (!ba2.isEmpty()) {
			if(!s.isEmpty())
				s += ' ';
			s += QString::fromLocal8Bit(ba2.data(), ba2.size());
		}
		s += ')';
		if(s == " ()")
			s = QString::null;
		edtName->setEnabled(true);
		cmbMail->setEnabled(true);
		edtComment->setEnabled(true);
		lblProcess->setText(QString::null);
		buttonOk->setEnabled(true);
		BalloonMsg::message(i18n("Generate key failed") + s, buttonOk);
	}
	delete m_process;
	m_process = 0;
}


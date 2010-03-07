/***************************************************************************
                          migratedlg.cpp  -  description
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

#include <QLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QLabel>
#include <QApplication>
#include <QTextCodec>
#include <QByteArray>
#include <QCloseEvent>
#include <QVBoxLayout>

#include "buffer.h"
#include "unquot.h"
#include "misc.h"

#include "migratedlg.h"

#include "simgui/ballonmsg.h"



using namespace std;
using namespace SIM;

MigrateDialog::MigrateDialog(const QString &dir, const QStringList &cnvDirs) : QWizard(NULL)
        //: MigrateDialogBase(NULL, "migrate", true)
{
	setupUi(this);
    m_dir      = dir;
    m_cnvDirs  = cnvDirs;
    m_bProcess = false;
    QVBoxLayout *lay = (QVBoxLayout*)(page1->layout());
    for (QStringList::Iterator it = m_cnvDirs.begin(); it != m_cnvDirs.end(); ++it){
        QCheckBox *chk = new QCheckBox(*it, page1);
        lay->insertWidget(1, chk);
        chk->show();
        chk->setChecked(true);
        m_boxes.push_back(chk);
    }
    chkRemove->setChecked(true);
    connect(this, SIGNAL(selected(const QString&)), this, SLOT(pageSelected(const QString&)));
    button( QWizard::HelpButton )->hide();
}

void MigrateDialog::closeEvent(QCloseEvent *e)
{
    if (!m_bProcess){
        QWizard::closeEvent(e);
        return;
    }
    e->ignore();
    ask();
}

void MigrateDialog::reject()
{
    if (!m_bProcess){
        QWizard::reject();
        return;
    }
    ask();
}

void MigrateDialog::ask()
{
    QAbstractButton *btn = button( QWizard::CancelButton );
    QPoint p = btn->mapToGlobal(QPoint(0, 0));
    QRect rc(p.x(), p.y(), btn->width(), btn->height());
    BalloonMsg::ask(NULL, i18n("Cancel convert?"), this, SLOT(cancel(void*)), NULL, &rc);
}

void MigrateDialog::cancel(void*)
{
    m_bProcess = false;
    reject();
}

void MigrateDialog::pageSelected(const QString&)
{
    if (currentPage() != page2)
        return;
    button( QWizard::BackButton )->hide();
    button( QWizard::FinishButton )->setEnabled(false);
    list<QCheckBox*>::iterator it;
    for (it = m_boxes.begin(); it != m_boxes.end(); ++it){
        if ((*it)->isChecked()){
            m_bProcess = true;
            break;
        }
    }
    if (!m_bProcess){
        reject();
        return;
    }
    unsigned totalSize = 0;
    for (it = m_boxes.begin(); it != m_boxes.end(); ++it){
        if (!(*it)->isChecked())
            continue;
        QString path = user_file((*it)->text());
        path += '/';
        QFile icq_conf(path + "icq.conf");
        totalSize += icq_conf.size();
        QString history_path = path + "history";
        history_path += '/';
        QDir history(history_path);
        QStringList l = history.entryList(QStringList("*.history"), QDir::Files);
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
            QFile hf(history_path + (*it));
            totalSize += hf.size();
        }
    }
    barCnv->setMaximum(totalSize);
    QTimer::singleShot(0, this, SLOT(process()));
}

void MigrateDialog::error(const QString &str)
{
    lblStatus->setText(str);
    barCnv->hide();
    button( QWizard::FinishButton )->setEnabled(true);
    m_bProcess = false;
}

void MigrateDialog::process()
{
    unsigned size = 0;
    for (list<QCheckBox*>::iterator it = m_boxes.begin(); it != m_boxes.end(); ++it){
        if (!(*it)->isChecked())
            continue;
        QString path = user_file((*it)->text());
        path += '/';
        icqConf.close();
        clientsConf.close();
        contactsConf.close();
        icqConf.setFileName(path + "icq.conf");
        clientsConf.setFileName(path + "clients.conf");
        contactsConf.setFileName(path + "contacts.conf");
        lblStatus->setText(path + "icq.conf");
        if (!icqConf.open(QIODevice::ReadOnly)){
            error(i18n("Can't open %1") .arg(path + "icq.conf"));
            return;
        }
        if (!clientsConf.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            error(i18n("Can't open %1") .arg(path + "clients.conf"));
            return;
        }
        if (!contactsConf.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            error(i18n("Can't open %1") .arg(path + "contacts.conf"));
            return;
        }
        m_uin    = 0;
        m_passwd = "";
        m_state  = 0;
        m_grpId		= 0;
        m_contactId = 0;
        Buffer cfg;
        cfg.init(icqConf.size());
        icqConf.read(cfg.data(), icqConf.size());
        for (;;){
            QByteArray section = cfg.getSection();
            if (section.isEmpty())
                break;
            m_state = 3;
            if (section == "Group")
                m_state = 1;
            if (section == "User")
                m_state = 2;
            if (!m_bProcess)
                return;
            for (;;){
                QByteArray l = cfg.getLine();
                if (l.isEmpty())
                    break;
                QByteArray line = l;
                QByteArray name = getToken(line, '=');
                if (name == "UIN")
                    m_uin = line.toUInt();
                if (name == "EncryptPassword")
                    m_passwd = line;
                if (name == "Name")
                    m_name = line;
                if (name == "Alias")
                    m_name = line;
            }
            flush();
            barCnv->setValue(cfg.readPos());
            qApp->processEvents();
        }
        icqConf.close();
        clientsConf.close();
        contactsConf.close();
        m_state = 3;
        size += icqConf.size();
        if (!m_bProcess)
            return;
        barCnv->setValue(size);
        qApp->processEvents();
        QString h_path = path;
#ifdef WIN32
        h_path += "history\\";
#else
        h_path += "history/";
#endif
        QDir history(h_path);
        QStringList l = history.entryList(QStringList("*.history"), QDir::Files);
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
            hFrom.close();
            hTo.close();
            hFrom.setFileName(h_path + (*it));
            lblStatus->setText(h_path + (*it));
            hTo.setFileName(h_path + QString(m_owner) + '.' + it->left(it->indexOf('.')));
            if (!hFrom.open(QIODevice::ReadOnly)){
                error(i18n("Can't open %1") .arg(hFrom.fileName()));
                return;
            }
            if (!hTo.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                error(i18n("Can't open %1") .arg(hTo.fileName()));
                return;
            }
            cfg.init(hFrom.size());
            hFrom.read(cfg.data(), hFrom.size());
            for (;;){
                QByteArray section = cfg.getSection();
                if (section.isEmpty())
                    break;
                m_state = 3;
                if (section == "Message")
                    m_state = 4;
                if (!m_bProcess)
                    return;
                for (;;){
                    QByteArray l = cfg.getLine();
                    if (l.isEmpty())
                        break;
                    QByteArray line = l;
                    QByteArray name = getToken(line, '=');
                    if (name == "Message")
                        m_message = line;
                    if (name == "Time")
                        m_time = line;
                    if (name == "Direction")
                        m_direction = line;
                    if (name == "Charset")
                        m_charset = line;
                }
                flush();
                barCnv->setValue(cfg.readPos());
                qApp->processEvents();
            }
            hFrom.close();
            hTo.close();
            m_state = 3;
            size += hFrom.size();
            if (!m_bProcess)
                return;
            barCnv->setValue(size);
            qApp->processEvents();
        }
        if (chkRemove->isChecked()){
            icqConf.remove();
            icqConf.setFileName(path + "sim.conf");
            icqConf.remove();
            for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
                hFrom.setFileName(h_path + (*it));
                hFrom.remove();
            }
        }
    }
    m_bProcess = false;
    accept();
}

void MigrateDialog::flush()
{
    QByteArray output;
    switch (m_state){
    case 0:
        output = "[icq/ICQ]\n";
        clientsConf.write(output, output.length());
        output = "Uin=";
        output += QByteArray::number(m_uin);
        output += "\n";
        if (!m_passwd.isEmpty()){
            m_passwd = unquoteString(m_passwd).toUtf8();
            unsigned char xor_table[] =
                {
                    0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
                    0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c
                };
            for (int i = 0; i < (int)m_passwd.length(); i++)
                m_passwd[i] = (char)(m_passwd[i] ^ xor_table[i]);
            QByteArray new_passwd;
            unsigned short temp = 0x4345;
            for (int i = 0; i < (int)m_passwd.length(); i++) {
                temp ^= m_passwd[i];
                new_passwd += '$';
                char buff[8];
                sprintf(buff, "%x", temp);
                new_passwd += buff;
            }
            output += "Password=\"";
            output += new_passwd;
            output += "\"\n";
        }
        clientsConf.write(output, output.length());
        m_owner = "ICQ.";
        m_owner += QByteArray::number(m_uin);
        break;
    case 1:
        if (!m_name.isEmpty()){
            output = "[Group=";
            output += QByteArray::number(++m_grpId);
            output += "]\n";
            output += "Name=\"";
            output += m_name;
            output += "\"\n";
            contactsConf.write(output, output.length());
        }
        break;
    case 2:
        output = "[Contact=";
        output += QByteArray::number(++m_contactId);
        output += "]\n";
        if (m_uin >= 0x80000000)
            m_uin = 0;
        if (m_name.isEmpty())
            m_name = QByteArray::number(m_uin);
        if (!m_name.isEmpty()){
            output += "Name=\"";
            output += m_name;
            output += "\"\n";
        }
        if (m_uin){
            output += "[";
            output += m_owner;
            output += "]\n";
            output += "Uin=";
            output += QByteArray::number(m_uin);
            output += "\n";
        }
        contactsConf.write(output, output.length());
        break;
    case 4:
        if (!m_message.isEmpty()){
            QString msg = QString::fromLocal8Bit(m_message);
            if (!m_charset.isEmpty()){
                QTextCodec *codec = QTextCodec::codecForName(m_charset);
                if (codec)
                    msg = codec->toUnicode(m_message);
            }
            output = "[Message]\n";
            output += "Text=\"";
            output += quoteChars(msg, "\"", false).toLocal8Bit();
            output += "\"\n";
            if (m_direction.isEmpty()){
                output += "Flags=2\n";
            }else{
                output += "Flags=3\n";
            }
            output += "Time=";
            output += m_time;
            output += "\n";
            hTo.write(output, output.length());
        }
        break;
    }
    m_uin		= 0;
    m_passwd	= "";
    m_name		= "";
    m_message	= "";
    m_time		= "";
    m_direction = "";
    m_charset	= "";
}

/*
#ifndef NO_MOC_INCLUDES
#include "migratedlg.moc"
#endif
*/


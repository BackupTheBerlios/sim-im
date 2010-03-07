/***************************************************************************
                          gpgcfg.cpp  -  description
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

#include "gpg.h"
#include "gpgcfg.h"
#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"
#include "simgui/linklabel.h"
#ifdef WIN32
#include "gpgfind.h"
#endif
#include "gpgadv.h"
#include "gpggen.h"

#include <QTabWidget>
#include <QComboBox>
#include <QTimer>
#include <QProcess>
#include <QByteArray>

using namespace SIM;

GpgCfg::GpgCfg(QWidget *parent, GpgPlugin *plugin) : QWidget(parent)
{
    setupUi(this);
    m_plugin = plugin;
    m_process= NULL;
    m_bNew   = false;
#ifdef WIN32
    edtGPG->setText(m_plugin->GPG());
    edtGPG->setFilter(i18n("GPG(gpg.exe)"));
    m_find = NULL;
#else
    lblGPG->hide();
    edtGPG->hide();
#endif
    edtHome->setText(m_plugin->getHomeDir());
    edtHome->setDirMode(true);
    edtHome->setShowHidden(true);
    edtHome->setTitle(i18n("Select home directory"));
    lnkGPG->setUrl("http://www.gnupg.org/(en)/download/index.html");
    lnkGPG->setText(i18n("Download GPG"));
    connect(btnFind, SIGNAL(clicked()), this, SLOT(find()));
    connect(edtGPG, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    textChanged(edtGPG->text());
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        m_adv = new GpgAdvanced(tab, plugin);
        tab->addTab(m_adv, i18n("&Advanced"));
        tab->adjustSize();
        break;
    }
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    connect(cmbKey, SIGNAL(activated(int)), this, SLOT(selectKey(int)));
    fillSecret();
    refresh();
}

GpgCfg::~GpgCfg()
{
}

void GpgCfg::apply()
{
    QString key;
    int nKey = cmbKey->currentIndex();
    if (nKey && (nKey < cmbKey->count() - 1)){
        QString k = cmbKey->currentText();
        key = getToken(k, ' ');
    }
    m_plugin->setValue("Key", key);
#ifdef WIN32
    m_plugin->setValue("GPG", edtGPG->text());
#endif
    m_plugin->setValue("Home", edtHome->text());
    m_adv->apply();
    m_plugin->reset();
}

#ifdef WIN32
void GpgCfg::textChanged(const QString &str)
{
    if (str.isEmpty()){
        lnkGPG->show();
        btnFind->show();
    }else{
        lnkGPG->hide();
        btnFind->hide();
    }
}
#else
void GpgCfg::textChanged(const QString&)
{
    lnkGPG->hide();
    btnFind->hide();
}
#endif

void GpgCfg::find()
{
#ifdef WIN32
    if (m_find == NULL){
        m_find = new GpgFind(edtGPG);
        connect(m_find, SIGNAL(finished()), this, SLOT(findFinished()));
    }
    raiseWindow(m_find);
#endif
}

void GpgCfg::findFinished()
{
#ifdef WIN32
    m_find = NULL;
#endif
}

void GpgCfg::fillSecret(const QByteArray &ba)
{
    int cur = 0;
    int n   = 1;
    cmbKey->clear();
    cmbKey->addItem(i18n("None"));
    if (!ba.isEmpty()){
        QByteArray all(ba);
        for (;;){
            QByteArray line = getToken(all, '\n');
            if(line.isEmpty())
                break;
            QByteArray type = getToken(line, ':');
            if (type == "sec"){
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                QString sign = QString::fromLocal8Bit(getToken(line, ':'));
                if (sign == m_plugin->value("Key").toString())
                    cur = n;
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                QString name = QString::fromLocal8Bit(getToken(line, ':'));
                cmbKey->addItem(sign + QString(" - ") + name);
                n++;
            }
        }
    }
    cmbKey->addItem(i18n("New"));
    if (m_bNew){
        cur = cmbKey->count() - 2;
        m_bNew = false;
    }
    cmbKey->setCurrentIndex(cur);
}

void GpgCfg::refresh()
{
#ifdef WIN32
    QString gpg  = edtGPG->text();
#else
    QString gpg  = m_plugin->GPG();
#endif
    QString home = edtHome->text();

    if (gpg.isEmpty() || home.isEmpty()){
        fillSecret();
        return;
    }
    if (m_process)
        return;

    QStringList sl;
    sl += gpg;
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += GpgPlugin::plugin->value("SecretList").toString().split(' ');

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(processExited()), this, SLOT(secretReady()));
	m_process->start(sl.join(" "));
}

void GpgCfg::secretReady()
{
    if (m_process->exitStatus() == QProcess::NormalExit && m_process->exitCode() == 0) {
		m_process->setReadChannel(QProcess::StandardOutput);
        fillSecret(m_process->readAll());
    } else {
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
        BalloonMsg::message(i18n("Get secret list failed") + s, btnRefresh);
    }
    delete m_process;
    m_process = 0;
}

void GpgCfg::selectKey(int n)
{
    if (n == cmbKey->count() - 1){
        if(edtHome->text().isEmpty())
            edtHome->setText(m_plugin->getHomeDir());
        GpgGen gen(this);
        if (gen.exec()){
            m_bNew = true;
            QTimer::singleShot(0, this, SLOT(refresh()));
        }
    }
}

// vim: set expandtab:


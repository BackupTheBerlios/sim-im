/***************************************************************************
                          gpguser.h  -  description
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

#include "gpguser.h"
#include "gpg.h"


#include <QPushButton>
#include <QComboBox>
#include <QByteArray>
#include <QProcess>


using namespace SIM;

GpgUser::GpgUser(QWidget *parent, GpgUserData *data) : QWidget(parent)
{
	setupUi(this);
    if(data)
        m_key = data->Key.str();
    m_process = NULL;
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    refresh();
}

GpgUser::~GpgUser()
{
    delete m_process;
}

void GpgUser::apply(void *_data)
{
    GpgUserData *data = (GpgUserData*)_data;
    QString key;
    int nKey = cmbPublic->currentIndex();
    if (nKey && (nKey < cmbPublic->count())){
        QString k = cmbPublic->currentText();
        key = getToken(k, ' ');
    }
    data->Key.str() = key;
    if (key.isEmpty())
        data->Use.asBool() = false;
}

void GpgUser::refresh()
{
    if (m_process)
        return;
    QString gpg  = GpgPlugin::plugin->GPG();
    QString home = GpgPlugin::plugin->getHomeDir();
    if (gpg.isEmpty() || home.isEmpty())
        return;

    QStringList sl;
    sl += gpg;
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += GpgPlugin::plugin->value("PublicList").toString().split(' ');

    m_process = new QProcess(this);

    connect(m_process, SIGNAL(processExited()), this, SLOT(publicReady()));
    m_process->start(sl.join(" "));
}

void GpgUser::publicReady()
{
    int cur = 0;
    int n   = 1;
    cmbPublic->clear();
    cmbPublic->insertItem(INT_MAX,i18n("None"));
    if (m_process->exitStatus() == QProcess::NormalExit && m_process->exitCode() == 0){
		m_process->setReadChannel(QProcess::StandardOutput);
        QByteArray str(m_process->readAll());
        for (;;){
            QByteArray line;
            line = getToken(str, '\n');
            if(line.isEmpty())
                    break;
            QByteArray type = getToken(line, ':');
            if (type == "pub"){
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                QByteArray sign = getToken(line, ':');
                if (QString::fromLocal8Bit(sign) == m_key)
                    cur = n;
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                QByteArray name = getToken(line, ':');
                cmbPublic->insertItem(INT_MAX,QString::fromLocal8Bit(sign) + QString(" - ") +
                                      QString::fromLocal8Bit(name));
                n++;
            }
        }
    }
    cmbPublic->setCurrentIndex(cur);
    delete m_process;
    m_process = 0;
}


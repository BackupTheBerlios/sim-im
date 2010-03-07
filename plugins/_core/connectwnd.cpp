/***************************************************************************
                          connectwnd.cpp  -  description
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
#include "connectwnd.h"
#include "simgui/linklabel.h"

#include <QLabel>
#include <QMovie>
#include <QFile>

ConnectWnd::ConnectWnd(bool bStart) : QWizardPage(NULL)
{
    setupUi(this);
    using SIM::app_file;
    m_bStart = bStart;
    setConnecting(true);
    QMovie movie(app_file("pict/connect.gif"));
    if (!movie.isValid())
        movie.setFileName(app_file("pict/connect.mng"));
    if (movie.isValid()){
        lblMovie->setMovie(&movie);
        connect(this, SIGNAL(updated()), this, SLOT(updateMovie()));
        movie.stop();
        movie.start();
        updateMovie();
    }
    setConnecting(true);
}

void ConnectWnd::updateMovie()
{
    lblMovie->repaint();
}

void ConnectWnd::setConnecting(bool bState)
{
    lnkPass->hide();
    if (bState){
        lblConnect->show();
        QMovie *pMovie = lblMovie->movie();
        if(pMovie){
            pMovie->start();
        }
        lblMovie->show();
        lblComplete->hide();
        lblNext->hide();
        frmError->hide();
    }else{
        lblConnect->hide();
        QMovie *pMovie = lblMovie->movie();
        if(pMovie){
            pMovie->stop();
        }
        lblMovie->hide();
        lblComplete->show();
        if (m_bStart){
            lblNext->show();
        }else{
            lblNext->hide();
        }
        frmError->hide();
    }
}

void ConnectWnd::setErr(const QString &text, const QString &url)
{
    lblConnect->hide();
    lblMovie->hide();
    lblComplete->hide();
    lblNext->hide();
    lblError->setText(text);
    frmError->show();
    if (!url.isEmpty()){
        lnkPass->setUrl(url);
        lnkPass->setText(i18n("Forgot password?"));
        lnkPass->show();
    }else{
        lnkPass->hide();
    }
}

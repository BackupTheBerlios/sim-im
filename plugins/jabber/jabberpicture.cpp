/***************************************************************************
                          jabberpicture.cpp  -  description
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

#include "simapi.h"

#include <QTabWidget>
#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QFile>

#include "misc.h"

#include "contacts/contact.h"
#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"
#include "simgui/preview.h"

#include "jabberclient.h"
#include "jabberpicture.h"

using namespace SIM;

#ifndef USE_KDE

static FilePreview *createPreview(QWidget *parent)
{
    return new PictPreview(parent);
}

#endif

JabberPicture::JabberPicture(QWidget *parent, JabberUserData *data, JabberClient *client, bool bPhoto) : QWidget(parent)
        //: JabberPictureBase(parent)
{
    setupUi(this);
    m_data   = data;
    m_client = client;
    m_bPhoto = bPhoto;
    tabPict->setTabText(tabPict->indexOf(tab), m_bPhoto ? i18n("&Photo") : i18n("&Logo"));
    if (m_data){
        edtPict->hide();
        btnClear->hide();
    }else{
        QString format = "*.bmp *.gif *.jpg *.jpeg";
#ifdef USE_KDE
        edtPict->setFilter(i18n("%1|Graphics") .arg(format));
#else
edtPict->setFilter(i18n("Graphics(%1)") .arg(format));
        edtPict->setFilePreview(createPreview);
#endif
        edtPict->setReadOnly(true);
        connect(btnClear, SIGNAL(clicked()), this, SLOT(clearPicture()));
        connect(edtPict, SIGNAL(textChanged(const QString&)), this, SLOT(pictSelected(const QString&)));
        QString pict = m_bPhoto ? client->getPhoto() : client->getLogo();
        edtPict->setText(pict);
        pictSelected(pict);
    }
    fill();
}

void JabberPicture::apply()
{
}

void JabberPicture::apply(Client *client, void*)
{
    if (client != m_client)
        return;
    QString pict = edtPict->text();
    if (lblPict->pixmap() == NULL)
        pict = QString::null;
    if (m_bPhoto){
        m_client->setPhoto(pict);
    }else{
        m_client->setLogo(pict);
    }
}

bool JabberPicture::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->have(m_data))
            fill();
    }
    return false;
}

void JabberPicture::fill()
{
    if (m_data == NULL)
        return;
    if (m_bPhoto){
        if (m_data->getPhotoHeight() && m_data->getPhotoWidth()){
            QImage img(m_client->photoFile(m_data));
            setPict(img);
            return;
        }
    }else{
        if (m_data->getLogoHeight() && m_data->getLogoWidth()){
            QImage img(m_client->logoFile(m_data));
            setPict(img);
            return;
        }
    }
    QImage img;
    setPict(img);
}

void JabberPicture::clearPicture()
{
    edtPict->setText(QString::null);
}

void JabberPicture::pictSelected(const QString &file)
{
    if (file.isEmpty()){
        QImage img;
        setPict(img);
        return;
    }
    QFile f(file);
    QImage img(file);
    setPict(img);
}

void JabberPicture::setPict(QImage &img)
{
    if (img.isNull()){
        lblPict->setText(i18n("Picture is not available"));
        return;
    }
    int w = img.width();
    int h = img.height();
    if (h > w){
        if (h > 300){
            w = w * 300 / h;
            h = 300;
        }
    }else{
        if (w > 300){
            h = h * 300 / w;
            w = 300;
        }
    }
    if ((w != img.width()) || (h != img.height()))
      img = img.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPixmap pict;
    pict = QPixmap::fromImage(img);
    lblPict->setPixmap(pict);
    lblPict->setMinimumSize(pict.size());
}

/*
#ifndef NO_MOC_INCLUDES
#include "jabberpicture.moc"
#endif
*/


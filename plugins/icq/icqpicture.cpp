/***************************************************************************
                          icqpicture.cpp  -  description
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

#include "icqpicture.h"
#include "icqclient.h"

#include "log.h"

#include "contacts/contact.h"
#include "simgui/editfile.h"
#include "simgui/ballonmsg.h"
#include "simgui/preview.h"

#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QImageReader>

#include <time.h>

using namespace SIM;

#ifndef USE_KDE

static FilePreview *createPreview(QWidget *parent)
{
    return new PictPreview(parent);
}

#endif

ICQPicture::ICQPicture(QWidget *parent, ICQUserData *data, ICQClient *client)
    : QWidget(parent)
{
    setupUi(this);
    m_data   = data;
    m_client = client;
    if (m_data)
    {
        edtPict->hide();
        btnClear->hide();
    } else {
        QString format = QString("*.jpg");
        QList<QByteArray> formats = QImageReader::supportedImageFormats();
        QByteArray f;
        foreach( f, formats )
        {
            f.toLower();
            format += " *." + f;
        }
#ifdef USE_KDE
        edtPict->setFilter(i18n("%1|Graphics").arg(format));
#else
        edtPict->setFilter(i18n("Graphics(%1)").arg(format));
        edtPict->setFilePreview(createPreview);
#endif
        edtPict->setReadOnly(true);
        connect(btnClear, SIGNAL(clicked()), this, SLOT(clearPicture()));
        connect(edtPict, SIGNAL(textChanged(const QString&)), this, SLOT(pictSelected(const QString&)));
        edtPict->setText(client->getPicture());
        pictSelected(client->getPicture());
    }
    fill();
}

void ICQPicture::apply()
{
}

void ICQPicture::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    QString pict = edtPict->text();
    log(L_DEBUG, "Pict: %s", qPrintable(pict));
    m_client->setPicture(pict);
    m_client->data.owner.Picture.setStr(pict);
    ICQUserData *data = m_client->toICQUserData((SIM::clientData*)_data);  // FIXME unsafe type conversion
    if (lblPict->pixmap() == NULL)
        pict.clear();
    if(pict != m_client->getPicture())
    {
        data->PluginInfoTime.asULong() = time(NULL);
    }
}

bool ICQPicture::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->clientData.have(m_data))
            fill();
    }
    return false;
}

void ICQPicture::fill()
{
    setPict(m_client->userPicture(m_data));
}

void ICQPicture::clearPicture()
{
    edtPict->setText(QString::null);
}

const unsigned short MAX_PICTURE_SIZE      = 7168;

void ICQPicture::pictSelected(const QString &file)
{
    if (file.isEmpty()){
        setPict(QImage());
    } else {
        QFile f(file);
        if (f.size() > MAX_PICTURE_SIZE){
            setPict(QImage());
            BalloonMsg::message(i18n("Picture can not be more than 7 kbytes"), edtPict);
        }
        setPict(QImage(file));
    }
}

void ICQPicture::setPict(const QImage &img)
{
    if (img.isNull()){
        lblPict->setText(i18n("Picture is not available"));
        return;
    }
    QPixmap pict = QPixmap::fromImage(img);
    lblPict->setPixmap(pict);
    lblPict->setMinimumSize(pict.size());
}


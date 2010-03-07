/***************************************************************************
 *                         preview.cpp  -  description
 *                         -------------------
 *                         begin                : Sun Mar 24 2002
 *                         copyright            : (C) 2002 by Vladimir Shutoff
 *                         email                : vovan@shutoff.ru
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "preview.h"

#include <QLabel>
#include <QLayout>
#include <QImage>
#include <QFile>
#include <QVBoxLayout>
#include <QPixmap>
#include <QUrl>

FilePreview::FilePreview(QWidget *parent)
#ifdef USE_KDE
        : KPreviewWidgetBase(parent)
#else
        : QFileDialog(parent)
#endif
{
}

FilePreview::~FilePreview()
{
}

#ifdef USE_KDE

void FilePreview::showPreview(const KURL &url)
{
    if (url.protocol()!=QString("file")){
        showPreview(NULL);
        return;
    }
    QString fileName = url.directory();
    if (!fileName.isEmpty() && (fileName[fileName.length() - 1] != '/'))
        fileName += '/';
    fileName += url.fileName(false);
    showPreview(fileName);
}

void FilePreview::clearPreview()
{
    showPreview(NULL);
}

#else

void FilePreview::previewUrl(const QUrl &url)
{
    if (url.scheme()!=QString("file")){
        showPreview(NULL);
        return;
    }
    QString fileName = url.toString();
    showPreview(fileName);
}

#endif

#ifndef USE_KDE

PictPreview::PictPreview(QWidget *parent)
        : FilePreview(parent)
{
    label = new QLabel(this);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->setMinimumSize(QSize(70, 70));
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(label);
}

void PictPreview::showPreview(const QString &file)
{
    if(file.isEmpty())
	{
        label->setPixmap(QPixmap());
        return;
    }
    QImage img(file);
    if ((img.width() > label->width()) || (img.height() > label->height())){
        bool bOk = false;
        if (img.width() > label->width()){
            int h = img.height() * label->width() / img.width();
            if (h <= label->height()){
                img = img.scaled( label->width(), h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
                bOk = true;
            }
        }
        if (!bOk){
            int w = img.width() * label->height() / img.height();
            img = img.scaled( w, label->height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
        }
    }
    QPixmap pict = QPixmap::fromImage( img );
    label->setPixmap(pict);
}

#endif


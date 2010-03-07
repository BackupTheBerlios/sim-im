/***************************************************************************
 *                         preview.h  -  description
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
#ifndef PREVIEW_H
#define PREVIEW_H

#include "simapi.h"
#include <QLabel>
#ifdef USE_KDE
    #include <kpreviewwidgetbase.h>
    #include <kurl.h>
#else
    #include <QFileDialog>
#endif

#ifdef USE_KDE
class EXPORT FilePreview : public KPreviewWidgetBase
#else
class EXPORT FilePreview : public QFileDialog
#endif
{
public:
    FilePreview(QWidget *parent);
    ~FilePreview();
#ifdef USE_KDE
    virtual void showPreview(const KURL &url);
    virtual void clearPreview();
#else
    virtual void showPreview(const QString &file) = 0;
    virtual void previewUrl(const QUrl&);
#endif
};

#ifndef USE_KDE

class EXPORT PictPreview : public FilePreview
{
public:
    PictPreview(QWidget *parent);
protected:
    QLabel  *label;
    void showPreview(const QString &);
};

#endif

#endif // PREVIEW_H

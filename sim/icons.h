/***************************************************************************
                          icons.h  -  description
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

#ifndef _ICONS_H
#define _ICONS_H

#include <QIcon>
#include <QImage>
#include <QString>
#include <QStringList>
#include <QPixmap>

#include "simapi.h"
#include "event.h"

namespace SIM
{

class IconSet;

struct PictDef
{
    QImage      image;
    QString     file;
#ifdef USE_KDE
    QString     system;
#endif
    unsigned    flags;
};

class EXPORT Icons : public QObject, public EventReceiver
{
    Q_OBJECT
    Icons();
public:
    static Icons *instance();
    virtual ~Icons();
    PictDef *getPict(const QString &name);
    QString parseSmiles(const QString&);
    QStringList getSmile(const QString &ame);
    void getSmiles(QStringList &smiles);
    QString getSmileName(const QString &name);
    static unsigned nSmile;
    IconSet *addIconSet(const QString &name, bool bDefault);
    void removeIconSet(IconSet*);
	void setPixmap(const QString& name, const QPixmap& pict);
protected slots:
    void iconChanged(int);
protected:
    virtual bool processEvent(Event *e);
    class IconsPrivate * const d;
    COPY_RESTRICTED(Icons);
};
inline Icons *getIcons() { return Icons::instance(); }

EXPORT QIcon   Icon(const QString &name);
EXPORT QPixmap Pict(const QString &name);
EXPORT QImage  Image(const QString &name);

};

#endif


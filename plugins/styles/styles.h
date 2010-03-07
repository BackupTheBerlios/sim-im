/***************************************************************************
                          styles.h  -  description
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

#ifndef _STYLES_H
#define _STYLES_H

#include "cfg.h"
#include "plugins.h"
#include "propertyhub.h"
#include "event.h"

class QFont;
class QStyle;
class QPalette;

class StylesPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
public:
    StylesPlugin(unsigned, Buffer*);
    ~StylesPlugin();
    virtual QByteArray getConfig();
    QWidget *createConfigWindow(QWidget *parent);
    void setFonts();
    void setupDefaultFonts();
    void setColors();
    void setStyles();
    void setPropertyHub(SIM::PropertyHubPtr hub);
    SIM::PropertyHubPtr propertyHub();
    QVariant value(const QString& key);
    void setValue(const QString& key, const QVariant& v);
protected:
    virtual bool processEvent(SIM::Event *e);

    QFont       *m_saveBaseFont;
    QFont       *m_saveMenuFont;
    QPalette	*m_savePalette;
    QStyle      *m_saveStyle;
    friend class FontConfig;
private:
    SIM::PropertyHubPtr m_propertyHub;
};

#endif


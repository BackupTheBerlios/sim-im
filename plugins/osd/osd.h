/***************************************************************************
                          osd.h  -  description
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

#ifndef _OSD_H
#define _OSD_H

#include <QFont>
#include <QLabel>
#include <QList>
#include <QThread>
#include <QTimer>
#include <QPixmap>
#include <QSet>

#include <simapi.h>
#include <cfg.h>
#include <event.h>
#include <plugins.h>
#include <propertyhub.h>

enum OSDType
{
    OSD_NONE,
    OSD_ALERTONLINE,
    OSD_ALERTAWAY,
    OSD_ALERTNA,
    OSD_ALERTDND,
    OSD_ALERTOCCUPIED,
    OSD_ALERTFFC,
    OSD_ALERTOFFLINE,
    OSD_TYPING,
    OSD_MESSAGE
};

struct OSDRequest
{
    unsigned long	contact;
    OSDType		type;
};

class OSDPlugin;

class OSDWidget : public QLabel
{
    Q_OBJECT
public:
    OSDWidget(OSDPlugin* plugin);
    void showOSD(const QString &text, SIM::PropertyHubPtr data);

    virtual QSize sizeHint () const;

signals:
    void dblClick();
    void closeClick();
protected slots:
    void slotCloseClick();
    void slotTimerFadeInTimeout();
public slots:
    void hide();

private:
	SIM::PropertyHubPtr currentData;
    bool isScreenSaverActive();
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *event);
    QRect recalcGeometry();
    void draw(QPainter &p);

    QFont	baseFont;
    QPixmap bgPict;
    OSDPlugin	*m_plugin;
    QTimer      m_transTimer;
    int transCounter;
    int transCounterDelta;
    bool m_bBackground;
    bool m_bShadow;
    int m_text_y;
    QString m_sText;
    bool m_bFading;
    QRect m_Rect;
    QImage m_imageButton;
    QRect m_rectButton;
};

class OSDPlugin : public QThread, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    OSDPlugin(unsigned);
    QFont getBaseFont(QFont font);
    virtual ~OSDPlugin();
    unsigned long user_data_id;
protected slots:
    void timeout();
    void dblClick();
    void closeClick();
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual bool processEvent(SIM::Event *e);
    virtual void run();
    void processQueue();
    void flashCapsLockLED(bool);
    OSDRequest          m_request;
    QList<OSDRequest>   m_queue;
    QSet<unsigned>      m_typing;
    OSDWidget           *m_osd;
    QTimer              *m_timer;
    bool bCapsState;
    bool bHaveUnreadMessages;   // Should use this flag in OSDPlugin::run instead of core->unread.size()
                                // see patch #2304 for more info.
    bool bTimerActive;
};

#endif


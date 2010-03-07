/***************************************************************************
                          floatywnd.h  -  description
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

#ifndef _FLOATYWND_H
#define _FLOATYWND_H


#include <QSet>
#include <QString>
#include <QWidget>

class FloatyPlugin;
class QTimer;
class QPainter;

#ifdef MAKE_FLOATY_LIB
# define FLOATY_EXPORTS Q_DECL_EXPORT
#else
# define FLOATY_EXPORTS Q_DECL_IMPORT
#endif

class FLOATY_EXPORTS FloatyWnd : public QWidget
{
    Q_OBJECT
public:
    FloatyWnd(FloatyPlugin*, unsigned long id);
    ~FloatyWnd();
    unsigned long id() { return m_id; }
    void init();
    void startBlink();
private slots:
    void startMove();
    void blink();
private:
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseDoubleClickEvent(QMouseEvent*);
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
    void dragEvent(QDropEvent *e, bool isDrop);
    void setFont(QPainter *p);
    void move(QPoint point);
    virtual bool event( QEvent *event );
    QPoint	mousePos;
    QPoint	initMousePos;
    QString	m_text;
    QSet<QString> m_icons;
    QString m_statusIcon;
    unsigned long m_id;
    unsigned m_style;
    unsigned m_unread;
    unsigned m_blink;
    unsigned long m_status;
    bool b_ignoreMouseClickRelease;
    QTimer	 *blinkTimer;
    QTimer	 *moveTimer;
    FloatyPlugin *m_plugin;
};

#endif


/***************************************************************************
                          ballonmsg.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#ifndef _BALLONMSG_H
#define _BALLONMSG_H

#include <QDialog>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include "simapi.h"

class QString;
class QStringList;
class QCheckBox;

class EXPORT BalloonMsg : public QDialog
{
    Q_OBJECT
public:
    BalloonMsg(void *param, const QString &_text, QStringList&, QWidget *parent,
               const QRect *rcParent = NULL, bool bModal=false, bool bAutoHide=true,
               unsigned bwidth=150, const QString &box_msg = QString::null, bool *bChecked=NULL);
    ~BalloonMsg();
    static void message(const QString &text, QWidget *parent, bool bModal=false, unsigned width=150, const QRect *rc=NULL);
    static void ask(void *param, const QString &text, QWidget *parent, const char *slotYes, const char *slotNo, const QRect *rc=NULL, QObject *receiver=NULL, const QString &boxText = QString::null, bool *bChecked=NULL);
    bool isChecked();
signals:
    void action(int, void*);
    void yes_action(void*);
    void no_action(void*);
    void finished();
protected slots:
    void action(int);
protected:
    bool eventFilter(QObject*, QEvent*);
    void mousePressEvent(QMouseEvent*);
    void paintEvent(QPaintEvent*);
    QPixmap m_backgroundPixmap;

    QCheckBox *m_check;
    bool m_bAutoHide;
    bool m_bYes;
    bool *m_bChecked;
    void *m_param;
	QFrame *frm;
	QVBoxLayout *vlay;
	QHBoxLayout *lay;
};

class BalloonButton : public QPushButton
{
    Q_OBJECT
public:
    BalloonButton(const QString &text, QWidget *parent, int id);
signals:
    void action(int);
protected slots:
    void click();
protected:
    int id;
};


#endif


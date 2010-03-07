/***************************************************************************
                          qchildwidget.cpp  -  description
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

#include "qchildwidget.h"

#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>
#include <QChildEvent>
#include <QEvent>

QChildWidget::QChildWidget(QWidget *parent, const char *name)
        : QWidget(parent)
{
    setObjectName(name);
    m_bInit = false;
}

void QChildWidget::childEvent(QChildEvent *e)
{
    if (!m_bInit){
        m_bInit = true;
        if (e->child()->inherits("QWidget")){
			// WHAT IS THIS????
            if ((static_cast<QWidget*>(e->child())->windowFlags() & Qt::Popup) == 0)
                e->child()->installEventFilter(this);
        }
    }
    QWidget::childEvent(e);
}

void QChildWidget::paintEvent(QPaintEvent*)
{
    for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
        const QPixmap bg = p->palette().brush(p->backgroundRole()).texture();
        if (!bg.isNull()){
            QPoint pos = mapToGlobal(QPoint(0, 0));
            pos = p->mapFromGlobal(pos);
            QPainter pp(this);
            pp.drawTiledPixmap(0, 0, width(), height(), bg, pos.x(), pos.y());
            return;
        }
        if (p == topLevelWidget())
            break;
    }
    QPainter pp(this);
    pp.eraseRect(0, 0, width(), height());
}

bool QChildWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Paint){
        QWidget *w = static_cast<QWidget*>(o);
        for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
            const QPixmap bg = p->palette().brush(p->backgroundRole()).texture();
            if (!bg.isNull()){
                QPoint pos = w->mapToGlobal(QPoint(0, 0));
                pos = p->mapFromGlobal(pos);
                QRect rc(pos.x(), pos.y(), w->width(), w->height());
                if (rc != rcChild){
                    rcChild = rc;
                    QPixmap new_bg(w->width(), w->height());
                    QPainter pp(&new_bg);
                    pp.drawTiledPixmap(0, 0, w->width(), w->height(), bg, pos.x(), pos.y());
                    pp.end();
                    QPalette palette = w->palette();
                    palette.setBrush(w->backgroundRole(), QBrush(new_bg));
                    w->setPalette(palette);
                }
                if (!w->palette().brush(w->backgroundRole()).texture().isNull()){
                    QPainter pp(w);
                    pp.drawPixmap(0, 0, w->palette().brush(w->backgroundRole()).texture());
                }
                return true;
            }
        }
    }
    return false;
}

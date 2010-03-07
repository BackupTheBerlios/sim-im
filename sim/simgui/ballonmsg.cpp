/***************************************************************************
                          ballonmsg.cpp  -  description
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

#include "simgui/ballonmsg.h"

#include <QApplication>
#include <QBitmap>

#include <QImage>
#include <QCheckBox>
#include <QLayout>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QToolTip>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QEvent>

#include <QTextDocument>

#include "misc.h"
#include "unquot.h"

#define BALLOON_R               10
#define BALLOON_TAIL            20
#define BALLOON_TAIL_WIDTH      12
#define BALLOON_MARGIN          8
#define BALLOON_SHADOW_DEF      2

using namespace SIM;

BalloonMsg::BalloonMsg(void *param, const QString &text, QStringList &btn, QWidget *parent, const QRect *rcParent,
                       bool bModal, bool bAutoHide, unsigned bwidth, const QString &box_msg, bool *bChecked)
        : QDialog(parent,
                  (bAutoHide ? Qt::Popup : Qt::Window | Qt::WindowStaysOnTopHint)
                  | Qt::FramelessWindowHint | Qt::Tool)
        , m_check(NULL)
		, m_bAutoHide(bAutoHide)
        , m_bYes(false)
        , m_bChecked(bChecked)
        , m_param(param)
		, frm(new QFrame(this))
		, vlay(new QVBoxLayout(frm))
		, lay(new QHBoxLayout())
{
    setModal(bModal);
    setAttribute(Qt::WA_DeleteOnClose);
    setPalette(QToolTip::palette());

    bool bTailDown = true;
    //QFrame *frm = new QFrame(this);
    //QVBoxLayout *vlay = new QVBoxLayout(frm);
    vlay->setMargin(0);
    
    if (!box_msg.isEmpty()){
        m_check = new QCheckBox(box_msg, frm);
        vlay->addWidget(m_check);
        if (m_bChecked)
            m_check->setChecked(*m_bChecked);
    }
    //QHBoxLayout *lay = new QHBoxLayout();
    vlay->addLayout(lay);
    lay->setSpacing(5);
    lay->addStretch();
    unsigned id = 0;
    bool bFirst = true;
    for (QStringList::Iterator it = btn.begin(); it != btn.end(); ++it, id++){
        BalloonButton *b = new BalloonButton(*it, frm, id);
        connect(b, SIGNAL(action(int)), this, SLOT(action(int)));
        lay->addWidget(b);
        if (bFirst){
            b->setDefault(true);
            bFirst = false;
        }
    }
    setButtonsPict(this);
    lay->addStretch();
    int hButton  = frm->minimumSizeHint().height();

    int txtWidth = bwidth;
    QRect rc;
    if (rcParent){
        rc = *rcParent;
    }else{
        QPoint p = parent->mapToGlobal(parent->rect().topLeft());
        rc = QRect(p.x(), p.y(), parent->width(), parent->height());
    }
    if (rc.width() > txtWidth)
        txtWidth = rc.width();

    QTextDocument doc;
    doc.setHtml(text);
    doc.adjustSize();
    doc.setDefaultFont(font());

    QSizeF s = doc.size();
    QSize sMin = frm->minimumSizeHint();
    if (s.width() < sMin.width())
        s.setWidth(sMin.width());
    int BALLOON_SHADOW = BALLOON_SHADOW_DEF;
    resize((int)s.width() + BALLOON_R * 2 + BALLOON_SHADOW,
           (int)s.height() + BALLOON_R * 2 + BALLOON_TAIL + BALLOON_SHADOW + hButton + BALLOON_MARGIN);
    int w = width() - BALLOON_SHADOW;
    int tailX = w / 2;
    int posX = rc.left() + rc.width() / 2 + BALLOON_TAIL_WIDTH - tailX;
    if (posX <= screenGeometry().left())
        posX = 1;
    QRect rcScreen = screenGeometry();
    if (posX + width() >= rcScreen.width())
        posX = rcScreen.width() - 1 - width();
    int tx = posX + tailX - BALLOON_TAIL_WIDTH;
    if (tx < rc.left())
        tx = rc.left();
    if (tx > rc.left() + rc.width())
        tx = rc.left() + rc.width();
    tailX = tx + BALLOON_TAIL_WIDTH - posX;
    if (tailX < BALLOON_R) tailX = BALLOON_R;
    if (tailX > width() - BALLOON_R - BALLOON_TAIL_WIDTH)
        tailX = width() - BALLOON_R - BALLOON_TAIL_WIDTH;
    if (rc.top() <= height() + 2){
        bTailDown = false;
        move(posX, rc.top() + rc.height() + 1);
    }else{
        move(posX, rc.top() - height() - 1);
    }
    int pos = 0;
    int h = height() - BALLOON_SHADOW - BALLOON_TAIL;
    if (!bTailDown)
        pos += BALLOON_TAIL;
    frm->resize((int)s.width(), hButton);
    frm->move(BALLOON_R, pos + h - BALLOON_R - hButton);

    QPixmap pm(width(), height());
    // fill with transparent color
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHints(QPainter::TextAntialiasing);
    // we need it more than one time
    const QBrush backBrush = palette().brush(QPalette::ToolTipBase);
    p.setPen(palette().color(QPalette::ToolTipText));
    p.setBrush(backBrush);

    // the four corners
    p.drawEllipse(0, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(w - BALLOON_R * 2, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);
    p.drawEllipse(0, pos + h - BALLOON_R * 2, BALLOON_R * 2, BALLOON_R * 2);

    // tail
    QPolygon arr(3);
    arr.setPoint(0, tailX, bTailDown ? h - 1 : pos + 1);
    arr.setPoint(1, tailX + BALLOON_TAIL_WIDTH, bTailDown ? h - 1 : pos + 1);
    arr.setPoint(2, tailX - BALLOON_TAIL_WIDTH, bTailDown ? height() - BALLOON_SHADOW : 0);
    p.drawPolygon(arr);

    // the inner rest (after tail!)
    p.fillRect(0, pos + BALLOON_R, w, h - BALLOON_R * 2, backBrush);
    p.fillRect(BALLOON_R, pos, w - BALLOON_R * 2, h, backBrush);

    p.setPen(palette().color(QPalette::ToolTipText));
    // black lines left and right
    p.drawLine(0, pos + BALLOON_R, 0, pos + h - BALLOON_R);
    p.drawLine(w, pos + BALLOON_R, w, pos + h - BALLOON_R);

    // black lines left and right
    if (bTailDown){
        // upper line
        p.drawLine(BALLOON_R, 0, w - BALLOON_R, 0);
        // lower left
        p.drawLine(BALLOON_R, h, tailX, h);
        // lower right
        p.drawLine(tailX + BALLOON_TAIL_WIDTH, h, w - BALLOON_R, h);
    }else{
        // lower line
        p.drawLine(BALLOON_R, pos + h, w - BALLOON_R, pos + h);
        // upper left
        p.drawLine(BALLOON_R, pos, tailX, pos);
        // upper right
        p.drawLine(tailX + BALLOON_TAIL_WIDTH, pos, w - BALLOON_R, pos);
    }

    // center text
    p.translate((pm.width() - s.width()) / 2, pos + BALLOON_R);
    doc.drawContents(&p);
    p.end();

    m_backgroundPixmap = pm;
    setMask(pm.mask());

    if (!bAutoHide)
        setFocusPolicy(Qt::NoFocus);

    QWidget *top = NULL;
    if (parent)
        top = parent->topLevelWidget();
    if (top){
        raiseWindow(top);
        top->installEventFilter(this);
    }
}

BalloonMsg::~BalloonMsg()
{
    if (!m_bYes)
        emit no_action(m_param);
    emit finished();
}

bool BalloonMsg::isChecked()
{
    if (m_check)
        return m_check->isChecked();
    return false;
}

bool BalloonMsg::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Hide) && (o == static_cast<QWidget*>(parent())->topLevelWidget()))
        return true;
    return QDialog::eventFilter(o, e);
}

void BalloonMsg::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setBackgroundMode(Qt::OpaqueMode);
    p.fillRect(rect(), QColor(Qt::transparent));
    p.drawPixmap(0, 0, m_backgroundPixmap);
    p.end();
}

void BalloonMsg::mousePressEvent(QMouseEvent *e)
{
    if (m_bAutoHide && rect().contains(e->pos())){
        const QRgb rgb = m_backgroundPixmap.toImage().pixel(e->pos());
        if (rgb & 0xFFFFFF)
            QTimer::singleShot(10, this, SLOT(close()));
    }
    QWidget::mousePressEvent(e);
}

void BalloonMsg::action(int id)
{
    if (m_bChecked && m_check)
        *m_bChecked = m_check->isChecked();
    emit action(id, m_param);
    if (id == 0){
        emit yes_action(m_param);
        m_bYes = true;
    }
}

void BalloonMsg::message(const QString &text, QWidget *parent, bool bModal, unsigned width, const QRect *rc)
{
    QStringList btns;
    btns.append(i18n("&Ok"));
    BalloonMsg *msg = new BalloonMsg(NULL, QString("<center>") + quoteString(text) + "</center>", btns, parent, rc, bModal, true, width);
    if (bModal){
        msg->exec();
    }else{
        msg->show();
    }
}

void BalloonMsg::ask(void *param, const QString &text, QWidget *parent,
                     const char *slotYes, const char *slotNo,
                     const QRect *rc, QObject *receiver,
                     const QString &checkText, bool *bCheck)
{
    QStringList btns;
    btns.append(i18n("&Yes"));
    btns.append(i18n("&No"));
    BalloonMsg *msg = new BalloonMsg(param, QString("<center>") + quoteString(text) + "</center>", btns, parent, rc, false, true, 300, checkText, bCheck);
    if (receiver == NULL)
        receiver = parent;
    if (slotYes)
        connect(msg, SIGNAL(yes_action(void*)), receiver, slotYes);
    if (slotNo)
        connect(msg, SIGNAL(no_action(void*)), receiver, slotNo);
    msg->show();
}

BalloonButton::BalloonButton(const QString &string, QWidget *parent, int _id)
        : QPushButton(string, parent)
{
    id = _id;
    setPalette(parent->palette());
    resize(sizeHint());
    connect(this, SIGNAL(clicked()), this, SLOT(click()));
}

void BalloonButton::click()
{
    topLevelWidget()->hide();
    emit action(id);
    topLevelWidget()->close();
}


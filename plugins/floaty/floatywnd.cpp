/***************************************************************************
                          floatywnd.cpp  -  description
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

#include "simapi.h"
#include <iostream>

#include "floatywnd.h"
#include "floaty.h"
#include "contacts/contact.h"

#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QApplication>
#include <QWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QDesktopWidget>
#include <QToolTip>

#ifdef USE_KDE
#include <kwin.h>
#endif

#include "icons.h"
#include "simgui/linklabel.h"
#include "userview.h"
#include "core.h"

using namespace std;
using namespace SIM;

namespace { namespace aux {

static QString
compose_floaty_name( unsigned long id )
{
    return QString( "floaty-%1" ).arg( id );
}

}}

FloatyWnd::FloatyWnd(FloatyPlugin *plugin, unsigned long id)
        : QWidget(NULL,
                  Qt::Tool
                  | Qt::WindowStaysOnTopHint
                  | Qt::CustomizeWindowHint
                  | Qt::FramelessWindowHint
                  | Qt::Tool
                  | Qt::X11BypassWindowManagerHint
                )
{
    setAttribute(Qt::WA_MacAlwaysShowToolWindow);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setObjectName(aux::compose_floaty_name( id ) );
    m_plugin = plugin;
    m_id = id;
    m_blink = 0;
    b_ignoreMouseClickRelease=false;
    init();
    setAcceptDrops(true);
#ifdef USE_KDE
    KWin::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    KWin::setOnAllDesktops(winId(), true);
#endif
    moveTimer = new QTimer(this);
    connect(moveTimer, SIGNAL(timeout()), this, SLOT(startMove()));
    blinkTimer = new QTimer(this);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
    setMouseTracking(true);
}

FloatyWnd::~FloatyWnd()
{
    delete moveTimer;
    delete blinkTimer;
}

void FloatyWnd::init()
{
    m_style = 0;
    m_icons.clear();
    m_unread = 0;
    Contact *contact = getContacts()->contact(m_id);
    if (contact == NULL)
        return;
    m_text = contact->getName();
    m_status = contact->contactInfo(m_style, m_statusIcon, &m_icons);
    unsigned blink = m_blink;
    m_blink = 1;
    m_blink = blink;
    QFontMetrics metr(font());
    QRect br = metr.boundingRect(m_text);
    int h = br.height();
    int w = br.width() + 5;
    const QPixmap &pict = Pict(m_statusIcon);
    w += pict.width() + 2;
    if (pict.height() > h)
        h = pict.height();
    Q_FOREACH(const QString &icon, m_icons) {
        const QPixmap &pict = Pict(icon);
        w += pict.width() + 2;
        if (pict.height() > h)
            h = pict.height();
    }
    w += 15;
    h += 6;
    resize(w, h);
    CorePlugin *core = GET_CorePlugin();
    for (list<msg_id>::iterator it = core->unread.begin(); it != core->unread.end(); ++it){
        if (it->contact != m_id)
            continue;
        m_unread = it->type;
        m_plugin->startBlink();
        break;
    }
}

void FloatyWnd::paintEvent(QPaintEvent*)
{
    int w = width()  - 4;
    int h = height() - 4;

    QPixmap pict(w, h);
    QPainter p(&pict);
    p.fillRect(QRect(0, 0, width(), height()), palette().brush(QPalette::Base));
    EventPaintView::PaintView pv;
    pv.p        = &p;
    pv.pos      = QPoint(2, 2);
    pv.size     = QSize(w, h);
    pv.win      = this;
    pv.isStatic = false;
    pv.height   = h;
    CorePlugin *core = GET_CorePlugin();
    if (core->value("UseSysColors").toBool()){
        p.setPen(palette().color(QPalette::Text));
    }else{
        p.setPen(QColor(core->value("ColorOnline").toUInt()));
    }
    EventPaintView e(&pv);
    e.process();

    if (core->value("UseSysColors").toBool()){
        if (m_status != STATUS_ONLINE)
            p.setPen(palette().color(QPalette::Disabled, QPalette::Text));
    }else{
        switch (m_status){
        case STATUS_ONLINE:
            p.setPen(core->value("ColorOnline").toUInt());
            break;
        case STATUS_AWAY:
            p.setPen(core->value("ColorAway").toUInt());
            break;
        case STATUS_NA:
            p.setPen(core->value("ColorNA").toUInt());
            break;
        case STATUS_DND:
            p.setPen(core->value("ColorDND").toUInt());
            break;
        default:
            p.setPen(core->value("ColorOffline").toUInt());
            break;
        }
    }

    int x = 0;
    QString statusIcon = m_statusIcon;
    if (m_unread && m_plugin->m_bBlink){
        CommandDef *def = core->messageTypes.find(m_unread);
        if (def)
            statusIcon = def->icon;
    }

    if (!statusIcon.isEmpty()){
        const QPixmap &pict = Pict(statusIcon);
        x += 2;
        p.drawPixmap(x, (h - pict.height()) / 2, pict);
        x += pict.width() + 2;
    }
    QRect br;
    setFont(&p);
    p.drawText(x, 0, w, h, Qt::AlignLeft | Qt::AlignVCenter, m_text, &br);
    x = br.right() + 5;
    Q_FOREACH(const QString &icon, m_icons) {
        const QPixmap &pict = Pict(icon);
        x += 2;
        p.drawPixmap(x, (h - pict.height()) / 2, pict);
        x += pict.width();
    }
    p.end();

    p.begin(this);
    p.drawPixmap(QPoint(2, 2), pict);

    p.setPen(palette().color(QPalette::Dark));
	p.drawLine(1, 1, width() - 2, 1);
	p.drawLine(width() - 2, 1, width() - 2, height() - 2);
	p.drawLine(width() - 2, height() - 2, 1, height() - 2);
	p.drawLine(1, height() - 2, 1, 1);

    p.setPen(palette().color(QPalette::Shadow));
	p.drawLine(0, height() - 1, width() - 1, height() - 1);
	p.drawLine(width() - 1, height() - 1, width() - 1, 1);
	p.drawLine(width() - 3, 2, 2, 2);
	p.drawLine(2, 2, 2, height() - 3);

    p.setPen(palette().color(QPalette::Light));
	p.drawLine(2, height() - 3, width() - 3, height() - 3);
	p.drawLine(width() - 3, height() - 3, width() - 3, 2);
	p.drawLine(width() - 1, 0, 0, 0);
	p.drawLine(0, 0, 0, height() - 1);
}

void FloatyWnd::setFont(QPainter *p)
{
    QFont f(font());
    CorePlugin *core = GET_CorePlugin();
    if (m_style & CONTACT_ITALIC){
        if (core->value("VisibleStyle").toUInt()  & STYLE_ITALIC)
            f.setItalic(true);
        if (core->value("VisibleStyle").toUInt()  & STYLE_UNDER)
            f.setUnderline(true);
        if (core->value("VisibleStyle").toUInt()  & STYLE_STRIKE)
            f.setStrikeOut(true);
    }
    if (m_style & CONTACT_UNDERLINE){
        if (core->value("AuthStyle").toUInt()  & STYLE_ITALIC)
            f.setItalic(true);
        if (core->value("AuthStyle").toUInt()  & STYLE_UNDER)
            f.setUnderline(true);
        if (core->value("AuthStyle").toUInt()  & STYLE_STRIKE)
            f.setStrikeOut(true);
    }
    if (m_style & CONTACT_STRIKEOUT){
        if (core->value("InvisibleStyle").toUInt()  & STYLE_ITALIC)
            f.setItalic(true);
        if (core->value("InvisibleStyle").toUInt()  & STYLE_UNDER)
            f.setUnderline(true);
        if (core->value("InvisibleStyle").toUInt()  & STYLE_STRIKE)
            f.setStrikeOut(true);
    }
    if (m_blink & 1){
        f.setBold(true);
    }else{
        f.setBold(false);
    }
    p->setFont(f);
}

void FloatyWnd::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton){
        initMousePos = e->pos();
        moveTimer->start(QApplication::startDragTime());
    }
    if (e->button() == Qt::RightButton){
        m_plugin->popupPos = e->globalPos();
        m_plugin->popupId  = m_id;
        QTimer::singleShot(0, m_plugin, SLOT(showPopup()));
    }
}

void FloatyWnd::mouseReleaseEvent(QMouseEvent *e)
{
    moveTimer->stop();
	
    if (!mousePos.isNull()){
		if (!b_ignoreMouseClickRelease) // we reached fetch positich
			move(e->globalPos() - mousePos);
        releaseMouse();
        Contact *contact = getContacts()->contact(m_id);
        if (contact){
            SIM::PropertyHubPtr data = contact->getUserData("floaty", false);
            if(!data.isNull())
            {
                data->setValue("X", x());
                data->setValue("Y", y());
            }
        }
        mousePos = QPoint();
    }
    else
    {
        CorePlugin *core = GET_CorePlugin();
        if ((e->pos() == initMousePos) && !core->value("UseDblClick").toBool())
        {
            EventDefaultAction(m_id).process();
        }
    }
    initMousePos = QPoint(0, 0);
}

void FloatyWnd::mouseMoveEvent(QMouseEvent *e)
{
    if ((e->buttons() & Qt::LeftButton) && !initMousePos.isNull() &&
            (QPoint(e->pos() - initMousePos).manhattanLength() > QApplication::startDragDistance()))
        startMove();
    if (!mousePos.isNull())
    {
        QWidgetList list = QApplication::topLevelWidgets();
        QWidget * w;
        foreach(w,list)
		{
			if (w->inherits("FloatyWnd"))
			{
				FloatyWnd *refwnd = static_cast<FloatyWnd*>(w);
				int dist = 4;
                                move(e->globalPos() - mousePos);
				//Top left:
				if (this->pos().x() + this->width()  - refwnd->pos().x() <= dist &&  //== x Top left
						this->pos().x() + this->width()  - refwnd->pos().x() >= 0 &&
						this->pos().y() + this->height() - refwnd->pos().y() <= dist &&
						this->pos().y() + this->height() - refwnd->pos().y() >= 0) {
                                        QWidget::move(refwnd->pos().x()-this->width(),   //== x Top left
							refwnd->pos().y()-this->height());
					b_ignoreMouseClickRelease=true;
					cout << "TOP LEFT" << endl;
					return;
				}

				//Bottom left
				if (this->pos().x() + this->width()  - refwnd->pos().x() <= dist &&
						this->pos().x() + this->width()  - refwnd->pos().x() >=0 && //== x Top left
						this->pos().y() - refwnd->pos().y() - refwnd->height() <= dist &&
						this->pos().y() - refwnd->pos().y() - refwnd->height() >=0 ) {
                                        QWidget::move(refwnd->pos().x()-this->width(),   //== x Top left
							refwnd->pos().y()+refwnd->height());
					b_ignoreMouseClickRelease=true;
					cout << "BOTTOM LEFT" << endl;
					return;
				}

				//Top right
				if (this->pos().x() + refwnd->width() - this->pos().x() <= dist &&
						this->pos().y() + this->height() - refwnd->pos().y() <= dist ) {//== y Top left
                                        QWidget::move(refwnd->pos().x()+refwnd->width(),
							refwnd->pos().y()-this->height());  //== y Top left
					b_ignoreMouseClickRelease=true;
					cout << "TOP RIGHT" << endl;
					return;
				}

				//Bottom right
				if (this->pos().x() + refwnd->width() - this->pos().x() <= dist &&
						this->pos().x() + refwnd->width() - this->pos().x() >=0 && //== x Top right
						this->pos().y() - refwnd->pos().y() - refwnd->height() <= dist &&
						this->pos().y() - refwnd->pos().y() - refwnd->height() >=0  ) { //== y Bottom left
                                        QWidget::move(refwnd->pos().x()+refwnd->width(),	 //== x Top right
							refwnd->pos().y()-refwnd->height());  //== y Bottom left
					b_ignoreMouseClickRelease=true;
					cout << "BOTTOM LEFT" << endl;
					return;
				}
				//Top
				if (this->pos().y()+this->height()-refwnd->pos().y() <= dist ) {
					if (this->pos().x() == refwnd->pos().x()) {//add distance
                                                QWidget::move(refwnd->pos().x(),	 //== x Top right
								refwnd->pos().y()-this->height());  //== y Top left
						b_ignoreMouseClickRelease=true;
						cout << "TOP dock left" << endl;
						return;
					}

					if (this->pos().x() + this->width() == refwnd->pos().x() + refwnd->width()) {//add distance
                                                QWidget::move(refwnd->pos().x() + refwnd->width() - this->width(),	 //== x Top right
								refwnd->pos().y()-this->height());  //== y Top left
						b_ignoreMouseClickRelease=true;
						cout << "TOP dock right" << endl;
						return;
					}
				}

				//Bottom
				if (refwnd->pos().y()+refwnd->height()-this->pos().y() <= dist ) {
					if (this->pos().x() == refwnd->pos().x()) {  //add distance
                                                QWidget::move(refwnd->pos().x(),	 //== x Top right
								refwnd->pos().y()+refwnd->height());  //== y Bottem left
						b_ignoreMouseClickRelease=true;
						cout << "BOTTOM dock left" << endl;
						return;
					}					

					if (this->pos().x() + this->width() == refwnd->pos().x() + refwnd->width()) {//add distance
                                                QWidget::move(refwnd->pos().x() + refwnd->width() - this->width(),	 //== x Top right
								refwnd->pos().y()+refwnd->height());  //== y Bottem left
						b_ignoreMouseClickRelease=true;
						cout << "BOTTOM dock right" << endl;
						return;
					}
				}

				//Left
				if (this->pos().x()+this->width()-refwnd->pos().x() <= dist && 
						this->pos().x()+this->width()-refwnd->pos().x() >= 0
				   ) 
					if (this->pos().y()   - refwnd->pos().y() < dist ||
							refwnd->pos().y() - this->pos().y()   < dist ) {
                                                QWidget::move(refwnd->pos().x() - this->width(),
								refwnd->pos().y());
						b_ignoreMouseClickRelease=true;
						cout << "LEFT" << endl;
						return;
					}



				//Right
				if (refwnd->pos().x()+refwnd->width()-this->pos().x() <= dist &&
						refwnd->pos().x()+refwnd->width()-this->pos().x() >=0
				   ) 
					if (this->pos().y()   - refwnd->pos().y() < dist ||
							refwnd->pos().y() - this->pos().y()   < dist ) {
                                                QWidget::move(refwnd->pos().x() + refwnd->width(),
								refwnd->pos().y());	
						b_ignoreMouseClickRelease=true;
						cout << "RIGHT" << endl;
						return;
					}

				//this->size();
				this->repaint();
			}
		}
	}
}

void FloatyWnd::startMove()
{
    if (initMousePos.isNull())
        return;
    moveTimer->stop();
    mousePos = initMousePos;
    initMousePos = QPoint(0, 0);
    grabMouse();
}

void FloatyWnd::move(QPoint point)
{
    QRect r = QApplication::desktop()->availableGeometry();
    QRect wr = frameGeometry();
    if(point.x() < r.x())
        point.setX(r.x());
    if(point.y() < r.y())
        point.setY(r.y());
    if(point.x()+wr.width() > r.x()+r.width())
        point.setX(r.x()+r.width()-wr.width());
    if(point.y()+wr.height() > r.y()+r.height())
        point.setY(r.y()+r.height()-wr.height());
    QWidget::move(point);
}

void FloatyWnd::blink()
{
    if (m_blink){
        m_blink--;
    }else{
        blinkTimer->stop();
    }
    repaint();
}

void FloatyWnd::mouseDoubleClickEvent(QMouseEvent *)
{
    EventDefaultAction(m_id).process();
}

void FloatyWnd::dragEnterEvent(QDragEnterEvent *e)
{
    dragEvent(e, false);
}

void FloatyWnd::dropEvent(QDropEvent *e)
{
    dragEvent(e, true);
}

void FloatyWnd::dragEvent(QDropEvent *e, bool isDrop)
{
    Message *msg = NULL;
    CommandDef *cmd;
    CorePlugin *core = GET_CorePlugin();
    CommandsMapIterator it(core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if (def && def->drag){
            msg = def->drag(e);
            if (msg){
                unsigned type = cmd->id;
                Command cmd;
                cmd->id      = type;
                cmd->menu_id = MenuMessage;
                cmd->param	 = (void*)m_id;
                if (EventCheckCommandState(cmd).process())
                    break;
            }
        }
    }
    if (msg){
        e->accept();
        if (isDrop){
            msg->setContact(m_id);
            EventOpenMessage(msg).process();
        }
        delete msg;
        return;
    }
    if (!e->mimeData()->text().isEmpty()) {
        e->accept();
        if (isDrop) {
            Message *msg = new Message(MessageGeneric);
            msg->setText(e->mimeData()->text());
            msg->setContact(m_id);
            EventOpenMessage(msg).process();
            delete msg;
        }
        return;
    }
}

bool FloatyWnd::event( QEvent *event ) {
    if( QEvent::ToolTip == event->type() ) {
        do {
            QHelpEvent *e = dynamic_cast<QHelpEvent*>( event );

            Contact *contact = getContacts()->contact(m_id);
            if (contact == NULL)
                break;

            QString tip = contact->tipText();
            QRect tipRect(pos().x(), pos().y(), width(), height());
            QToolTip::showText( mapToGlobal( e->pos() ), tip, this, tipRect );

            return true;
        } while( false );

        QToolTip::hideText();
    }

    return QWidget::event( event );
}

// vim: set expandtab: 


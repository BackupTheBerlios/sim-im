/***************************************************************************
                          toolbtn.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "simgui/toolbtn.h"

#include <QApplication>
#include <QMainWindow>
#include <QComboBox>
#include <QLayout>
#include <QIcon>
#include <QLayout>
#include <QPainter>
#include <QMenu>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolTip>
#include <QContextMenuEvent>
#include <QLabel>
#include <QPixmap>
#include <QHideEvent>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPaintEvent>
#include <QDesktopWidget>
#include <QHash>
#include <QStylePainter>

#include "cmddef.h"
#include "log.h"
#include "icons.h"
#include "misc.h"

using namespace SIM;

/*****************************
 *  CToolButton              *
******************************/
CToolItem::CToolItem(CommandDef *def)
  : m_def(*def)
  , m_text(def->text_wrk)
  , m_action(0)
{
    def->text_wrk.clear();
}

void CToolItem::setCommand(CommandDef *def)
{
    m_text = def->text_wrk;
    def->text_wrk.clear();
    def->bar_id   = m_def.bar_id;
    def->bar_grp  = m_def.bar_grp;
    m_def = *def;
    setState();
}

void CToolItem::setChecked(CommandDef *def)
{
    m_def.flags &= ~COMMAND_CHECKED;
    m_def.flags |= (def->flags & COMMAND_CHECKED);
    setState();
}

void CToolItem::setDisabled(CommandDef *def)
{
    m_def.flags &= ~COMMAND_DISABLED;
    m_def.flags |= (def->flags & COMMAND_DISABLED);
    setState();
}

void CToolItem::setShow(CommandDef *def)
{
    m_def.flags &= ~BTN_HIDE;
    m_def.flags |= (def->flags & BTN_HIDE);
    setState();
}

void CToolItem::setState()
{
    bool bVisible = ((m_def.flags & BTN_HIDE) != BTN_HIDE);
    bool bEnabled = ((m_def.flags & COMMAND_DISABLED) == 0);
    widget()->setVisible(bVisible);
    widget()->setEnabled(bEnabled);
    if (m_action) {
        m_action->setVisible(bVisible);
        m_action->setEnabled(bEnabled);
    }
}

void CToolItem::checkState()
{
    if (m_def.flags & COMMAND_CHECK_STATE)
    {
        m_def.param = static_cast<CToolBar*>(widget()->parent())->param();
        EventCheckCommandState(&m_def).process();
        m_def.flags |= COMMAND_CHECK_STATE;
        setState();
    }
}

/*****************************
 *  CToolButton              *
******************************/
CToolButton::CToolButton (CToolBar *parent, CommandDef *def)
        : QToolButton(parent), CToolItem(def)
{
    accelKey = 0;
    connect(this, SIGNAL(clicked()), this, SLOT(btnClicked()));
    connect(this, SIGNAL(toggled(bool)), this, SLOT(btnToggled(bool)));
    if(!def->accel.isEmpty())
    {
        QAction *pAction = new QAction( this );
        pAction->setShortcut( QKeySequence( def->accel ) );
        connect( pAction, SIGNAL(triggered()), SLOT(accelActivated()) );
    }
    setState();
}

CToolButton::~CToolButton()
{
    emit buttonDestroyed();
}

void CToolButton::setTextLabel(const QString &text)
{
    m_text = text;
}

void CToolButton::setTextLabel()
{
    QString text = m_text;
    if (text.isEmpty()) {
      text = i18n(m_def.text.toUtf8().constData());
    }
    int key = QKeySequence(text);
    setAccel(key);
    QString t = text;
    int pos = t.indexOf("<br>");
    if (pos >= 0) t = t.left(pos);
    setText(t);
    t = text;
    while ((pos = t.indexOf('&')) >= 0){
        t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
    }
    setToolTip(t);
}

void CToolButton::setState()
{
    setTextLabel();
    if(!m_def.icon_on.isEmpty())
    {
        setCheckable(true);
        QToolButton::setChecked((m_def.flags & COMMAND_CHECKED) != 0);
    }
    QIcon icon;
    if((!m_def.icon_on.isEmpty()) && (m_def.icon != m_def.icon_on))
    {
        QPixmap on = Pict(m_def.icon_on);
        QPixmap off = Pict(m_def.icon);
        icon.addPixmap(on,QIcon::Normal,QIcon::On);
        icon.addPixmap(off,QIcon::Normal,QIcon::Off);
    }
    else
    {
        icon=Icon(m_def.icon);
    }
    setIcon(icon);
    CToolItem::setState();
}

void CToolButton::accelActivated(int)
{
    btnClicked();
}

void CToolButton::btnClicked()
{
    m_def.param = static_cast<CToolBar*>(parent())->param();
    if (m_def.popup_id){
        EventMenuGet e(&m_def);
        e.process();
        QMenu *popup = e.menu();
        if (popup){
            QPoint pos = popupPos(popup);
            popup->popup(pos);
        }
        return;
    }
    if (isCheckable())
        return;
    EventCommandExec(&m_def).process();
}

void CToolButton::btnToggled(bool state)
{
    m_def.param = static_cast<CToolBar*>(parent())->param();
    if (!isCheckable())
        return;
    if (state){
        m_def.flags |= COMMAND_CHECKED;
    }else{
        m_def.flags &= ~COMMAND_CHECKED;
    }
    EventCommandExec(&m_def).process();
}

QPoint CToolButton::popupPos(QWidget *p)
{
    return popupPos(this, p);
}

QPoint CToolButton::popupPos(QWidget *btn, QWidget *p)
{
    QPoint pos;
    QToolBar *bar = NULL;
    for (QWidget *pw = btn->parentWidget(); pw; pw = pw->parentWidget()){
        if (pw->inherits("QToolBar")){
            bar = static_cast<QToolBar*>(pw);
            break;
        }
    }
    QWidget* desktop = qApp->desktop();
    QSize s = p->sizeHint();
    s = p->sizeHint();
	if(bar)
		if(bar->orientation() == Qt::Vertical)
			pos = QPoint(btn->width(), 0);
		else
			pos = QPoint(0, btn->height());
	else
		pos = QPoint(btn->width() - s.width(), btn->height());
    pos = btn->mapToGlobal(pos);
    if (pos.x() + s.width() > desktop->width()){
        pos.setX(desktop->width() - s.width());
        if (pos.x() < 0)
            pos.setX(0);
    }
    if (pos.y() + s.height() > desktop->height()){
        pos.setY(pos.y() - btn->height() - s.height());
        if (pos.y() < 0)
            pos.setY(0);
    }
    return pos;
}

void CToolButton::mousePressEvent(QMouseEvent *e)
{
    QToolButton::mousePressEvent(e);
}

void CToolButton::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    emit showPopup(e->globalPos());
}

void CToolButton::setAccel(int key)
{
    accelKey = key;
    if (isVisible())
        QToolButton::setShortcut(key);
}

void CToolButton::showEvent(QShowEvent *e)
{
    enableAccel(true);
    QToolButton::showEvent(e);
}

void CToolButton::hideEvent(QHideEvent *e)
{
    enableAccel(false);
    QToolButton::hideEvent(e);
}

void CToolButton::enableAccel(bool bState)
{
    if (accelKey == 0) return;
    QToolButton::setShortcut(bState ? accelKey : 0);
}

/*****************************
 *  CToolPictButton          *
******************************/
CToolPictButton::CToolPictButton(CToolBar *parent, CommandDef *def)
        : CToolButton(parent, def)
{
    setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setState();
}

CToolPictButton::~CToolPictButton()
{
}

QSize CToolPictButton::minimumSizeHint() const
{
    QSize size = QToolButton::minimumSizeHint();
    QToolBar *bar = static_cast<QToolBar*>(parent());
    if(bar->orientation() == Qt::Vertical)
    {
		// minimumSizeHint seem to call sizeHint, and it already transposes size,
		// so we don't have to transpose it here
        //size.transpose();
    }
    return size;
}

QSize CToolPictButton::sizeHint() const
{
    QSize size = QToolButton::sizeHint();
    QToolBar *bar = static_cast<QToolBar*>(parent());
    if(bar->orientation() == Qt::Vertical)
    {
        size.transpose();
    }
    return size;
}

void CToolPictButton::setState()
{
    setTextLabel();
    CToolButton::setState();
    repaint();
}

void CToolPictButton::paintEvent(QPaintEvent * /*e*/)
{
    QStyleOptionToolButton opt;
    initStyleOption( &opt );
    QToolBar *bar = static_cast<QToolBar*>( parent() );
    QRect rect = opt.rect;
    if( bar->orientation() == Qt::Vertical ) {
        opt.rect.setHeight( rect.width() );
        opt.rect.setWidth( rect.height() );
    }
    QPixmap pixmap( opt.rect.width(), opt.rect.height() );
    pixmap.fill( Qt::transparent );
    QStylePainter p( &pixmap, this );
    p.drawComplexControl( QStyle::CC_ToolButton, opt );
    QPainter rp( this );
    QMatrix m;
    if( bar->orientation() == Qt::Vertical ) {
        m.rotate( 90. );
    }
    rp.drawPixmap( rect, pixmap.transformed( m ) );
    return;
}

/*****************************
 *  CToolCombo               *
******************************/
CToolCombo::CToolCombo(CToolBar *parent, CommandDef *def, bool bCheck)
        : QComboBox(parent), CToolItem(def)
{
    m_bCheck = bCheck;
    m_btn    = NULL;
    setEditable(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    if ((def->flags & BTN_NO_BUTTON) == 0){
        m_btn = new CToolButton(parent, def);
        connect(m_btn, SIGNAL(buttonDestroyed()), this, SLOT(btnDestroyed()));
        if (bCheck)
            connect(lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(slotTextChanged(const QString&)));
    }
    setState();
    setMinimumSize(minimumSizeHint());
}

CToolCombo::~CToolCombo()
{
    delete m_btn;
}

void CToolCombo::setText(const QString &str)
{
    lineEdit()->setText(str);
}

void CToolCombo::btnDestroyed()
{
    m_btn = NULL;
}

void CToolCombo::slotTextChanged(const QString &str)
{
    if (m_btn && m_bCheck){
        m_btn->setEnabled(!str.isEmpty());
        if (str.isEmpty())
            m_btn->QToolButton::setChecked(false);
    }
}

void CToolCombo::setState()
{
    CToolItem::setState();
    if (!m_def.text.isEmpty()){
        QString t = i18n(m_def.text);
        int pos;
        while ((pos = t.indexOf('&')) >= 0)
            t = t.left(pos) + "<u>" + t.mid(pos+1, 1) + "</u>" + t.mid(pos+2);
        setToolTip(t);
    }
    if (m_btn){
        m_btn->setDef(m_def);
        m_btn->setState();
        if (m_bCheck)
            m_btn->setEnabled(!lineEdit()->text().isEmpty());
    }
}

QSizePolicy CToolCombo::sizePolicy() const
{
    QSizePolicy p = QComboBox::sizePolicy();
    p.setHorizontalPolicy(QSizePolicy::Expanding);
    return p;
}

QSize CToolCombo::minimumSizeHint() const
{
    int wChar = QFontMetrics(font()).width('0');
    int w = wChar * 10 + 6;
    int h = 22;
    return QSize(w, h);
}

QSize CToolCombo::sizeHint() const
{
    int wChar = QFontMetrics(font()).width('0');
    int w = wChar * 15 + 6;
    int h = 22;
    return QSize(w, h);
}


/*****************************
 *  CToolEdit                *
******************************/
CToolEdit::CToolEdit(CToolBar *parent, CommandDef *def)
        : QLineEdit(parent), CToolItem(def)
{
    m_btn = NULL;
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    if ((def->flags & BTN_NO_BUTTON) == 0){
        m_btn = new CToolButton(parent, def);
        connect(m_btn, SIGNAL(buttonDestroyed()), this, SLOT(btnDestroyed()));
    }
    setState();
}

CToolEdit::~CToolEdit()
{
    delete m_btn;
}

void CToolEdit::btnDestroyed()
{
    m_btn = NULL;
}

void CToolEdit::setState()
{
    CToolItem::setState();
    if (m_btn){
        m_btn->setDef(m_def);
        m_btn->setState();
    }
}

QSizePolicy CToolEdit::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

QSize CToolEdit::minimumSizeHint() const
{
    return QLineEdit::minimumSizeHint();
}

QSize CToolEdit::sizeHint() const
{
    return QLineEdit::sizeHint();
}


/*****************************
 *  CToolLabel               *
******************************/
CToolLabel::CToolLabel(CToolBar *parent, CommandDef *def)
        : QLabel(parent), CToolItem(def)
{}

CToolLabel::~CToolLabel()
{}

QSizePolicy CToolLabel::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize CToolLabel::minimumSizeHint() const
{
    return pixmap() ? pixmap()->size() : QSize();
}

QSize CToolLabel::sizeHint() const
{
    return pixmap() ? pixmap()->size() : QSize();
}

/*****************************
 *  CToolBar                 *
******************************/
CToolBar::CToolBar(CommandsDef *def, QMainWindow *parent)
  : QToolBar(parent)
  , EventReceiver(LowPriority)
  , m_def(def)
{ 
//    setIconSize(QSize(16,16));
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    bChanged = false;
    m_param = this;
    toolBarChanged();
//    parent->addToolBar(Qt::AllToolBarAreas, this);
}

CToolBar::~CToolBar()
{
    qDeleteAll(buttons);
}

void CToolBar::checkState()
{
    QHashIterator<unsigned, CToolItem*> it(buttons);
    while(it.hasNext())
        it.next().value()->checkState();
}

void CToolBar::mousePressEvent(QMouseEvent *e)
{
    QToolBar::mousePressEvent(e);
}

void CToolBar::contextMenuEvent(QContextMenuEvent *e)
{
    e->accept();
    showPopup(e->globalPos());
}

bool CToolBar::processEvent(Event *e)
{
	switch (e->type()){
		case eEventToolbarChanged:
			{
				EventToolbarChanged *tc = static_cast<EventToolbarChanged*>(e);
				if (tc->defs() == m_def)
					toolBarChanged();
				break;
			}
		case eEventCommandRemove:
			{
				EventCommandRemove *ecr = static_cast<EventCommandRemove*>(e);
				delete  buttons.take(ecr->id());
				break;
			}
		case eEventCommandWidget:
			{
				EventCommandWidget *ecw = static_cast<EventCommandWidget*>(e);
				CommandDef *cmd = ecw->cmd();
				if ((cmd->param == NULL) || (cmd->param == m_param)){
					ButtonsMap::iterator it = buttons.find(cmd->id);
					if (it != buttons.end())
						ecw->setWidget(it.value()->widget());
					return true;
				}
				return false;
			}
		case eEventLanguageChanged:
		case eEventIconChanged:
			{
                                QHashIterator<unsigned, CToolItem*> it(buttons);
                                while(it.hasNext())
                                    it.next().value()->setState();
				return false;
			}
		case eEventCommandCreate:
			{
				EventCommandCreate *ecc = static_cast<EventCommandCreate*>(e);
				CommandDef *cmd = ecc->cmd();
				if (cmd->bar_id == m_def->id())
					toolBarChanged();
				break;
			}
		case eEventCommandChange:
			{
				EventCommandChange *ecc = static_cast<EventCommandChange*>(e);
				CommandDef *cmd = ecc->cmd();
				if ((cmd->param == NULL) || (cmd->param == m_param)){
					ButtonsMap::iterator it = buttons.find(cmd->id);
					if (it != buttons.end())
						it.value()->setCommand(cmd);
				}
				return false;
			}
		case eEventCommandChecked:
			{
				EventCommandChecked *ecc = static_cast<EventCommandChecked*>(e);
				CommandDef *cmd = ecc->cmd();
				if ((cmd->param == NULL) || (cmd->param == m_param)){
					ButtonsMap::iterator it = buttons.find(cmd->id);
					if (it != buttons.end())
						it.value()->setChecked(cmd);
				}
				return false;
			}
		case eEventCommandDisabled:
			{
				EventCommandDisabled *ecd = static_cast<EventCommandDisabled*>(e);
				CommandDef *cmd = ecd->cmd();
				if ((cmd->param == NULL) || (cmd->param == m_param)){
					ButtonsMap::iterator it = buttons.find(cmd->id);
					if (it != buttons.end())
                                        {
                                            it.value()->setDisabled(cmd);
                                            it.value()->checkState();
                                        }
				}
				return false;
			}
		case eEventCommandShow:
			{
				EventCommandShow *ecs = static_cast<EventCommandShow*>(e);
				CommandDef *cmd = ecs->cmd();
				if ((cmd->param == NULL) || (cmd->param == m_param)){
					ButtonsMap::iterator it = buttons.find(cmd->id);
					if (it != buttons.end())
						it.value()->setShow(cmd);
				}
				return false;
			}
		default:
			break;
	}
	return false;
}

void CToolBar::toolBarChanged()
{
	if (bChanged)
		return;
	bChanged = true;
        QHashIterator<unsigned, CToolItem*> it(buttons);
	while(it.hasNext())
		m_def->set(it.next().value()->def());

	clear();
	buttons.clear();
	CommandsList list(*m_def);
	CommandDef *s;
	while((s = ++list) != NULL)
	{
		if (s->id == 0){
			addSeparator();
			continue;
		}
		s->text_wrk = QString::null;
		CToolItem *btn = NULL;
		switch (s->flags & BTN_TYPE)
		{
			case BTN_PICT:
				btn = new CToolPictButton(this, s);
				connect(btn->widget(), SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
				break;
			case BTN_COMBO:
				btn = new CToolCombo(this, s, false);
				break;
			case BTN_COMBO_CHECK:
				btn = new CToolCombo(this, s, true);
				break;
			case BTN_EDIT:
				btn = new CToolEdit(this, s);
				break;
			case BTN_LABEL:
				btn = new CToolLabel(this, s);
				break;
			case BTN_DEFAULT:
				btn = new CToolButton(this, s);
				connect(btn->widget(), SIGNAL(showPopup(QPoint)), this, SLOT(showPopup(QPoint)));
				break;
			default:
				log(L_WARN, "Unknown button type");
		}
		if (btn == NULL)
			continue;
        buttons.insert(s->id, btn);
        btn->setAction(addWidget(btn->widget()));
        CToolItem *pSubItem = btn->subItem();
        if(pSubItem) {
            pSubItem->setAction(addWidget(pSubItem->widget()));
        }
        btn->checkState();
    }
	bChanged = false;
	QTimer::singleShot(0, this, SLOT(checkState()));
}

void CToolBar::showPopup(QPoint p)
{
    QMenu *popup = new QMenu(this);
    popup->popup(p);
    QRect available_geom = QDesktopWidget().availableGeometry();

    QPoint correction=p;
    if (p.x()+popup->width()>available_geom.width())
        correction.setX(available_geom.width()-popup->rect().width());
    if (p.y()+popup->height()>available_geom.height())
        correction.setY(available_geom.height()-popup->rect().height());
    popup->move(correction);

}

QSizePolicy CToolBar::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

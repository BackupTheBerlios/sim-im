/***************************************************************************
                          datepicker.cpp  -  description
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

#include "datepicker.h"
#include "icons.h"
#include "misc.h"
#include "simgui/toolbtn.h"

#include <QCalendarWidget>
#include <QDateEdit>
#include <QDateTime>
#include <QGridLayout>
#include <QPushButton>

class PickerPopup : public QFrame
{
public:
    PickerPopup(DatePicker *parent);
    ~PickerPopup();
};

DatePicker::DatePicker(QWidget *parent)
        : QWidget(parent)
		, lay(new QHBoxLayout(this))
{
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
    //QHBoxLayout *lay = new QHBoxLayout(this);
    m_edit = new QDateEdit(this);
    lay->addWidget(m_edit);
    m_button = new QPushButton(this);
    m_button->setIcon(SIM::Icon("more"));
    lay->addWidget(m_button);
    lay->addStretch();
    connect(m_button, SIGNAL(clicked()), this, SLOT(showPopup()));
}

DatePicker::~DatePicker()
{
}

void DatePicker::setEnabled(bool state)
{
    m_edit->setReadOnly(!state);
    m_button->setEnabled(state);
}

QDate DatePicker::getDate() const
{
    return m_edit->date();
}

void DatePicker::setDate(const QDate &date)
{
    m_edit->setDate(date);
    emit changed();
}

void DatePicker::showPopup()
{
    PickerPopup *popup = new PickerPopup(this);
    QPoint p = CToolButton::popupPos(m_button, popup);
    popup->move(p);
    popup->show();
}

PickerPopup::PickerPopup(DatePicker *picker)
        : QFrame(picker, Qt::Popup)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    setLineWidth(1);

    QCalendarWidget *cw = new QCalendarWidget(this);
    cw->setSelectedDate(picker->getDate());
    connect(cw, SIGNAL(clicked(const QDate&)), picker, SLOT(setDate(const QDate&)));

    QGridLayout *l = new QGridLayout(this);
    l->addWidget(cw);
}

PickerPopup::~PickerPopup()
{
}

// vim: set expandtab:


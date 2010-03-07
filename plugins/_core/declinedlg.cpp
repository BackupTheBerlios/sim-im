/***************************************************************************
                          declinedlg.cpp  -  description
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

#include "icons.h"
#include "misc.h"
#include "declinedlg.h"

#include <QPixmap>
#include <QLineEdit>

using namespace SIM;

DeclineDlg::DeclineDlg(Message *msg) 
    : QDialog(NULL)
    , m_msg (msg)
    //: DeclineDlgBase(NULL, NULL, false, Qt::WDestructiveClose)
{
	setupUi(this);
    
    SET_WNDPROC("decline")
    setWindowIcon(Icon("file"));
    setButtonsPict(this);
}

DeclineDlg::~DeclineDlg()
{
}

void DeclineDlg::accept()
{
    EventMessageDecline(m_msg, edtReason->text());
    QDialog::accept();
}

bool DeclineDlg::processEvent(Event *e)
{
    if (e->type() == eEventMessageDeleted)
    {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->id() == m_msg->id())
            close();
    }
    return false;
}

/*
#ifndef NO_MOC_INCLUDES
#include "declinedlg.moc"
#endif
*/


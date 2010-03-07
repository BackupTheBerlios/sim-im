/***************************************************************************
                          additem.cpp  -  description
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
#include "additem.h"
#include "core.h"
#include "core_events.h"

#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"

#include <QPixmap>
#include <QTimer>
#include <QPushButton>

using namespace SIM;

AddItem::AddItem(QWidget *parent)
  : QDialog(parent)
{
    setupUi(this);
    setModal(true);
    setWindowIcon(Icon("run"));
    setButtonsPict(this);
    QTimer::singleShot(0, this, SLOT(changed()));
    connect(edtItem, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPrg, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(buttonHelp, SIGNAL(clicked()), this, SLOT(help()));
    EventTmplHelpList e;
    e.process();
    edtPrg->setHelpList(e.helpList());
}

void AddItem::changed()
{
    buttonOk->setEnabled(!edtItem->text().isEmpty() && !edtPrg->text().isEmpty());
}

void AddItem::changed(const QString&)
{
    changed();
}

void AddItem::help()
{
    QString helpString = i18n("In command line you can use:") + "\n";
    EventTmplHelp e(helpString);
    e.process();
    BalloonMsg::message(e.help(), buttonHelp, false, 400);
}

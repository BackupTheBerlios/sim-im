/***************************************************************************
                          actioncfg.cpp  -  description
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
#include "actioncfg.h"
#include "menucfg.h"
#include "action.h"
#include "core.h"
#include "simgui/ballonmsg.h"
#include "simgui/editfile.h"

#include <QTabWidget>
#include <QPushButton>
#include <QItemDelegate>

using namespace SIM;

unsigned CONTACT_ONLINE = 0x10000;
unsigned CONTACT_STATUS = 0x10001;

static void addRow(QTableWidget *lstEvent, int row, const QIcon &icon, const QString &text,
                   unsigned int id, const QString &program)
{
    QTableWidgetItem *item;
    lstEvent->setRowCount(row+1);

    item = new QTableWidgetItem(icon, text);
    item->setData(Qt::UserRole, id);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    lstEvent->setItem(row, 0, item);

    item = new QTableWidgetItem(program);
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
    lstEvent->setItem(row, 1, item);
}

ActionConfig::ActionConfig(QWidget *parent, SIM::PropertyHubPtr data, ActionPlugin *plugin)
  : QWidget(parent)
  , m_menu(NULL)
  , m_data(data)
  , m_plugin(plugin)
{
    setupUi(this);
    setButtonsPict(this);

    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));

    int row = 0;
    addRow(lstEvent, row, Icon("SIM"), i18n("Contact online"), CONTACT_ONLINE, data->value("OnLine").toString() );

    row++;
    addRow(lstEvent, row, Icon("SIM"), i18n("Status changed"), CONTACT_STATUS, data->value("Status").toString() );

    CommandDef *cmd;
    CorePlugin *core = GET_CorePlugin();
    CommandsMapIterator it(core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if ((def == NULL) || (cmd->icon.isEmpty()) ||
                (def->flags & (MESSAGE_HIDDEN | MESSAGE_SENDONLY | MESSAGE_CHILD)))
            continue;
        if ((def->singular == NULL) || (def->plural == NULL) ||
                (*def->singular == 0) || (*def->plural == 0))
            continue;
        QString type = i18n(def->singular, def->plural, 1);
        int pos = type.indexOf("1 ");
        if (pos == 0){
            type = type.mid(2);
        }else if (pos > 0){
            type = type.left(pos);
        }
        type = type.left(1).toUpper() + type.mid(1);

        row++;
        addRow(lstEvent, row, Icon(cmd->icon), type, cmd->id, data->stringMapValue("Message", cmd->id));
    }
    EventTmplHelpList e;
    e.process();
    LineEditDelegate *dg = new LineEditDelegate(1, lstEvent);
    dg->setHelpList(e.helpList());
    lstEvent->setItemDelegate(dg);
    lstEvent->resizeColumnToContents(0);
    lstEvent->sortByColumn(0, Qt::AscendingOrder);

    for (QObject *p = parent; p != NULL; p = p->parent()){
        QTabWidget *tab = qobject_cast<QTabWidget*>(p);
        if (!tab)
            continue;
        m_menu = new MenuConfig(tab, data);
        tab->addTab(m_menu, i18n("Menu"));
        tab->adjustSize();
        break;
    }
}

ActionConfig::~ActionConfig()
{
}

void ActionConfig::apply()
{
    PropertyHubPtr data = getContacts()->getUserData("action");
    apply(data);
}

void ActionConfig::apply(PropertyHubPtr data)
{
    //ActionUserData *data = (ActionUserData*)_data;
    if (m_menu)
        m_menu->apply(data);

    for (int row = 0; row < lstEvent->rowCount(); ++row)
    {
        unsigned id = lstEvent->item(row, 0)->data(Qt::UserRole).toUInt();
        const QString text = lstEvent->item(row, 1)->data(Qt::EditRole).toString();

        if (id == CONTACT_ONLINE)
            data->setValue("OnLine", text);
        else if (id == CONTACT_STATUS)
            data->setValue("Status", text);
        else
            data->setStringMapValue("Message",id, text);
    }
}

void ActionConfig::setEnabled(bool state)
{
    if (m_menu)
        m_menu->setEnabled(state);
    QWidget::setEnabled(state);
}

void ActionConfig::help()
{
    QString helpString = i18n("In command line you can use:") + "\n";
    EventTmplHelp e(helpString);
    e.process();
    helpString = e.help();
    helpString += "\n\n";
    helpString += i18n("For message events message text will be sent on standard input of the program.\n"
                       "If the program will return a zero error code message text will be replaced with program output.\n"
                       "If program output is empty,  message will be destroyed.\n"
                       "Thus it is possible to organize additional messages filters.\n");
    BalloonMsg::message(helpString, btnHelp, false, 400);
}


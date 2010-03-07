/***************************************************************************
                          menucfg.cpp  -  description
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


#include "misc.h"
#include "menucfg.h"
#include "action.h"
#include "additem.h"

#include "simgui/editfile.h"

#include <QListWidgetItem>

using namespace SIM;

MenuConfig::MenuConfig(QWidget *parent, PropertyHubPtr data)
  : QWidget(parent)
  , m_data(data)
{
    setupUi(this);

    connect(lstMenu, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(edit()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));

    for (int row = 0; row < m_data->value("NMenu").toInt(); row++){
        lstMenu->setRowCount(row+1);

        const QStringList sl = data->stringMapValue("Menu", row +1).split(';');
        if(sl.count() != 2)
            continue;

        QTableWidgetItem *item = new QTableWidgetItem(sl[0]);
        lstMenu->setItem(row, 0, item);

        item = new QTableWidgetItem(sl[1]);
        lstMenu->setItem(row, 1, item);
    }
    itemSelectionChanged();
}

MenuConfig::~MenuConfig()
{
}

void MenuConfig::itemSelectionChanged()
{
    const bool rowSelected = (lstMenu->selectedItems().count() == 2);
    btnEdit->setEnabled(rowSelected);
    btnRemove->setEnabled(rowSelected);
}

void MenuConfig::add()
{
    AddItem add(topLevelWidget());
    if (add.exec()){
        const int row = lstMenu->rowCount();
        lstMenu->setRowCount(row+1);

        QTableWidgetItem *item = new QTableWidgetItem(add.edtItem->text());
        lstMenu->setItem(row, 0, item);

        item = new QTableWidgetItem(add.edtPrg->text());
        lstMenu->setItem(row, 1, item);

        lstMenu->resizeColumnToContents(0);
    }
}

void MenuConfig::edit()
{
    const int row = lstMenu->currentRow();
    if (row < 0)
        return;
    AddItem add(topLevelWidget());
    add.edtItem->setText(lstMenu->item(row, 0)->text());
    add.edtPrg->setText(lstMenu->item(row, 1)->text());
    if (add.exec()){
        lstMenu->item(row, 0)->setText(add.edtItem->text());
        lstMenu->item(row, 1)->setText(add.edtPrg->text());
        lstMenu->resizeColumnToContents(0);
    }
}

void MenuConfig::remove()
{
    lstMenu->removeRow(lstMenu->currentRow());
}

void MenuConfig::apply(PropertyHubPtr data)
{
    //ActionUserData *data = (ActionUserData*)_data;
    data->clearStringMap("Menu");
    data->setValue("NMenu", 0);
    for (int row = 0; row < lstMenu->rowCount(); ++row){
        QString s = lstMenu->item(row, 0)->text() + ";" + lstMenu->item(row, 1)->text();
        data->setValue("NMenu", data->value("NMenu").toUInt() + 1);
        data->setStringMapValue("Menu", data->value("NMenu").toUInt(), s);
    }
}


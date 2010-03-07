/***************************************************************************
                          iconcfg.cpp  -  description
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

#include <QPushButton>
#include <QListWidget>

#ifdef USE_KDE
# include <kfiledialog.h>
# define QFileDialog	KFileDialog
#else
# include <QFileDialog>
#endif

#include "icons.h"
#include "misc.h"

#include "iconcfg.h"
#include "icon.h"

IconCfg::IconCfg(QWidget *parent, IconsPlugin *plugin)
  : QWidget(parent)
  , m_plugin(plugin)
{
    setupUi(this);
    connect(btnUp, SIGNAL(clicked()), this, SLOT(up()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(down()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(add()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
    connect(lstIcon, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
    if (m_plugin->value("Default").toBool()){
        lstIcon->addItem(QDir::toNativeSeparators("icons/smiles.jisp"));

    }else{
        const QStringList l = m_plugin->value("Icons").toStringList();
        for (int i = 0; i < m_plugin->value("NIcons").toInt(); i++)
        {
            if(i >= l.size())
                break;
            lstIcon->addItem(l[i]);
        }
    }
    itemSelectionChanged();
}

void IconCfg::apply()
{
    m_plugin->setValue("Default", false);
    QStringList l;
    for (int i = 0; i < lstIcon->count(); i++)
        l.append(lstIcon->item(i)->text());
    m_plugin->setValue("Icons", l);
    m_plugin->setValue("NIcons", lstIcon->count());
    m_plugin->setIcons(true);
}

void IconCfg::up()
{
    int n = lstIcon->currentRow();
    if (n < 1)
        return;
    QListWidgetItem *item = lstIcon->takeItem(n);
    lstIcon->insertItem(n - 1, item);
    if (n==lstIcon->count()-1)
        lstIcon->setCurrentRow(lstIcon->currentRow()-1);
    else
        lstIcon->setCurrentRow(lstIcon->currentRow()-2);

    itemSelectionChanged();
}

void IconCfg::down()
{
    int n = lstIcon->currentRow();
    if ((n < 0) || (n >= lstIcon->count() - 1))
        return;
    QListWidgetItem *item = lstIcon->takeItem(n);
    lstIcon->insertItem(n + 1, item);
    lstIcon->setCurrentRow(lstIcon->currentRow()+1);
    itemSelectionChanged();
}

void IconCfg::add()
{
#ifdef USE_KDE
    QString filter = i18n("*.jisp|Icon set");
#else
    QString filter = i18n("Icon set(*.jisp)");
#endif
    QString jisp = QFileDialog::getOpenFileName(topLevelWidget(), i18n("Select icon set"), SIM::app_file("icons/"), filter);
    if (!jisp.isEmpty()) {
        int n = lstIcon->currentRow();
        lstIcon->insertItem(n, QDir::toNativeSeparators(jisp));
        itemSelectionChanged();
    }
}

void IconCfg::remove()
{
    delete lstIcon->takeItem(lstIcon->currentRow());
    itemSelectionChanged();
}

void IconCfg::itemSelectionChanged()
{
    int n = lstIcon->currentRow();
    if (n < 0){
        btnUp->setEnabled(false);
        btnDown->setEnabled(false);
        btnRemove->setEnabled(false);
        return;
    }
    btnRemove->setEnabled(true);
    btnUp->setEnabled(n > 0);
    btnDown->setEnabled(n < lstIcon->count() - 1);
}


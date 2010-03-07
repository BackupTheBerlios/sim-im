/***************************************************************************
                          mousecfg.cpp  -  description
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

#include <QLabel>
#include <QRegExp>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QResizeEvent>
#include <QScrollBar>

#include "log.h"
#include "cmddef.h"
#include "misc.h"
#include "core_consts.h"

#include "mousecfg.h"
#include "shortcuts.h"

using namespace SIM;

MouseConfig::MouseConfig(QWidget *parent, ShortcutsPlugin *plugin) : QWidget(parent)
{
	setupUi(this);
    m_plugin = plugin;
    //lstCmd->setSorting(0);
    loadMenu(MenuMain);
    loadMenu(MenuGroup);
    loadMenu(MenuContact);
    adjustColumns();
    cmbButton->insertItem(INT_MAX,"");
    cmbButton->insertItem(INT_MAX,i18n("Left click"));
    cmbButton->insertItem(INT_MAX,i18n("Right click"));
    cmbButton->insertItem(INT_MAX,i18n("Middle click"));
    cmbButton->insertItem(INT_MAX,i18n("Left dblclick"));
    cmbButton->insertItem(INT_MAX,i18n("Right dblclick"));
    cmbButton->insertItem(INT_MAX,i18n("Middle dblclick"));
    selectionChanged();
    connect(lstCmd, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    connect(cmbButton, SIGNAL(activated(int)), this, SLOT(buttonChanged(int)));
    connect(chkAlt, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    connect(chkCtrl, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
    connect(chkShift, SIGNAL(toggled(bool)), this, SLOT(changed(bool)));
}

MouseConfig::~MouseConfig()
{
}

void MouseConfig::apply()
{
	QMap<QString, QVariant> map;
	for(int i = 0; i < lstCmd->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = lstCmd->topLevelItem(i);
		map.insert(item->text(2), item->text(1).toLatin1());
    }
        m_plugin->setValue("Mouse", map);
}

void MouseConfig::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    adjustColumns();
}

void MouseConfig::adjustColumns()
{
    QScrollBar *bar = lstCmd->verticalScrollBar();
    int wScroll = 0;
    if (bar && bar->isVisible())
        wScroll = bar->width();
    lstCmd->setColumnWidth(0, lstCmd->width() -
                           lstCmd->columnWidth(1) - 4 - wScroll);
}

void MouseConfig::loadMenu(unsigned long id)
{
    EventMenuGetDef eMenu(id);
    eMenu.process();
    CommandsDef *def = eMenu.defs();
    if (def){
        CommandsList list(*def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if ((s->id == 0) || (s->popup_id == 0))
                continue;
            QString title = i18n(s->text);
            if (title == "_")
                continue;
            QTreeWidgetItem *item = NULL;
            for(int i = 0; i < lstCmd->topLevelItemCount(); i++)
            {
                item = lstCmd->topLevelItem(i);
                if (QString::number(s->popup_id) == item->text(3))
                    break;
            }
            if ( NULL != item)
                continue;
            title = title.remove('&');
            QTreeWidgetItem* it = new QTreeWidgetItem(lstCmd, QStringList(title));
            it->setText(1, m_plugin->value("Mouse").toMap().value(QString::number(s->id)).toString());
            it->setText(2, QString::number(s->id));
            it->setText(3, QString::number(s->popup_id));
        }
    }
}

void MouseConfig::selectionChanged()
{
    QTreeWidgetItem *item = lstCmd->currentItem();
    if (item == NULL){
        lblCmd->setText("");
        cmbButton->setCurrentIndex(0);
        cmbButton->setEnabled(false);
        return;
    }
    lblCmd->setText(item->text(0));
    int n = ShortcutsPlugin::stringToButton(item->text(1).toLatin1());
// ToDo: Restore this
//    if (n == 0)
//        chkAlt->setChecked((n & Qt::AltButton) != 0);
//    chkCtrl->setChecked((n & Qt::ControlButton) != 0);
//    chkShift->setChecked((n & Qt::ShiftButton) != 0);
    cmbButton->setEnabled(true);
    cmbButton->setCurrentIndex(n);
    buttonChanged(0);
}

void MouseConfig::buttonChanged(int)
{
    if (cmbButton->currentIndex()){
        chkAlt->setEnabled(true);
        chkCtrl->setEnabled(true);
        chkShift->setEnabled(true);
    }else{
        chkAlt->setChecked(false);
        chkAlt->setEnabled(false);
        chkCtrl->setChecked(false);
        chkCtrl->setEnabled(false);
        chkShift->setChecked(false);
        chkShift->setEnabled(false);
    }
    changed(false);
}

void MouseConfig::changed(bool)
{
    QString res;
// ToDo: Restore this
/*
    int n = cmbButton->currentIndex();
    if (n){
        if (chkAlt->isChecked())
            n |= Qt::AltButton;
        if (chkCtrl->isChecked())
            n |= Qt::ControlButton;
        if (chkShift->isChecked())
            n |= Qt::ShiftButton;
        res = ShortcutsPlugin::buttonToString(n);
    }
*/
    QTreeWidgetItem *item = lstCmd->currentItem();
    if (item == NULL)
        return;
    item->setText(1, res);
    adjustColumns();
}

// vim: set expandtab:

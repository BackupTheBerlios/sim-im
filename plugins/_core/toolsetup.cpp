/***************************************************************************
                          toolsetup.cpp  -  description
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

#include "cmddef.h"
#include "icons.h"
#include "misc.h"

#include "toolsetup.h"
#include "commands.h"


#include <QRegExp>
#include <QPushButton>
#include <QPixmap>

using namespace std;
using namespace SIM;

ToolBarSetup::ToolBarSetup(Commands *bars, CommandsDef *def) : QDialog(NULL)
{
    setupUi(this);
    setObjectName("toolbar_setup");
    SET_WNDPROC("configure")
    setWindowIcon(Icon("configure"));
    setWindowTitle(def->isMenu() ?
               i18n("Customize menu") :
               i18n("Customize toolbar"));

    setButtonsPict(this);
    m_def   = def;
    m_bars  = bars;

    CommandsList list(*m_def);
    CommandDef *s;
    while ((s = ++list) != NULL){
        if (s->id && (s->text.isEmpty()))
            continue;
        active.push_back(s->id);
    }

    setWindowIcon(Icon("setup"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(lstButtons, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    connect(lstActive, SIGNAL(itemSelectionChanged()), this, SLOT(selectionChanged()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClick()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeClick()));
    connect(btnUp, SIGNAL(clicked()), this, SLOT(upClick()));
    connect(btnDown, SIGNAL(clicked()), this, SLOT(downClick()));
    connect(btnOk, SIGNAL(clicked()), this, SLOT(okClick()));
    connect(btnApply, SIGNAL(clicked()), this, SLOT(applyClick()));

    setButtons();
    lstActive->clear();
    for (vector<unsigned>::iterator it = active.begin(); it != active.end(); ++it)
        addButton(lstActive, *it);

    selectionChanged();
    bDirty = false;
}

ToolBarSetup::~ToolBarSetup()
{
}

void ToolBarSetup::okClick()
{
    applyClick();
    close();
}

void ToolBarSetup::applyClick()
{
    if (bDirty)
    {
        QByteArray config;
        vector<unsigned>::iterator it;
        for (it = active.begin(); it != active.end(); ++it){
            if (config.length())
                config += ',';
            config += QByteArray::number(*it);
        }
        bool bFirst = true;
        CommandsList list(*m_def, true);
        CommandDef *s;
        while ((s = ++list) != NULL){
            if ((s->id == 0) || ((m_def->isMenu() ? s->menu_grp : s->bar_grp) == 0))
                continue;
            unsigned id = s->id;
            for (it = active.begin(); it != active.end(); ++it)
                if ((*it) == id)
                    break;
            if (it != active.end())
                continue;
            if (bFirst){
                config += '/';
                bFirst = false;
            }else{
                config += ',';
            }
            config += QByteArray::number(id);
        }
        m_def->setConfig(QString(config));
        m_bars->set(m_def, config);
        bDirty = false;
    }
}

void ToolBarSetup::addButton(QListWidget *lst, unsigned id)
{
    if (id == 0){
        QListWidgetItem* item = new QListWidgetItem(i18n("Separator"), lst);
        item->setIcon(Pict("separator"));
        lst->addItem(item);
        return;
    }
    CommandsList list(*m_def, true);
    CommandDef *s;
    while ((s = ++list) != NULL){
        if ((s->id == id) && !s->text.isEmpty()){
            QString name = i18n(s->text);
            name = name.remove('&');
            QListWidgetItem* item = new QListWidgetItem(name, lst);
            if (!s->icon.isEmpty()){
                item->setIcon(Pict(s->icon));
            }
            return;
        }
    }
}

void ToolBarSetup::selectionChanged()
{
	//orig
    /*
	btnAdd->setEnabled(lstButtons->currentItem() >= 0);
    btnRemove->setEnabled(lstActive->currentItem() >= 0);
    btnUp->setEnabled(lstActive->currentItem() > 0);
    */

	/* new*/
    btnAdd->setEnabled(lstButtons->count() >= 0);
    btnRemove->setEnabled(lstActive->count() >= 0);
    btnUp->setEnabled(lstActive->currentRow() > 0);
	
    btnDown->setEnabled((lstActive->currentRow() >= 0) &&
                        (lstActive->currentRow() < (int)(lstActive->count() - 1)));
}

void ToolBarSetup::setButtons()
{
    lstButtons->clear();
    CommandsList list(*m_def, true);
    CommandDef *s;
    while ((s = ++list) != NULL){
        unsigned id = s->id;
        vector<unsigned>::iterator it_active;
        for (it_active = active.begin(); it_active != active.end(); ++it_active)
            if ((*it_active) == id)
                break;
        if (it_active != active.end())
            continue;
        addButton(lstButtons, id);
    }
    addButton(lstButtons, 0);
}

void ToolBarSetup::addClick()
{
    int i = lstButtons->currentRow();
    if (i < 0)
        return;
    if (i == (int)(lstButtons->count() - 1))
    {
        active.push_back(0);
        addButton(lstActive, 0);
        lstActive->setCurrentRow(lstActive->count() - 1);
        return;
    }
    int n = i;
    CommandsList list(*m_def, true);
    CommandDef *s;
    while (((s = ++list) != NULL) && (i >= 0)){
        unsigned id = s->id;
        vector<unsigned>::iterator it_active;
        for (it_active = active.begin(); it_active != active.end(); ++it_active)
            if ((*it_active) == id)
                break;
        if (it_active != active.end())
            continue;
        if (i-- == 0){
            active.push_back(id);
            addButton(lstActive, id);
            delete lstButtons->item(n);
            lstActive->setCurrentRow(lstActive->count() - 1);
            bDirty = true;
            return;
        }
    }
}

void ToolBarSetup::removeClick()
{
    int i = lstActive->currentRow();
    if (i < 0) return;
    delete lstActive->item(i);
    vector<unsigned>::iterator it = active.begin();
    for (; i > 0; i--, ++it) {};
    active.erase(it);
    setButtons();
    bDirty = true;
}

void ToolBarSetup::upClick()
{
    int i = lstActive->currentRow();
    if (i <= 0) return;

    lstActive->insertItem ( i - 1, lstActive->takeItem ( i ) );
    lstActive->setCurrentRow(i - 1);

    unsigned old = active[i - 1];
    active[i - 1] = active[i];
    active[i] = old;

    bDirty = true;
}

void ToolBarSetup::downClick()
{
    int i = lstActive->currentRow();
    if ((i < 0) || (i >= (int)(lstActive->count() - 1))) return;
    
    lstActive->insertItem ( i + 1, lstActive->takeItem ( i ) );
    lstActive->setCurrentRow(i + 1);

    unsigned old = active[i + 1];
    active[i + 1] = active[i];
    active[i] = old;

    bDirty = true;
}

// vim: set expandtab: 


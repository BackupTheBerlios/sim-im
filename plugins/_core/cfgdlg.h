/***************************************************************************
                          cfgdlg.h  -  description
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

#ifndef _CFGDLG_H
#define _CFGDLG_H

#include "ui_cfgdlgbase.h"
#include <QCloseEvent>
#include "event.h"
#include "contacts/client.h"

namespace ConfigDlg
{

using namespace std;
using namespace SIM;

const unsigned CONFIG_ITEM  = 1;
const unsigned PLUGIN_ITEM  = 2;
const unsigned CLIENT_ITEM  = 3;
const unsigned MAIN_ITEM    = 4;
const unsigned AR_ITEM      = 5;

class ConfigItem;

class ConfigureDialog : public QDialog, public Ui::ConfigureDialogBase, public SIM::EventReceiver
{
    Q_OBJECT
public:
    ConfigureDialog();
    ~ConfigureDialog();
    void raisePage(SIM::Client *client);
    void raisePhoneBook();
    void showUpdate(bool bShow);
signals:
    void applyChanges();
    void applyChanges(SIM::Client*, void*);
    void applyContactChanges(SIM::ClientPtr, SIM::IMContact*);
    void finished();
protected slots:
    void apply();
    void updateInfo();
    void itemSelected(QTreeWidgetItem*, QTreeWidgetItem*);
    void raisePage(QWidget*);
    void repaintCurrent();
protected:
    void accept();
    void reject();
    void apply(QTreeWidgetItem *item);
    virtual bool processEvent(SIM::Event*);
    void fill(unsigned id);
    void setTitle();
    bool setCurrentItem(QTreeWidgetItem *parent, unsigned id);
    QTreeWidgetItem *findItem(QWidget *w);
    QTreeWidgetItem *findItem(QWidget *w, QTreeWidgetItem *parent);
    unsigned m_nUpdates;
    bool m_bAccept;
    void closeEvent(QCloseEvent*);
    bool bLanguageChanged;
private:
	ConfigItem *m_parentItem;
};

class ConfigItem : public QTreeWidgetItem
{
public:
    ConfigItem(QTreeWidgetItem *item, unsigned id);
    ConfigItem(QTreeWidget *view, unsigned id);
    ~ConfigItem();
    void show();
    void deleteWidget();
    virtual void apply();
    virtual unsigned type() { return CONFIG_ITEM; }
    unsigned id() { return m_id; }
    static unsigned curIndex;
    bool raisePage(QWidget *w);
    QWidget *widget() { return m_widget; }
    QWidget *m_widget;
protected:
    unsigned m_id;
    static unsigned defId;
    void init(unsigned id);
    virtual QWidget *getWidget(ConfigureDialog *dlg);
};

class MainInfoItem : public ConfigItem
{
public:
    MainInfoItem(QTreeWidget *view, unsigned id);
    unsigned type() { return MAIN_ITEM; }
protected:
    virtual QWidget *getWidget(ConfigureDialog *dlg);
};

class ARItem : public ConfigItem
{
public:
    ARItem(QTreeWidgetItem *view, const CommandDef *d);
    virtual unsigned type() { return AR_ITEM; }
private:
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    unsigned m_status;
};


}
#endif


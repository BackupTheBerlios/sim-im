/***************************************************************************
                          cfgdlg.cpp  -  description
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
#include "cfgdlg.h"
#include "plugincfg.h"
#include "maininfo.h"
#include "arcfg.h"
#include "buffer.h"
#include "core.h"
#include "contacts/client.h"
#include "profilemanager.h"
#include "log.h"

#include <QPixmap>
#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <QCloseEvent>






using namespace ConfigDlg;

//unsigned ConfigItem::defId = 0x10000; //obsoleted?
unsigned ConfigItem::curIndex;  

ConfigItem::ConfigItem(QTreeWidget *view, unsigned id)
        : QTreeWidgetItem(view)
{
    init(id);
}

ConfigItem::ConfigItem(QTreeWidgetItem *item, unsigned id)
        : QTreeWidgetItem(item)
{
    init(id);
}

ConfigItem::~ConfigItem()
{
    //deleteWidget(); //obsolete?
}


void ConfigItem::deleteWidget()//obsolete?
{
    /*if (m_widget){
        delete m_widget;
        m_widget = NULL;
    }*/
}

void ConfigItem::init(unsigned id)
{
    m_widget = NULL;
    m_id = id;
    QString key = QString::number(++curIndex);
    while (key.length() < 4)
        key = '0' + key;
    setText(1, key);
}

bool ConfigItem::raisePage(QWidget *w)
{
    if (m_widget == w)
	{
        treeWidget()->setCurrentItem(this);
        return true;
    }
	for(int i = 0; i < childCount(); i++)
	{
		QTreeWidgetItem* item = child(i);
        if (static_cast<ConfigItem*>(item)->raisePage(w))
            return true;
    }
    return false;
}

void ConfigItem::show()
{
	ConfigureDialog *dlg = static_cast<ConfigureDialog*>(treeWidget()->topLevelWidget());
    if (m_widget == NULL)
	{
        m_widget = getWidget(dlg);
        if (m_widget == NULL)
            return;
        m_id = dlg->wnd->addWidget(m_widget/*, id() ? id() : defId++*/);
        dlg->wnd->setMinimumSize(dlg->wnd->sizeHint());
        QObject::connect(dlg, SIGNAL(applyChanges()), m_widget, SLOT(apply()));
        QTimer::singleShot(50, dlg, SLOT(repaintCurrent()));
    }
    dlg->showUpdate(type() == CLIENT_ITEM);
    dlg->wnd->setCurrentWidget(m_widget);
}

void ConfigItem::apply()
{
}

QWidget *ConfigItem::getWidget(ConfigureDialog*)
{
	return NULL;
}

class PluginItem : public ConfigItem
{
public:
    PluginItem(QTreeWidgetItem *view, const QString& name, const QString& title, unsigned id);
    virtual void apply();
    virtual unsigned type() { return PLUGIN_ITEM; }
private:
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    QString m_name;
};

PluginItem::PluginItem(QTreeWidgetItem *item, const QString& name, const QString& title, unsigned id)
        : ConfigItem(item, id), m_name(name)
{
    setText(0, title);
    setText(1, title);
}

void PluginItem::apply()
{
    if (getPluginManager()->isPluginAlwaysEnabled(m_name))
        return;
    if (m_widget) {
        PluginCfg *w = static_cast<PluginCfg*>(m_widget);
        if (w->chkEnable->isChecked())
            ProfileManager::instance()->currentProfile()->enablePlugin(m_name);
        else
            ProfileManager::instance()->currentProfile()->disablePlugin(m_name);
        delete m_widget;
        m_widget = 0;
    }
}

QWidget *PluginItem::getWidget(ConfigureDialog *dlg)
{
    return new PluginCfg(dlg->wnd, m_name);
}

class ClientItem : public ConfigItem
{
public:
    ClientItem(QTreeWidgetItem *item, Client *client, CommandDef *cmd);
    ClientItem(QTreeWidget *view, Client *client, CommandDef *cmd);
    Client *client() { return m_client;  }
    virtual unsigned type() { return CLIENT_ITEM; }
private:
    void init();
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    CommandDef *m_cmd;
    Client *m_client;
};

ClientItem::ClientItem(QTreeWidgetItem *item, Client *client, CommandDef *cmd)
        : ConfigItem(item, 0)
{
    m_client = client;
    m_cmd	 = cmd;
    init();
}

ClientItem::ClientItem(QTreeWidget *view, Client *client, CommandDef *cmd)
        : ConfigItem(view, 0)
{
    m_client = client;
    m_cmd    = cmd;
    init();
}

void ClientItem::init()
{
    if (!m_cmd->text_wrk.isEmpty()){
        setText(0, m_cmd->text_wrk);
        m_cmd->text_wrk = QString::null;
    }else{
        setText(0, i18n(m_cmd->text));
    }
    if (!m_cmd->icon.isEmpty())
        setIcon(0, Pict(m_cmd->icon));
}

QWidget *ClientItem::getWidget(ConfigureDialog *dlg)
{
    QWidget *res = m_client->configWindow(dlg, m_cmd->id);
    if (res)
        QObject::connect(dlg, SIGNAL(applyChanges(SIM::Client*, void*)), res, SLOT(apply(SIM::Client*, void*)));
    return res;
}

ARItem::ARItem(QTreeWidgetItem *item, const CommandDef *d)
        : ConfigItem(item, 0)
{
    QString icon;

    m_status = d->id;
    setText(0, i18n(d->text));
    switch (d->id){
    case STATUS_ONLINE: 
        icon="SIM_online";
        break;
    case STATUS_AWAY:
        icon="SIM_away";
        break;
    case STATUS_NA:
        icon="SIM_na";
        break;
    case STATUS_DND:
        icon="SIM_dnd";
        break;
	case STATUS_OCCUPIED:
        icon="SIM_occupied";
        break;
    case STATUS_FFC:
        icon="SIM_ffc";
        break;
    case STATUS_OFFLINE:
        icon="SIM_offline";
        break;
    default:
        icon=d->icon;
        break;
    }
    setIcon(0, Pict(icon));
}

QWidget *ARItem::getWidget(ConfigureDialog *dlg)
{
    return new ARConfig(dlg, m_status, text(0), NULL);
}

MainInfoItem::MainInfoItem(QTreeWidget *view, unsigned id)
        : ConfigItem(view, id)
{
    setText(0, i18n("User info"));
    setIcon(0, Pict("info"));
}

QWidget *MainInfoItem::getWidget(ConfigureDialog *dlg)
{
    return new MainInfo(dlg, NULL);
}


using namespace ConfigDlg;

ConfigureDialog::ConfigureDialog() 
	: QDialog(NULL)
	, m_nUpdates(0)
	, m_parentItem(NULL)
{
    setupUi(this);
    setWindowIcon(Icon("configure"));
    setButtonsPict(this);
    setTitle();
    lstBox->header()->hide();
    QIcon iconSet = Icon("webpress");
    if (!iconSet.pixmap(QSize(16,16), QIcon::Normal).isNull())
        btnUpdate->setIcon(iconSet);
    btnUpdate->hide();
    lstBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    fill(0);
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateInfo()));
    connect(lstBox, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)),
            this, SLOT(itemSelected(QTreeWidgetItem*, QTreeWidgetItem*)));
    lstBox->setCurrentItem(lstBox->topLevelItem(0));
    itemSelected(lstBox->topLevelItem(0), 0);
}

ConfigureDialog::~ConfigureDialog()
{
    lstBox->clear();
}

static unsigned itemWidth(QTreeWidgetItem *item, QFontMetrics &fm)
{
    unsigned w = fm.width(item->text(0)) + 64;
	for(int i = 0; i < item->childCount(); i++)
	{
		QTreeWidgetItem *child = item->child(i);
        w = qMax(w, itemWidth(child, fm));
    }
    return w;
}

void ConfigureDialog::fill(unsigned id)
{
    lstBox->clear();
    lstBox->sortItems(1, Qt::AscendingOrder);

    m_parentItem = new MainInfoItem(lstBox, 0);
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        CommandDef *cmds = client->configWindows();
        if (cmds){
            m_parentItem = NULL;
            for (; !cmds->text.isEmpty(); cmds++){
                if (m_parentItem){
                    new ClientItem(m_parentItem, client, cmds);
                }else{
                    m_parentItem = new ClientItem(lstBox, client, cmds);
                    m_parentItem->setExpanded(true);
                }
            }
        }
    }

    unsigned long n;
    m_parentItem = NULL;
    list<unsigned> st;
    for (n = 0; n < getContacts()->nClients(); n++){
        Protocol *protocol = getContacts()->getClient(n)->protocol();
        if ((protocol->description()->flags & (PROTOCOL_AR | PROTOCOL_AR_USER)) == 0)
            continue;
        if (m_parentItem == NULL){
            m_parentItem = new ConfigItem(lstBox, 0);
            m_parentItem->setText(0, i18n("Autoreply"));
            m_parentItem->setExpanded(true);
        }
        for (const CommandDef *d = protocol->statusList(); !d->text.isEmpty(); d++){
            if (((protocol->description()->flags & PROTOCOL_AR_OFFLINE) == 0) &&
                    ((d->id == STATUS_ONLINE) || (d->id == STATUS_OFFLINE)))
                continue;
            list<unsigned>::iterator it;
            for (it = st.begin(); it != st.end(); ++it)
                if ((*it) == d->id)
                    break;
            if (it != st.end())
                continue;
            st.push_back(d->id);
            new ARItem(m_parentItem, d);
        }
    }

    m_parentItem = new ConfigItem(lstBox, 0);
    m_parentItem->setText(0, i18n("Plugins"));
    m_parentItem->setIcon(0, Pict("run"));
    m_parentItem->setExpanded(true);

    QStringList plugins = getPluginManager()->enumPlugins();
    foreach(QString plugin, plugins)
    {
        log(L_DEBUG, "plugin: %s", qPrintable(plugin));
        QString title = getPluginManager()->pluginTitle(plugin);
        if(title.isEmpty())
            continue;
        new PluginItem(m_parentItem, plugin, title, n);
    }

    QFontMetrics fm(lstBox->font());
    unsigned w = 0;
	for(int i = 0; i < lstBox->topLevelItemCount(); i++)
	{
		QTreeWidgetItem *item = lstBox->topLevelItem(i);
        w = qMax(w, itemWidth(item, fm));
    }
    lstBox->setFixedWidth(w);
    lstBox->setColumnWidth(0, w - 2);

    if (id)
    {
        for(int i = 0; i < lstBox->topLevelItemCount(); i++)
        {
            QTreeWidgetItem *item = lstBox->topLevelItem(i);
            if (setCurrentItem(item, id))
                return;
        }
    }
    lstBox->setCurrentItem(lstBox->topLevelItem(0));
}

bool ConfigureDialog::setCurrentItem(QTreeWidgetItem *parent, unsigned id)
{
    if (static_cast<ConfigItem*>(parent)->id() == id){
        lstBox->setCurrentItem(parent);
        return true;
    }
    for(int i = 0; i < parent->childCount(); i++)
    {
    QTreeWidgetItem *item = parent->child(i);
        if (setCurrentItem(item, id))
            return true;
    }
    return false;
}

void ConfigureDialog::closeEvent(QCloseEvent *e)
{
    QDialog::closeEvent(e);
    emit finished();
}

void ConfigureDialog::itemSelected(QTreeWidgetItem *item, QTreeWidgetItem* /* previous */)
{
    if (item)
    {
        static_cast<ConfigItem*>(item)->show();
        lstBox->setCurrentItem(item);
    }
}

void ConfigureDialog::apply(QTreeWidgetItem *item)
{
    static_cast<ConfigItem*>(item)->apply();
    for(int i = 0; i < item->childCount(); i++)
    {
        QTreeWidgetItem* it = item->child(i);
        apply(it);
    }
}

void ConfigureDialog::reject()
{
    QDialog::reject();
    emit finished();
}

void ConfigureDialog::apply()
{
    bLanguageChanged = false;
    m_bAccept = true;
    emit applyChanges();
    if (!m_bAccept)
        return;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        const DataDef *def = client->protocol()->userDataDef();
        if (def == NULL)
            continue;
        size_t size = 0;
        for (const DataDef *d = def; d->name; ++d)
            size += d->n_values;
        Data *data = new Data[size];
        QByteArray cfg = client->getConfig();
        if (cfg.isEmpty()){
            load_data(def, data, NULL);
        }else{
            Buffer config;
            config = "[Title]\n" + cfg;
            config.setWritePos(0);
            config.getSection();
            load_data(def, data, &config);
        }
        emit applyChanges(client, data);
        client->setClientInfo(data);
        free_data(def, data);
        delete[] data;
    }
    for(int i = 0; i < lstBox->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = lstBox->topLevelItem(i);
        apply(item);
    }
    if (bLanguageChanged){
        unsigned id = 0;
        if (lstBox->currentItem())
            id = static_cast<ConfigItem*>(lstBox->currentItem())->id();
        disconnect(lstBox, SIGNAL(currentChanged(QTreeWidgetItem*)), this, SLOT(itemSelected(QTreeWidgetItem*)));
        fill(id);
        connect(lstBox, SIGNAL(currentChanged(QTreeWidgetItem*)), this, SLOT(itemSelected(QTreeWidgetItem*)));
        itemSelected(lstBox->currentItem(), 0);
        buttonApply->setText(i18n("&Apply"));
        buttonOk->setText(i18n("&OK"));
        buttonCancel->setText(i18n("&Cancel"));
        setWindowTitle(i18n("Setup"));
    }
    if (lstBox->currentItem())
        static_cast<ConfigItem*>(lstBox->currentItem())->show();
    EventSaveState e;
    e.process();
}

bool ConfigureDialog::processEvent(Event *e)
{
    if (e->type() == eEventLanguageChanged)
        bLanguageChanged = true;
    if (e->type() == eEventClientsChanged){
        unsigned id = 0;
        if (lstBox->currentItem())
            id = static_cast<ConfigItem*>(lstBox->currentItem())->id();
        fill(id);
    }
    if (e->type() == eEventClientChanged){
        if (m_nUpdates){
            if (--m_nUpdates == 0){
                setTitle();
                btnUpdate->setEnabled(true);
            }
        }
    }
    return false;
}

void ConfigureDialog::setTitle()
{
    QString title = i18n("Configure");
    if (m_nUpdates){
        title += " [";
        title += i18n("Update info");
        title += ']';
    }
    setWindowTitle(title);
}

void ConfigureDialog::accept()
{
    apply();
    if (m_bAccept){
        QDialog::accept();
        emit finished();
    }
}

void ConfigureDialog::showUpdate(bool bShow)
{
    if (bShow){
        btnUpdate->show();
    }else{
        btnUpdate->hide();
    }
}

void ConfigureDialog::updateInfo()
{
    if (m_nUpdates)
        return;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        m_nUpdates++;
        getContacts()->getClient(i)->updateInfo(NULL, NULL);
    }
    btnUpdate->setEnabled(!m_nUpdates);
    setTitle();
}

void ConfigureDialog::raisePage(Client *client)
{
    for(int i = 0; i < lstBox->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = lstBox->topLevelItem(i);
        if (static_cast<ConfigItem*>(item)->type() != CLIENT_ITEM)
            continue;
        if (static_cast<ClientItem*>(item)->client() == client){
            lstBox->setCurrentItem(item);
            //lstBox->ensureItemVisible(item); //FIXME
            return;
        }
    }
}

void ConfigureDialog::raisePage(QWidget *widget)
{
    if (!m_bAccept)
        return;
    for(int i = 0; i < lstBox->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = lstBox->topLevelItem(i);
        if (static_cast<ConfigItem*>(item)->raisePage(widget)){
            m_bAccept = false;
            break;
        }
    }
}

void ConfigureDialog::raisePhoneBook()
{
    lstBox->setCurrentItem(lstBox->topLevelItem(0));
    QWidget *w = static_cast<ConfigItem*>(lstBox->currentItem())->widget();
    if (w == NULL)
        return;
    QList<QTabWidget*> l = topLevelWidget()->findChildren<QTabWidget*>();
	if(l.isEmpty())
		return;
    QTabWidget *tab = l.first();
    if(tab == NULL)
        return;
    tab->setCurrentIndex(2);
}

void ConfigureDialog::repaintCurrent()
{
    QWidget *active = wnd->currentWidget();
    if (active == NULL)
        return;
    active->repaint();
    QTreeWidgetItem *item = findItem(active);
    if (item)
        lstBox->setCurrentItem(item);
    lstBox->repaint();
}

QTreeWidgetItem *ConfigureDialog::findItem(QWidget *w)
{
    for(int i = 0; i < lstBox->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = lstBox->topLevelItem(0);
        QTreeWidgetItem *res = findItem(w, item);
        if (res)
            return res;
    }
    return NULL;
}

QTreeWidgetItem *ConfigureDialog::findItem(QWidget *w, QTreeWidgetItem *parent)
{
    if (static_cast<ConfigItem*>(parent)->m_widget == w)
        return parent;
    for(int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem *item = parent->child(i);
        QTreeWidgetItem *res = findItem(w, item);
        if (res)
            return res;
    }
    return NULL;
}

// vim: set expandtab:

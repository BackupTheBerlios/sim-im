/***************************************************************************
                          interfacecfg.cpp  -  description
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

#ifdef WIN32
#include <windows.h>
#endif

#include <QTabWidget>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>
#include <QDir>

#include "log.h"

#include "interfacecfg.h"
#include "userviewcfg.h"
#include "historycfg.h"
#include "msgcfg.h"
#include "smscfg.h"
#include "core.h"

#ifdef WIN32
static WCHAR key_name[]   = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static WCHAR value_name[] = L"SIM";
#endif

using namespace std;
using namespace SIM;

#ifndef USE_KDE

struct language
{
    const char *code;
    const char *name;
};

static language langs[] =
    {
        { "-", I18N_NOOP("English") },
        { "bg", I18N_NOOP("Bulgarian") },
        { "ca", I18N_NOOP("Catalan") },
        { "cs", I18N_NOOP("Czech") },
        { "de", I18N_NOOP("German") },
        { "el", I18N_NOOP("Greek") },
        { "es", I18N_NOOP("Spanish") },
        { "fr", I18N_NOOP("French") },
        { "he", I18N_NOOP("Hebrew") },
        { "hu", I18N_NOOP("Hungarian") },
        { "it", I18N_NOOP("Italian") },
        { "nl", I18N_NOOP("Dutch") },
        { "pl", I18N_NOOP("Polish") },
        { "pt_BR", I18N_NOOP("Portuguese") },
        { "ru", I18N_NOOP("Russian") },
        { "sk", I18N_NOOP("Slovak") },
        { "sw", I18N_NOOP("Swabian") },
        { "th", I18N_NOOP("Thai") },
        { "tr", I18N_NOOP("Turkish") },
        { "uk", I18N_NOOP("Ukrainian") },
        { "zh_TW", I18N_NOOP("Chinese") },
        { NULL, NULL }
    };
#endif

InterfaceConfig::InterfaceConfig(QWidget *parent)
    : QWidget(parent)
{
	setupUi(this);
    connectControls();

    for(QObject *p = parent; p != NULL; p = p->parent())
	{
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        userview_cfg = new UserViewConfig(tab);
        tab->addTab(userview_cfg, i18n("Contact list"));
        history_cfg = new HistoryConfig(tab);
        tab->addTab(history_cfg, i18n("History"));
		SIM::PropertyHubPtr data = getContacts()->getUserData("_core");
        msg_cfg = new MessageConfig(tab, data);
        tab->addTab(msg_cfg, i18n("Messages"));
        data = getContacts()->getUserData("SMS");
        sms_cfg = new SMSConfig(tab, data);
        tab->addTab(sms_cfg, i18n("SMS"));
        break;
    }
#ifndef USE_KDE
    QString cur = CorePlugin::instance()->value("Lang").toString();
    cmbLang->insertItem(INT_MAX,i18n("System"));
    cmbLang->addItems(getLangItems());
    int nCurrent = 0;
    if(!cur.isEmpty()) 
    {
        const language *l;
        for (l = langs; l->code; l++)
            if (cur == l->code)
                break;
        if (l->code)
            nCurrent = cmbLang->findText(i18n(l->name));
    }
    cmbLang->setCurrentIndex(nCurrent);
#else
    TextLabel1_2->hide();
    cmbLang->hide();
#endif
    
    if (CorePlugin::instance()->getContainerMode())
    {
        optChat->setChecked(true);
        if (CorePlugin::instance()->getContainerMode()==1)
            optNew->setChecked(true);
        if (CorePlugin::instance()->getContainerMode()==2)
            optGroup->setChecked(true);
        if (CorePlugin::instance()->getContainerMode()==3)
            optOne->setChecked(true);
        chkEnter->setChecked(CorePlugin::instance()->value("SendOnEnter").toBool());
    }
    else
    {
        optSimple->setChecked(true);
        grpContainer->setEnabled(false);
    }
    chkSaveFont->setChecked(CorePlugin::instance()->value("EditSaveFont").toBool());
    QString copy2;
    QString copy1 = i18n("Copy %1 messages from history");
    int n = copy1.indexOf("%1");
    if (n >= 0)
    {
        copy2 = copy1.mid(n + 2);
        copy1 = copy1.left(n);
    }
    lblCopy1->setText(copy1);
    lblCopy2->setText(copy2);
    spnCopy->setValue(CorePlugin::instance()->value("CopyMessages").toUInt());
    chkOwnerName->setText(i18n("Show own nickname in window title"));
    chkOwnerName->setChecked(CorePlugin::instance()->value("ShowOwnerName").toBool());
    chkAvatar->setText(i18n("Show user avatar"));
    chkAvatar->setChecked(CorePlugin::instance()->value("ShowAvatarInContainer").toBool());
#ifdef WIN32
    HKEY subKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, key_name, 0,
                      KEY_READ | KEY_QUERY_VALUE, &subKey) == ERROR_SUCCESS){
        DWORD vType = REG_SZ;
        DWORD vCount = 0;
        if (RegQueryValueExW(subKey, value_name, NULL, &vType, NULL, &vCount) == ERROR_SUCCESS)
            chkStart->setChecked(true);
        RegCloseKey(subKey);
    }
#else
    chkStart->hide();
#endif
}

InterfaceConfig::~InterfaceConfig()
{
}

#ifndef USE_KDE

QStringList InterfaceConfig::getLangItems()
{
    QStringList items;
    const language *l;
    for (l = langs; l->code; l++)
    {
        if (strcmp(l->code, "-") == 0)
        {
            items.append(i18n(l->name));
            continue;
        }
        QString ts = CorePlugin::instance()->tsFile(l->code);
        if (ts.isEmpty())
            continue;
        items.append(i18n(l->name));
    }
    items.sort();
    return items;
}

#endif

void InterfaceConfig::modeChanged(int mode)
{
    if (mode==1)
        setOpenEachContactInContainer(true);
    if (mode==2)
        setOpenGroupInContainer(true);
    if (mode==3)
        setOpenAllContactsInOneContainer(true);
}

void InterfaceConfig::setSimpleMode(bool)
{
    disconnectControls();
    grpContainer->setEnabled(false);
    connectControls();
}

void InterfaceConfig::setChatMode(bool)
{
    disconnectControls();
    grpContainer->setEnabled(true);
    chkEnter->setChecked(false);
    connectControls();
    setOpenAllContactsInOneContainer(true);
}

void InterfaceConfig::setOpenEachContactInContainer(bool)
{
    disconnectControls();
    grpContainer->setEnabled(true);
    optNew->setChecked(true);
    chkEnter->setChecked(false);
    connectControls();
}

void InterfaceConfig::setOpenGroupInContainer(bool)
{
    disconnectControls();
    optGroup->setChecked(true);
    chkEnter->setChecked(false);
    connectControls();

}

void InterfaceConfig::setOpenAllContactsInOneContainer(bool)
{
    disconnectControls();
    optOne->setChecked(true);
    chkEnter->setChecked(false);
    connectControls();
}

void InterfaceConfig::connectControls()
{
    connect(optSimple, SIGNAL(toggled(bool)), this, SLOT(setSimpleMode                   (bool)));
    connect(optChat,   SIGNAL(toggled(bool)), this, SLOT(setChatMode                     (bool)));
    connect(optNew,    SIGNAL(toggled(bool)), this, SLOT(setOpenEachContactInContainer   (bool)));
    connect(optGroup,  SIGNAL(toggled(bool)), this, SLOT(setOpenGroupInContainer         (bool)));
    connect(optOne,    SIGNAL(toggled(bool)), this, SLOT(setOpenAllContactsInOneContainer(bool)));
    connect(CorePlugin::instance(),SIGNAL(modeChanged(int)), this, SLOT( modeChanged(int)));
}

void InterfaceConfig::disconnectControls()
{
    disconnect(optSimple, SIGNAL(toggled(bool)), this, SLOT(setSimpleMode                   (bool)));
    disconnect(optChat,   SIGNAL(toggled(bool)), this, SLOT(setChatMode                     (bool)));
    disconnect(optNew,    SIGNAL(toggled(bool)), this, SLOT(setOpenEachContactInContainer   (bool)));
    disconnect(optGroup,  SIGNAL(toggled(bool)), this, SLOT(setOpenGroupInContainer         (bool)));
    disconnect(optOne,    SIGNAL(toggled(bool)), this, SLOT(setOpenAllContactsInOneContainer(bool)));
    disconnect(CorePlugin::instance(),SIGNAL(modeChanged(int)), this, SLOT( modeChanged(int)));
}

void InterfaceConfig::apply()
{
    userview_cfg->apply();
    history_cfg->apply();
	SIM::PropertyHubPtr data = getContacts()->getUserData("_core");
    msg_cfg->apply(data);
    data = getContacts()->getUserData("SMS");
    sms_cfg->apply(data);
    CorePlugin::instance()->setValue("EditSaveFont", chkSaveFont->isChecked());
#ifndef USE_KDE
    int res = cmbLang->currentIndex();
    const char *lang = "";
    if (res > 0)
    {
        QStringList items = getLangItems();
        QString name = items[res - 1];
        const language *l;
        for (l = langs; l->code; l++)
        {
            if (name == i18n(l->name))
            {
                lang = l->code;
                break;
            }
        }
    }
#endif
    int mode = 0;
    if (optSimple->isChecked())
    {
        CorePlugin::instance()->setContainerMode(mode);
        CorePlugin::instance()->setValue("SendOnEnter", false);
    }
    else
    {
        
        if (optNew->isChecked())
            mode = 1;
        if (optGroup->isChecked())
            mode = 2;
        if (optOne->isChecked())
            mode = 3;
        CorePlugin::instance()->setContainerMode(mode);
        CorePlugin::instance()->setValue("SendOnEnter", chkEnter->isChecked());
        CorePlugin::instance()->setValue("CopyMessages", (uint)spnCopy->text().toULong());
    }

    CorePlugin::instance()->setValue("ShowOwnerName", chkOwnerName->isChecked());
    CorePlugin::instance()->setValue("ShowAvatarInContainer", chkAvatar->isChecked());
#ifndef USE_KDE
    if (lang != CorePlugin::instance()->value("Lang").toString())
    {
        CorePlugin::instance()->removeTranslator();
        CorePlugin::instance()->setValue("Lang", lang);
        CorePlugin::instance()->installTranslator();
    }
#endif
#ifdef WIN32
    HKEY subKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, key_name, 0,
                      KEY_WRITE | KEY_QUERY_VALUE, &subKey) == ERROR_SUCCESS){
        if (chkStart->isChecked())
        {
            QString path = app_file("sim.exe");
            DWORD res = RegSetValueExW(subKey, value_name, 0, REG_SZ, (BYTE*)path.utf16(), (path.length() + 1) * 2);
            if (res != ERROR_SUCCESS)
                log(L_WARN, "RegSetValue fail %u", (unsigned int)res);
        }
        else
        {
            DWORD res = RegDeleteValueW(subKey, value_name);
            if (res!=ERROR_SUCCESS && res!=ERROR_FILE_NOT_FOUND)
                log(L_WARN, "RegDeleteValue fail %u", (unsigned int)res);
        }
    }
    RegCloseKey(subKey);
#endif
}

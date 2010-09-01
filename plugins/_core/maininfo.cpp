/***************************************************************************
                          maininfo.cpp  -  description
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
#include "maininfo.h"
#include "editmail.h"
#include "editphone.h"
#include "simgui/listview.h"
#include "core.h"

#include <QLineEdit>
#include <QComboBox>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QLabel>
#include <QTabWidget>
#include <QByteArray>

#include "contacts/contact.h"


using namespace std;
using namespace SIM;

const unsigned PHONE_TYPE		= 0;
const unsigned PHONE_NUMBER		= 1;
const unsigned PHONE_PUBLISH	= 2;
const unsigned PHONE_ICON		= 0x10;
const unsigned PHONE_PROTO		= 0x11;
const unsigned PHONE_TYPE_ASIS	= 0x12;
const unsigned PHONE_ACTIVE		= 0x13;

const unsigned MAIL_ADDRESS		= 0;
const unsigned MAIL_PUBLISH		= 1;
const unsigned MAIL_PROTO		= 0x10;

const char *phoneTypeNames[] =
    {
        I18N_NOOP("Home Phone"),
        I18N_NOOP("Home Fax"),
        I18N_NOOP("Work Phone"),
        I18N_NOOP("Work Fax"),
        I18N_NOOP("Private Cellular"),
        I18N_NOOP("Wireless Pager"),
        NULL
    };

ext_info phoneIcons[] =
    {
        { "phone", PHONE },
        { "fax", FAX },
        { "cell", CELLULAR },
        { "pager", PAGER },
        { NULL, 0 }
    };

MainInfo::MainInfo(QWidget *parent, Contact *contact) : QWidget(parent)
{
	setupUi(this);
    m_contact = contact;
    m_bInit   = false;
    cmbDisplay->setEditable(true);

    QStringList phoneColumns;
    phoneColumns.append(i18n("Type"));
    phoneColumns.append(i18n("Phone"));

    QStringList mailColumns;
    mailColumns.append(i18n("EMail"));
    if(m_contact == NULL)
    {
        phoneColumns.append(i18n("Publish"));
        mailColumns.append(i18n("Publish"));
    }
    lstPhones->setColumnCount(phoneColumns.size());
    lstPhones->setHeaderLabels(phoneColumns);

    lstMails->setColumnCount(mailColumns.size());
    lstMails->setHeaderLabels(mailColumns);

    //lstPhones->setMenu(MenuPhoneList);
    //lstMails->setMenu(MenuMailList);

    if (m_contact == NULL){
        lblCurrent->setText(i18n("I'm currently available at:"));
        cmbStatus->addItem(i18n("Don't show"));
        cmbStatus->addItem(Icon("phone"), i18n("Available"));
        cmbStatus->addItem(Icon("nophone"), i18n("Busy"));
        cmbStatus->setCurrentIndex(getContacts()->owner()->getPhoneStatus());
    }else{
        lblCurrent->setText(i18n("User is crrently available at:"));
        disableWidget(cmbCurrent);
        lblStatus->hide();
        cmbStatus->hide();
    }
    Command cmd;
    cmd->id = CmdPhones;
    if (!EventCheckCommandState(cmd).process()){
        lblCurrent->hide();
        cmbCurrent->hide();
        lblStatus->hide();
        cmbStatus->hide();
    }
    //lstMails->setExpandingColumn(0);
    //lstPhones->setExpandingColumn(PHONE_NUMBER);
    if (m_contact == NULL)
        tabMain->removeTab(tabMain->indexOf(tabNotes));
    fill();
    connect(lstMails, SIGNAL(itemSelectionChanged()), this, SLOT(mailSelectionChanged()));
    connect(lstPhones, SIGNAL(itemSelectionChanged()), this, SLOT(phoneSelectionChanged()));
//    connect(lstMails, SIGNAL(deleteItem(QTreeWidgetItem*)), this, SLOT(deleteMail(QTreeWidgetItem*)));
//    connect(lstPhones, SIGNAL(deleteItem(QTreeWidgetItem*)), this, SLOT(deletePhone(QTreeWidgetItem*)));
    connect(btnMailAdd, SIGNAL(clicked()), this, SLOT(addMail()));
    connect(btnMailEdit, SIGNAL(clicked()), this, SLOT(editMail()));
    connect(btnMailDelete, SIGNAL(clicked()), this, SLOT(deleteMail()));
    connect(btnPhoneAdd, SIGNAL(clicked()), this, SLOT(addPhone()));
    connect(btnPhoneEdit, SIGNAL(clicked()), this, SLOT(editPhone()));
    connect(btnPhoneDelete, SIGNAL(clicked()), this, SLOT(deletePhone()));
}

bool MainInfo::processEvent(Event *e)
{
    switch (e->type()) {
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() == EventContact::eChanged) {
            Contact *contact = ec->contact();
            if (contact == m_contact)
                fill();
        }
        break;
    }
    case eEventCheckCommandState: {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->menu_id == MenuMailList){
            if ((cmd->id != CmdEditList) && (cmd->id != CmdRemoveList))
                return false;
            QTreeWidgetItem *item = (QTreeWidgetItem*)(cmd->param);
            if (item->treeWidget() != lstMails)
                return false;
            cmd->flags &= ~(COMMAND_CHECKED | COMMAND_DISABLED);
            bool bEnable = ((item != NULL) && (item->text(MAIL_PROTO).isEmpty() || (item->text(MAIL_PROTO) == "-")));
            if (!bEnable)
                cmd->flags |= COMMAND_DISABLED;
            return true;
        }
        if (cmd->menu_id == MenuPhoneList){
            if ((cmd->id != CmdEditList) && (cmd->id != CmdRemoveList))
                return false;
            QTreeWidgetItem *item = (QTreeWidgetItem*)(cmd->param);
            if (item->treeWidget() != lstPhones)
                return false;
            cmd->flags &= ~(COMMAND_CHECKED | COMMAND_DISABLED);
            bool bEnable = ((item != NULL) && (item->text(PHONE_PROTO).isEmpty() || (item->text(PHONE_PROTO) == "-")));
            if (!bEnable)
                cmd->flags |= COMMAND_DISABLED;
            return true;
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->menu_id == MenuMailList){
            QTreeWidgetItem *item = (QTreeWidgetItem*)(cmd->param);
            if (item->treeWidget() != lstMails)
                return false;
            bool bEnable = ((item != NULL) && (item->text(MAIL_PROTO).isEmpty() || (item->text(MAIL_PROTO) == "-")));
            if (!bEnable)
                return false;
            if (cmd->id == CmdEditList){
                editMail(item);
                return true;
            }
            if (cmd->id == CmdRemoveList){
                deleteMail(item);
                return true;
            }
        }
        if (cmd->menu_id == MenuPhoneList){
            QTreeWidgetItem *item = (QTreeWidgetItem*)(cmd->param);
            if (item->treeWidget() != lstPhones)
                return false;
            bool bEnable = ((item != NULL) && (item->text(PHONE_PROTO).isEmpty() || (item->text(PHONE_PROTO) == "-")));
            if (!bEnable)
                return false;
            if (cmd->id == CmdEditList){
                editPhone(item);
                return true;
            }
            if (cmd->id == CmdRemoveList){
                deletePhone(item);
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void MainInfo::fill()
{
    Contact *contact = m_contact;
    if (contact == NULL)
        contact = getContacts()->owner();

    QString firstName = contact->getFirstName();
    firstName = getToken(firstName, '/');
    edtFirstName->setText(firstName);
    QString lastName = contact->getLastName();
    lastName = getToken(lastName, '/');
    edtLastName->setText(lastName);

    cmbDisplay->clear();
    QString name = contact->getName();
    if (name.length())
        cmbDisplay->insertItem(INT_MAX,name);
    if (firstName.length() && lastName.length()){
        cmbDisplay->insertItem(INT_MAX,firstName + ' ' + lastName);
        cmbDisplay->insertItem(INT_MAX,lastName + ' ' + firstName);
    }
    if (firstName.length())
        cmbDisplay->insertItem(INT_MAX,firstName);
    if (lastName.length())
        cmbDisplay->insertItem(INT_MAX,lastName);
    cmbDisplay->lineEdit()->setText(contact->getName());

    edtNotes->setPlainText(contact->getNotes());
    QString mails = contact->getEMails();
    lstMails->clear();
    while (mails.length()){
        QString mailItem = getToken(mails, ';', false);
        QString mail = getToken(mailItem, '/');
        QTreeWidgetItem *item = new QTreeWidgetItem(lstMails);
        item->setText(MAIL_ADDRESS, mail);
        item->setText(MAIL_PROTO, mailItem);
        item->setIcon(MAIL_ADDRESS, Pict("mail_generic"));
        if ((m_contact == NULL) && mailItem.isEmpty())
            item->setText(MAIL_PUBLISH, i18n("Yes"));
    }
    mailSelectionChanged();
    QString phones = contact->getPhones();
    lstPhones->clear();
    unsigned n = 1;
    cmbCurrent->clear();
    cmbCurrent->insertItem(INT_MAX,"");
    while (phones.length()){
        unsigned icon = 0;
        QString phone = getToken(phones, ';', false);
        QString phoneItem = getToken(phone, '/', false);
        QString number = getToken(phoneItem, ',');
        QString type = getToken(phoneItem, ',');
        QString proto = phone;

        if (!phoneItem.isEmpty())
            icon = getToken(phoneItem, ',').toULong();
        QTreeWidgetItem *item = new QTreeWidgetItem(lstPhones);
        fillPhoneItem(item, number, type, icon, proto);
        cmbCurrent->insertItem(INT_MAX,number);
        if (!phoneItem.isEmpty()){
            item->setText(PHONE_ACTIVE, "1");
            cmbCurrent->setCurrentIndex(n);
        }
        n++;
    }
    connect(lstPhones, SIGNAL(selectionChanged()), this, SLOT(phoneSelectionChanged()));
    phoneSelectionChanged();
    if (!m_bInit)
        fillEncoding();
}

void MainInfo::apply()
{
    getEncoding(false); /* EventContactChanged kills all our settings ...
                           and we send event also :) */
    Contact *contact = m_contact;
    if (contact == NULL){
        contact = getContacts()->owner();
        contact->setPhoneStatus(cmbStatus->currentIndex());
    }
    contact->setNotes(edtNotes->toPlainText());
    QTreeWidgetItem *item;
    QString mails;
    for (int i = 0; i <lstMails->topLevelItemCount(); i++)
	{
        item = static_cast<QTreeWidgetItem*>(lstMails->topLevelItem(i));
        if (mails.length())
            mails += ';';
        mails += quoteChars(item->text(MAIL_ADDRESS), ";/");
        mails += '/';
        mails += item->text(MAIL_PROTO);
    }
    contact->setEMails(mails);
    QString phones;
    for (int i = 0; i < lstPhones->topLevelItemCount(); i++)
    {
        item = static_cast<QTreeWidgetItem*>(lstPhones->topLevelItem(i));
        if (phones.length())
            phones += ';';
        phones += quoteChars(item->text(PHONE_NUMBER), ";/,");
        phones += ',';
        phones += quoteChars(item->text(PHONE_TYPE_ASIS), ";/,");
        phones += ',';
        phones += item->text(PHONE_ICON);
        if (m_contact){
            if (!item->text(PHONE_ACTIVE).isEmpty())
                phones += ",1";
        }else{
            if (item->text(PHONE_NUMBER) == cmbCurrent->currentText())
                phones += ",1";
        }
        phones += '/';
        phones += item->text(PHONE_PROTO);
    }
    contact->setPhones(phones);
    /* Christian: The checks if the name has changed took longer
       than setting the new value directly */
    contact->setFirstName(edtFirstName->text(), QString::null);
    contact->setLastName(edtLastName->text(), QString::null);

    QString name = cmbDisplay->lineEdit()->text();
    if (name.isEmpty()){
        name = edtFirstName->text();
        if (!edtLastName->text().isEmpty()){
            if (!name.isEmpty()){
                name += ' ';
                name += edtLastName->text();
            }
        }
    }
    contact->setName(name);

    EventContact(contact, EventContact::eChanged).process();
}

void MainInfo::mailSelectionChanged()
{
    QTreeWidgetItem *item = lstMails->currentItem();
    bool bEnable = ((item != NULL) && (item->text(MAIL_PROTO).isEmpty() || (item->text(MAIL_PROTO) == "-")));
    btnMailEdit->setEnabled(bEnable);
    btnMailDelete->setEnabled(bEnable);
}

void MainInfo::phoneSelectionChanged()
{
    QTreeWidgetItem *item = lstPhones->currentItem();
    bool bEnable = ((item != NULL) && (item->text(PHONE_PROTO).isEmpty() || (item->text(PHONE_PROTO) == "-")));
    btnPhoneEdit->setEnabled(bEnable);
    btnPhoneDelete->setEnabled(bEnable);
}

void MainInfo::addMail()
{
    EditMail dlg(this, "", false, m_contact == NULL);
    if (dlg.exec() && !dlg.res.isEmpty()){
        QTreeWidgetItem *item = new QTreeWidgetItem(lstMails);
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish){
            item->setText(MAIL_PUBLISH, i18n("Yes"));
            proto = QString::null;
        }
        item->setText(MAIL_ADDRESS, dlg.res);
        item->setText(MAIL_PROTO, proto);
        item->setIcon(MAIL_ADDRESS, Pict("mail_generic"));
        lstMails->setCurrentItem(item);
    }
}

void MainInfo::editMail()
{
    QTreeWidgetItem *item = lstMails->currentItem();
    editMail(item);
}

void MainInfo::editMail(QTreeWidgetItem *item)
{
    if ((item == NULL) || (!item->text(MAIL_PROTO).isEmpty() && (item->text(MAIL_PROTO) != "-")))
        return;
    EditMail dlg(this, item->text(MAIL_ADDRESS), item->text(MAIL_PROTO).isEmpty(), m_contact == NULL);
    if (dlg.exec() && !dlg.res.isEmpty()){
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish){
            item->setText(MAIL_PUBLISH, i18n("Yes"));
            proto = QString::null;
        }
        item->setText(MAIL_ADDRESS, dlg.res);
        item->setText(MAIL_PROTO, proto);
        item->setIcon(MAIL_ADDRESS, Pict("mail_generic"));
        lstMails->setCurrentItem(item);
    }
}

void MainInfo::deleteMail()
{
    deleteMail(lstMails->currentItem());
}

void MainInfo::deleteMail(QTreeWidgetItem *item)
{
    if ((item == NULL) || (!item->text(MAIL_PROTO).isEmpty() && (item->text(MAIL_PROTO) != "-")))
        return;
    delete item;
}

void MainInfo::addPhone()
{
    EditPhone dlg(this, "", "", PHONE, false, m_contact == NULL);
    if (dlg.exec() && !dlg.number.isEmpty() && !dlg.type.isEmpty()){
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish)
            proto = QString::null;
        fillPhoneItem(new QTreeWidgetItem(lstPhones), dlg.number, dlg.type, dlg.icon, proto);
        fillCurrentCombo();
    }
}

void MainInfo::editPhone()
{
    QTreeWidgetItem *item = lstPhones->currentItem();
    editPhone(item);
}

void MainInfo::editPhone(QTreeWidgetItem *item)
{
    if (item == NULL)
        return;
    QString proto = item->text(PHONE_PROTO);
    if (!proto.isEmpty() && (proto != "-"))
        return;
    EditPhone dlg(this, item->text(PHONE_NUMBER), item->text(PHONE_TYPE_ASIS), item->text(PHONE_ICON).toULong(), item->text(PHONE_PROTO).isEmpty(), m_contact == NULL);
    if (dlg.exec() && !dlg.number.isEmpty() && !dlg.type.isEmpty()){
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish)
            proto = QString::null;
        fillPhoneItem(item, dlg.number, dlg.type, dlg.icon, proto);
        fillCurrentCombo();
    }
}

void MainInfo::deletePhone()
{
    deletePhone(lstPhones->currentItem());
}

void MainInfo::deletePhone(QTreeWidgetItem *item)
{
    if (item == NULL)
        return;
    QString proto = item->text(PHONE_PROTO);
    if (!proto.isEmpty() && (proto != "-"))
        return;
    delete item;
    fillCurrentCombo();
}

void MainInfo::fillPhoneItem(QTreeWidgetItem *item, const QString &number, const QString &type, unsigned icon, const QString &proto)
{
    item->setText(PHONE_PROTO, proto);
    item->setText(PHONE_NUMBER, number);
    item->setText(PHONE_TYPE_ASIS, type);
    if (!type.isEmpty()){
        const QByteArray t = type.toLatin1();
        const char **p;
        for	(p = phoneTypeNames; *p; p++){
            if (!strcmp(*p, t.constData()))
                break;
        }
        if (*p){
            item->setText(PHONE_TYPE, i18n(type));
        }else{
            item->setText(PHONE_TYPE, type);
        }
    }
    item->setText(PHONE_ICON, QString::number(icon));
    for (const ext_info *info = phoneIcons; info->szName; info++){
        if (info->nCode == icon){
            item->setIcon(PHONE_TYPE, Pict(info->szName));
            break;
        }
    }
    if (m_contact == NULL)
        item->setText(PHONE_PUBLISH, proto.isEmpty() ? i18n("Yes") : QString(""));
    //lstPhones->adjustColumn();
    lstPhones->resizeColumnToContents(0);
}

void MainInfo::fillCurrentCombo()
{
    if (m_contact)
        return;
    QString current = cmbCurrent->currentText();
    cmbCurrent->clear();
    cmbCurrent->insertItem(INT_MAX,"");
    int n = 1;
    int cur = 0;
    for (n = 1; n < lstPhones->topLevelItemCount(); n++){
        QTreeWidgetItem* item = static_cast<QTreeWidgetItem*>(lstPhones->topLevelItem(n));
        QString phone = item->text(PHONE_NUMBER);
        if (phone == current)
            cur = n;
        cmbCurrent->insertItem(INT_MAX,phone);
    }
    cmbCurrent->setCurrentIndex(cur);
}

void MainInfo::fillEncoding()
{
    m_bInit = true;
    int current = 0;
    int n_item = 1;
    cmbEncoding->clear();
    cmbEncoding->insertItem(INT_MAX,"Default");
    const ENCODING *e;
    QStringList main;
    QStringList::Iterator it;
    for (e = getContacts()->getEncodings(); e->language; e++){
        if (!e->bMain)
            continue;
        main.append(i18n(e->language) + " (" + e->codec + ')');
    }
    main.sort();
    Contact *contact = m_contact;
    if (contact == NULL)
        contact = getContacts()->owner();
    for (it = main.begin(); it != main.end(); ++it, n_item++){
        QString str = *it;
        int n = str.indexOf('(');
        str = str.mid(n + 1);
        n = str.indexOf(')');
        str = str.left(n);
        if (str == contact->getEncoding())
            current = n_item;
        cmbEncoding->insertItem(INT_MAX,*it);
    }
    QStringList noMain;
    for (e = getContacts()->getEncodings(); e->language; e++){
        if (e->bMain)
            continue;
        noMain.append(i18n(e->language) + " (" + e->codec + ')');
    }
    noMain.sort();
    for (it = noMain.begin(); it != noMain.end(); ++it, n_item++){
        QString str = *it;
        int n = str.indexOf('(');
        str = str.mid(n + 1);
        n = str.indexOf(')');
        str = str.left(n);
        if (str == contact->getEncoding())
            current = n_item;
        cmbEncoding->insertItem(INT_MAX,*it);
    }
    cmbEncoding->setCurrentIndex(current);
}

void MainInfo::getEncoding(bool SendContactChangedEvent)
{
    QString encoding;
    int n = cmbEncoding->currentIndex();
    Contact *contact = m_contact;
    if (contact == NULL)
        contact = getContacts()->owner();
    if (n){
        n--;
        QStringList l;
        const ENCODING *e;
        QStringList main;
        for (e = getContacts()->getEncodings(); e->language; e++){
            if (!e->bMain)
                continue;
            main.append(i18n(e->language) + " (" + e->codec + ')');
        }
        main.sort();
        QStringList::Iterator it;
        for (it = main.begin(); it != main.end(); ++it){
            l.append(*it);
        }
        QStringList noMain;
        for (e = getContacts()->getEncodings(); e->language; e++){
            if (e->bMain)
                continue;
            noMain.append(i18n(e->language) + " (" + e->codec + ')');
        }
        noMain.sort();
        for (it = noMain.begin(); it != noMain.end(); ++it){
            l.append(*it);
        }
        for (it = l.begin(); it != l.end(); ++it){
            if (n-- == 0){
                QString str = *it;
                int n = str.indexOf('(');
                str = str.mid(n + 1);
                n = str.indexOf(')');
                str = str.left(n);
                encoding = str;
                break;
            }
        }
    }
    QString oldEncoding = contact->getEncoding();
    if (oldEncoding == encoding)
        return;
    contact->setEncoding(encoding);
    if (SendContactChangedEvent){
        EventContact(contact, EventContact::eChanged).process();
    }
    EventHistoryConfig(contact->id()).process();
}

// vim: set expandtab:


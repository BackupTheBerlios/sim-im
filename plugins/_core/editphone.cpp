/***************************************************************************
                          editphone.cpp  -  description
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

#include "country.h"
#include "icons.h"
#include "misc.h"

#include "editphone.h"
#include "phonedetails.h"
#include "pagerdetails.h"

#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>

using namespace SIM;

extern ext_info phoneIcons[];
extern const char *phoneTypeNames[];

EditPhone::EditPhone(QWidget *parent, const QString &number, const QString &type, unsigned icon, bool bPublish, bool bShowPublish)
    : QDialog(parent)
    , m_ok      (false)
    , m_phone   (new PhoneDetails(wndDetails, (icon == PAGER) ? QString()   : number    ) )
    , m_pager   (new PagerDetails(wndDetails, (icon == PAGER) ? number      : QString() ) )
{
    setObjectName("editphone");
    setupUi(this);
    SET_WNDPROC("editphone")
    setWindowIcon(Icon("phone"));
    setButtonsPict(this);
    setWindowTitle(number.isEmpty() ? i18n("Add phone number") : i18n("Edit phone number"));
    wndDetails->addWidget(m_phone);
    wndDetails->addWidget(m_pager);
    connect(m_phone, SIGNAL(numberChanged(const QString&, bool)), this, SLOT(numberChanged(const QString&, bool)));
    connect(m_pager, SIGNAL(numberChanged(const QString&, bool)), this, SLOT(numberChanged(const QString&, bool)));
    edtDetails->setReadOnly(true);
    for (const ext_info *icons = phoneIcons; icons->szName; icons++)
        cmbType->insertItem(INT_MAX,Icon(icons->szName),QString());

    for (const char **names = phoneTypeNames; *names; names++)
        cmbName->insertItem(INT_MAX,i18n(*names));

    cmbName->setEditable(true);
    cmbName->lineEdit()->setText(type);
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    connect(cmbName, SIGNAL(textChanged(const QString&)), this, SLOT(nameChanged(const QString&)));
    cmbType->setCurrentIndex(icon);
    typeChanged(icon);
    publish = bPublish;
    if (bShowPublish)
        chkPublish->setChecked(publish);
    else
        chkPublish->hide();
}

void EditPhone::typeChanged(int)
{
    switch (cmbType->currentIndex())
    {
    case 0:
        wndDetails->setCurrentWidget(m_phone);
        m_phone->setExtensionShow(true);
        m_phone->getNumber();
        break;
    case 1:
    case 2:
        wndDetails->setCurrentWidget(m_phone);
        m_phone->setExtensionShow(false);
        m_phone->getNumber();
        break;
    case 3:
        wndDetails->setCurrentWidget(m_pager);
        m_pager->getNumber();
        break;
    }
}

void EditPhone::numberChanged(const QString &number, bool isOK)
{
    edtDetails->setText(number);
    m_ok = isOK;
    changed();
}

void EditPhone::nameChanged(const QString &name)
{
    unsigned i = 0;
    for (const char **p = phoneTypeNames; *p; p++, i++)
        if (name == i18n(*p))
            break;
    switch (i)
    {
    case 0:
    case 2:
        cmbType->setCurrentIndex(0);
        typeChanged(0);
        break;
    case 1:
    case 3:
        cmbType->setCurrentIndex(1);
        typeChanged(1);
        break;
    case 4:
        cmbType->setCurrentIndex(2);
        typeChanged(2);
        break;
    case 5:
        cmbType->setCurrentIndex(3);
        typeChanged(3);
        break;
    }
    changed();
}

void EditPhone::changed()
{
    buttonOk->setEnabled(m_ok && !cmbName->lineEdit()->text().isEmpty());
}

void EditPhone::accept()
{
    number = edtDetails->text();
    type = cmbName->lineEdit()->text();
    for (const char **names = phoneTypeNames; *names; names++)
    {
        if (type != i18n(*names))
            continue;

        type = *names;
        break;
    }
    icon = cmbType->currentIndex();
    publish = chkPublish->isChecked();
    QDialog::accept();
}

/*
#ifndef NO_MOC_INCLUDES
#include "editphone.moc"
#endif
*/

// vim: set expandtab:


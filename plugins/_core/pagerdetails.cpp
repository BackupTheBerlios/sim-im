/***************************************************************************
							pagerdetails.cpp  -  description
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

#include "country.h"
#include "misc.h"

#include "pagerdetails.h"

#include <QComboBox>
#include <QLineEdit>
#include <QFontMetrics>

using namespace SIM;

PagerDetails::PagerDetails(QWidget *p, const QString &oldNumber)
    : QWidget(p)
{
    setupUi(this);
    cmbProvider->setEditable(true);
    for (const pager_provider *provider = getProviders(); *provider->szName; provider++)
        cmbProvider->addItem(provider->szName);
    cmbProvider->lineEdit()->clear();
    connect(cmbProvider, SIGNAL(textChanged(const QString&)), this, SLOT(providerChanged(const QString&)));
    connect(edtNumber, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtGateway, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    QString pagerNumber = oldNumber;
    QString number = getToken(pagerNumber, '@').trimmed();
    QString gateway = getToken(pagerNumber, '[').trimmed();
    QString providerName = getToken(pagerNumber, ']').trimmed();
    cmbProvider->lineEdit()->setText(providerName);
    edtNumber->setText(number);
    edtGateway->setText(gateway);
    providerChanged(cmbProvider->lineEdit()->text());
}

void PagerDetails::getNumber()
{
    bool bOK = true;
    QString res;
    if (!edtNumber->text().isEmpty()){
        res = edtNumber->text();
    }else{
        bOK = false;
    }
    if (!edtGateway->text().isEmpty()){
        res += '@';
        res += edtGateway->text();
    }else{
        bOK = false;
    }
    if (!cmbProvider->lineEdit()->text().isEmpty()){
        res += " [";
        res += cmbProvider->lineEdit()->text();
        res += ']';
    }
    emit numberChanged(res, bOK);
}

void PagerDetails::providerChanged(const QString &str)
{
    for (const pager_provider *p = getProviders(); *p->szName; p++){
        if (str == p->szName){
            edtGateway->setText(p->szGate);
            edtGateway->setEnabled(false);
            getNumber();
            return;
        }
    }
    edtGateway->setEnabled(true);
}

void PagerDetails::textChanged(const QString&)
{
    getNumber();
}

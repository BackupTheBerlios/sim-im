/***************************************************************************
                          workinfo.cpp  -  description
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
#include "workinfo.h"
#include "icqclient.h"
#include "contacts/contact.h"

#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>

using namespace SIM;

WorkInfo::WorkInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client) : QWidget(parent)
{
    setupUi(this);
    m_data    = data;
    m_client  = client;
    m_contact = contact;
    btnSite->setIcon(Icon("home"));
    connect(btnSite, SIGNAL(clicked()), this, SLOT(goUrl()));
    if (m_data){
        edtAddress->setReadOnly(true);
        edtCity->setReadOnly(true);
        edtState->setReadOnly(true);
        edtZip->setReadOnly(true);
        disableWidget(cmbCountry);
        disableWidget(cmbOccupation);
        edtName->setReadOnly(true);
        edtDept->setReadOnly(true);
        edtPosition->setReadOnly(true);
        edtSite->setReadOnly(true);
    }else{
        connect(edtSite, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    }
    fill();
}

void WorkInfo::apply()
{
}

bool WorkInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->have(m_data))
            fill();
    } else
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return false;
}

static const ext_info occupations[] =
    {
        { I18N_NOOP("Academic"), 1  },
        { I18N_NOOP("Administrative"), 2  },
        { I18N_NOOP("Art/Entertainment"), 3  },
        { I18N_NOOP("College Student"), 4  },
        { I18N_NOOP("Computers"), 5  },
        { I18N_NOOP("Community & Social"), 6  },
        { I18N_NOOP("Education"), 7  },
        { I18N_NOOP("Engineering"), 8  },
        { I18N_NOOP("Financial Services"), 9  },
        { I18N_NOOP("Government"), 10  },
        { I18N_NOOP("High School Student"), 11  },
        { I18N_NOOP("Home"), 12  },
        { I18N_NOOP("ICQ - Providing Help"), 13  },
        { I18N_NOOP("Law"), 14  },
        { I18N_NOOP("Managerial"), 15  },
        { I18N_NOOP("Manufacturing"), 16  },
        { I18N_NOOP("Medical/Health"), 17  },
        { I18N_NOOP("Military"), 18  },
        { I18N_NOOP("Non-Goverment Organisation"), 19  },
        { I18N_NOOP("Professional"), 20  },
        { I18N_NOOP("Retail"), 21  },
        { I18N_NOOP("Retired"), 22  },
        { I18N_NOOP("Science & Research"), 23  },
        { I18N_NOOP("Sports"), 24  },
        { I18N_NOOP("Technical"), 25  },
        { I18N_NOOP("University student"), 26  },
        { I18N_NOOP("Web building"), 27  },
        { I18N_NOOP("Other services"), 99  },
        { "", 0  }
    };

const ext_info *p_occupations = occupations;

void WorkInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    edtAddress->setPlainText(data->getWorkAddress());
    edtCity->setText(data->getWorkCity());
    edtState->setText(data->getWorkState());
    edtZip->setText(data->getWorkZip());
    initCombo(cmbCountry, data->getWorkCountry(), getCountries());
    initCombo(cmbOccupation, data->getOccupation(), occupations);
    edtName->setText(data->getWorkName());
    edtDept->setText(data->getWorkDepartment());
    edtPosition->setText(data->getWorkPosition());
    edtSite->setText(data->getWorkHomepage());
    urlChanged(edtSite->text());
}

void WorkInfo::goUrl()
{
    QString url = edtSite->text();
    if (url.isEmpty())
        return;
    EventGoURL e(url);
    e.process();
}

void WorkInfo::urlChanged(const QString &text)
{
    btnSite->setEnabled(!text.isEmpty());
}

void WorkInfo::updateData(ICQUserData* data)
{
    data->setWorkAddress(edtAddress->toPlainText());
    data->setWorkCity(edtCity->text());
    data->setWorkState(edtState->text());
    data->setWorkZip(edtZip->text());
    data->setWorkCountry(getComboValue(cmbCountry, getCountries()));
    data->setOccupation(getComboValue(cmbOccupation, occupations));
    data->setWorkName(edtName->text());
    data->setWorkDepartment(edtDept->text());
    data->setWorkPosition(edtPosition->text());
    data->setWorkHomepage(edtSite->text());
}

void WorkInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
{
    if (client != m_client)
        return;
    updateData(m_client->toICQUserData(contact));
}

void WorkInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
    updateData(data);
}


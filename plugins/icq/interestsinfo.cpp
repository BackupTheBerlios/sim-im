/***************************************************************************
                          interestsinfo.cpp  -  description
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

#include "interestsinfo.h"
#include "icqclient.h"
#include "contacts/contact.h"

#include <QLineEdit>
#include <QComboBox>

using namespace SIM;

InterestsInfo::InterestsInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client) : QWidget(parent)
{
	setupUi(this);
    m_data    = data;
    m_client  = client;
    m_contact = contact;
    if (m_data){
        edtBg1->setReadOnly(true);
        edtBg2->setReadOnly(true);
        edtBg3->setReadOnly(true);
        edtBg4->setReadOnly(true);
        disableWidget(cmbBg1);
        disableWidget(cmbBg2);
        disableWidget(cmbBg3);
        disableWidget(cmbBg4);
    }else{
        connect(cmbBg1, SIGNAL(activated(int)), this, SLOT(cmbChanged(int)));
        connect(cmbBg2, SIGNAL(activated(int)), this, SLOT(cmbChanged(int)));
        connect(cmbBg3, SIGNAL(activated(int)), this, SLOT(cmbChanged(int)));
        connect(cmbBg4, SIGNAL(activated(int)), this, SLOT(cmbChanged(int)));
    }
    fill();
}

void InterestsInfo::apply()
{
}

void InterestsInfo::updateData(ICQUserData* data)
{
    QString info[4];
    info[0] = getInfo(cmbBg1, edtBg1);
    info[1] = getInfo(cmbBg2, edtBg2);
    info[2] = getInfo(cmbBg3, edtBg3);
    info[3] = getInfo(cmbBg4, edtBg4);
    QString res;
    for (unsigned i = 0; i < 4; i++){
        if (info[i].isEmpty())
            continue;
        if (!res.isEmpty())
            res += ';';
        res += info[i];
    }
    data->setInterests(res);
}

void InterestsInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
{
    if (client != m_client)
        return;
    updateData(m_client->toICQUserData(contact));
}

void InterestsInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
    updateData(data);
}

bool InterestsInfo::processEvent(Event *e)
{
    if (e->type() == eEventContact){
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eChanged)
            return false;
        Contact *contact = ec->contact();
        if (contact->have(m_data))
            fill();
    }
    if ((e->type() == eEventClientChanged) && (m_data == 0)){
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        if (ecc->client() == m_client)
            fill();
    }
    return false;
}

static const ext_info interests[] =
    {
        { I18N_NOOP("Art"), 100  },
        { I18N_NOOP("Cars"), 101  },
        { I18N_NOOP("Celebrity Fans"), 102  },
        { I18N_NOOP("Collections"), 103  },
        { I18N_NOOP("Computers"), 104  },
        { I18N_NOOP("Culture & Literature"), 105 },
        { I18N_NOOP("Fitness"), 106  },
        { I18N_NOOP("Games"), 107  },
        { I18N_NOOP("Hobbies"), 108  },
        { I18N_NOOP("ICQ - Providing Help"), 109  },
        { I18N_NOOP("Internet"), 110  },
        { I18N_NOOP("Lifestyle"), 111  },
        { I18N_NOOP("Movies/TV"), 112  },
        { I18N_NOOP("Music"), 113  },
        { I18N_NOOP("Outdoor Activities"), 114  },
        { I18N_NOOP("Parenting"), 115  },
        { I18N_NOOP("Pets/Animals"), 116  },
        { I18N_NOOP("Religion"), 117  },
        { I18N_NOOP("Science/Technology"), 118  },
        { I18N_NOOP("Skills"), 119  },
        { I18N_NOOP("Sports"), 120  },
        { I18N_NOOP("Web Design"), 121  },
        { I18N_NOOP("Nature and Environment"), 122  },
        { I18N_NOOP("News & Media"), 123  },
        { I18N_NOOP("Government"), 124  },
        { I18N_NOOP("Business & Economy"), 125  },
        { I18N_NOOP("Mystics"), 126  },
        { I18N_NOOP("Travel"), 127  },
        { I18N_NOOP("Astronomy"), 128  },
        { I18N_NOOP("Space"), 129  },
        { I18N_NOOP("Clothing"), 130  },
        { I18N_NOOP("Parties"), 131  },
        { I18N_NOOP("Women"), 132  },
        { I18N_NOOP("Social science"), 133  },
        { I18N_NOOP("60's"), 134  },
        { I18N_NOOP("70's"), 135  },
        { I18N_NOOP("80's"), 136  },
        { I18N_NOOP("50's"), 137  },
        { I18N_NOOP("Finance and corporate"), 138  },
        { I18N_NOOP("Entertainment"), 139  },
        { I18N_NOOP("Consumer electronics"), 140  },
        { I18N_NOOP("Retail stores"), 141  },
        { I18N_NOOP("Health and beauty"), 142  },
        { I18N_NOOP("Media"), 143  },
        { I18N_NOOP("Household products"), 144  },
        { I18N_NOOP("Mail order catalog"), 145  },
        { I18N_NOOP("Business services"), 146  },
        { I18N_NOOP("Audio and visual"), 147  },
        { I18N_NOOP("Sporting and athletic"), 148  },
        { I18N_NOOP("Publishing"), 149  },
        { I18N_NOOP("Home automation"), 150  },
        { "", 0  }
    };

const ext_info *p_interests = interests;

void InterestsInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    unsigned i = 0;
    QString str = data->getInterests();
    while (str.length()){
        QString info = getToken(str, ';', false);
        QString n = getToken(info, ',');
        unsigned short category = n.toUShort();
        switch (i){
        case 0:
            edtBg1->setText(unquoteChars(info,";"));
            initCombo(cmbBg1, category, interests);
            break;
        case 1:
            edtBg2->setText(unquoteChars(info,";"));
            initCombo(cmbBg2, category, interests);
            break;
        case 2:
            edtBg3->setText(unquoteChars(info,";"));
            initCombo(cmbBg3, category, interests);
            break;
        case 3:
            edtBg4->setText(unquoteChars(info,";"));
            initCombo(cmbBg4, category, interests);
            break;
        }
        i++;
    }
    for (; i < 4; i++){
        switch (i){
        case 0:
            initCombo(cmbBg1, 0, interests);
            break;
        case 1:
            initCombo(cmbBg2, 0, interests);
            break;
        case 2:
            initCombo(cmbBg3, 0, interests);
            break;
        case 3:
            initCombo(cmbBg4, 0, interests);
            break;
        }
    }
    if (m_data == NULL)
        cmbChanged(0);
}

QString InterestsInfo::getInfo(QComboBox *cmb, QLineEdit *edt)
{
    unsigned n = getComboValue(cmb, interests);
    if (n == 0)
        return QString::null;
    QString res = QString::number(n) + ',';
    res += quoteChars(edt->text(), ";");
    return res;
}

void InterestsInfo::cmbChanged(int)
{
    QComboBox *cmbs[4] = { cmbBg1, cmbBg2, cmbBg3, cmbBg4 };
    QLineEdit *edts[4] = { edtBg1, edtBg2, edtBg3, edtBg4 };
    unsigned n = 0;
    for (unsigned i = 0; i < 4; i++){
        unsigned short value = getComboValue(cmbs[i], interests);
        if (value){
            if (i != n){
                cmbs[n]->setEnabled(true);
                edts[n]->setEnabled(true);
                initCombo(cmbs[n], value, interests, true);
                edts[n]->setText(edts[i]->text());
            }
            edts[n]->setEnabled(true);
            edts[n]->setReadOnly(false);
            n++;
        }
    }
    if (n >= 4)
        return;
    cmbs[n]->setEnabled(true);
    disableWidget(edts[n]);
    cmbs[n]->setCurrentIndex(0);
    edts[n]->setText(QString::null);
    for (n++; n < 4; n++){
        disableWidget(cmbs[n]);
        disableWidget(edts[n]);
        initCombo(cmbs[n], 0, interests, true);
        edts[n]->setText(QString::null);
    }
}


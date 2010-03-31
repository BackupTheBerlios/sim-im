/***************************************************************************
                          moreinfo.cpp  -  description
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
#include "moreinfo.h"
#include "icqclient.h"

#include "contacts/contact.h"
#include "simgui/datepicker.h"

#include <QPushButton>
#include <QDateTime>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>

using namespace SIM;

MoreInfo::MoreInfo(QWidget *parent, ICQUserData *data, unsigned contact, ICQClient *client) : QWidget(parent)
{
	setupUi(this);
    m_data    = data;
    m_client  = client;
    m_contact = contact;
    btnHomePage->setIcon(Icon("home"));
    connect(btnHomePage, SIGNAL(clicked()), this, SLOT(goUrl()));
    spnAge->setSpecialValueText(" ");
    spnAge->setRange(0, 100);
    connect(cmbLang1, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang2, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(cmbLang3, SIGNAL(activated(int)), this, SLOT(setLang(int)));
    connect(edtDate, SIGNAL(changed()), this, SLOT(birthDayChanged()));
    if (m_data){
        disableWidget(spnAge);
        edtHomePage->setReadOnly(true);
        disableWidget(cmbGender);
        disableWidget(edtDate);
        disableWidget(cmbLang1);
        disableWidget(cmbLang2);
        disableWidget(cmbLang3);
    }else{
        connect(edtHomePage, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
    }
    fill();
}

void MoreInfo::apply()
{
}


void MoreInfo::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
{
    if (client != m_client)
        return;
    updateData(m_client->toICQUserData(contact));
}

bool MoreInfo::processEvent(Event *e)
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

const ext_info genders[] =
    {
        { I18N_NOOP("Female"), 1 },
        { I18N_NOOP("Male"), 2 },
        { "", 0 }
    };

const ext_info *p_genders = genders;

const ext_info languages[] =
    {
        {I18N_NOOP("Arabic"), 1},
        {I18N_NOOP("Bhojpuri"), 2},
        {I18N_NOOP("Bulgarian"), 3},
        {I18N_NOOP("Burmese"), 4},
        {I18N_NOOP("Cantonese"), 5},
        {I18N_NOOP("Catalan"), 6},
        {I18N_NOOP("Chinese"), 7},
        {I18N_NOOP("Croatian"), 8},
        {I18N_NOOP("Czech"), 9},
        {I18N_NOOP("Danish"), 10},
        {I18N_NOOP("Dutch"), 11},
        {I18N_NOOP("English"), 12},
        {I18N_NOOP("Esperanto"), 13},
        {I18N_NOOP("Estonian"), 14},
        {I18N_NOOP("Farsi"), 15},
        {I18N_NOOP("Finnish"), 16},
        {I18N_NOOP("French"), 17},
        {I18N_NOOP("Gaelic"), 18},
        {I18N_NOOP("German"), 19},
        {I18N_NOOP("Greek"), 20},
        {I18N_NOOP("Hebrew"), 21},
        {I18N_NOOP("Hindi"), 22},
        {I18N_NOOP("Hungarian"), 23},
        {I18N_NOOP("Icelandic"), 24},
        {I18N_NOOP("Indonesian"), 25},
        {I18N_NOOP("Italian"), 26},
        {I18N_NOOP("Japanese"), 27},
        {I18N_NOOP("Khmer"), 28},
        {I18N_NOOP("Korean"), 29},
        {I18N_NOOP("Lao"), 30},
        {I18N_NOOP("Latvian"), 31},
        {I18N_NOOP("Lithuanian"), 32},
        {I18N_NOOP("Malay"), 33},
        {I18N_NOOP("Norwegian"), 34},
        {I18N_NOOP("Polish"), 35},
        {I18N_NOOP("Portuguese"), 36},
        {I18N_NOOP("Romanian"), 37},
        {I18N_NOOP("Russian"), 38},
        {I18N_NOOP("Serbian"), 39},
        {I18N_NOOP("Slovak"), 40},
        {I18N_NOOP("Slovenian"), 41},
        {I18N_NOOP("Somali"), 42},
        {I18N_NOOP("Spanish"), 43},
        {I18N_NOOP("Swahili"), 44},
        {I18N_NOOP("Swedish"), 45},
        {I18N_NOOP("Tagalog"), 46},
        {I18N_NOOP("Tatar"), 47},
        {I18N_NOOP("Thai"), 48},
        {I18N_NOOP("Turkish"), 49},
        {I18N_NOOP("Ukrainian"), 50},
        {I18N_NOOP("Urdu"), 51},
        {I18N_NOOP("Vietnamese"), 52},
        {I18N_NOOP("Yiddish"), 53},
        {I18N_NOOP("Yoruba"), 54},
        {I18N_NOOP("Afrikaans"), 55},
        {I18N_NOOP("Persian"), 57},
        {I18N_NOOP("Albanian"), 58},
        {I18N_NOOP("Armenian"), 59},
        {I18N_NOOP("Kyrgyz"), 123},
        {I18N_NOOP("Maltese"), 125},
        {"", 0}
    };

const ext_info *p_languages = languages;

void MoreInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL)
        data = &m_client->data.owner;
    edtHomePage->setText(data->getHomepage());
    initCombo(cmbGender, data->getGender(), genders);
    if (spnAge->text() == "0")
        spnAge->setSpecialValueText(QString::null);
    
    if (data->getBirthYear() > 0 && data->getBirthMonth() > 0 && data->getBirthDay() > 0) {
		QDate date;
        date.setYMD(data->getBirthYear(), data->getBirthMonth(), data->getBirthDay());
		edtDate->setDate(date);
		birthDayChanged();
	}

    unsigned l = data->getLanguage();
    char l1 = (char)(l & 0xFF);
    l = l >> 8;
    char l2 = (char)(l & 0xFF);
    l = l >> 8;
    char l3 = (char)(l & 0xFF);
    initCombo(cmbLang1, l1, languages);
    initCombo(cmbLang2, l2, languages);
    initCombo(cmbLang3, l3, languages);
    setLang(0);
    urlChanged(edtHomePage->text());
}

void MoreInfo::birthDayChanged()
{
    int day = edtDate->getDate().day();
    int month = edtDate->getDate().month();
    int year = edtDate->getDate().year();
    if (year){
        QDate now = QDate::currentDate();
        int age = now.year() - year;
        if ((now.month() < month) || ((now.month() == month) && (now.day() < day))) age--;
        if (age < 100){
            spnAge->setValue(age);
        }else{
            spnAge->setValue(0);
        }
    }else{
        spnAge->setValue(0);
    }
}

void MoreInfo::goUrl()
{
    QString url = edtHomePage->text();
    if (url.isEmpty())
        return;
    if(!url.startsWith("http://"))
        url = "http://" + url;
    EventGoURL e(url);
    e.process();
}

void MoreInfo::setLang(int)
{
    unsigned l[3], sl[3];
    l[0] = cmbLang1->currentIndex();
    l[1] = cmbLang2->currentIndex();
    l[2] = cmbLang3->currentIndex();
    unsigned j = 0;
    for (unsigned i = 0; i < 3; i++)
        if (l[i])
            sl[j++] = l[i];
    for (; j < 3; j++)
        sl[j] = 0;
    cmbLang1->setCurrentIndex(sl[0]);
    cmbLang2->setCurrentIndex(sl[1]);
    cmbLang3->setCurrentIndex(sl[2]);
    cmbLang2->setEnabled(sl[0] != 0);
    cmbLang3->setEnabled(sl[1] != 0);
}

void MoreInfo::updateData(ICQUserData* data)
{
    data->setHomepage(edtHomePage->text());
    data->setGender(getComboValue(cmbGender, genders));
    data->setBirthMonth(edtDate->getDate().month());
    data->setBirthDay(edtDate->getDate().day());
    data->setBirthYear(edtDate->getDate().year());
    unsigned l1 = getComboValue(cmbLang1, languages);
    unsigned l2 = getComboValue(cmbLang2, languages);
    unsigned l3 = getComboValue(cmbLang3, languages);
    data->setLanguage((l3 << 16) | (l2 << 8) | l1);
}

void MoreInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = m_client->toICQUserData((SIM::IMContact*)_data);  // FIXME unsafe type conversion
    updateData(data);
}

void MoreInfo::urlChanged(const QString &text)
{
    btnHomePage->setEnabled(!text.isEmpty());
}


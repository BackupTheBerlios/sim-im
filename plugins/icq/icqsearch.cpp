/***************************************************************************
                          icqsearch.cpp  -  description
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
#include "icqsearch.h"
#include "icqclient.h"
#include "advsearch.h"
#include "aimsearch.h"
#include "simgui/intedit.h"
#include "log.h"
#include "contacts/contact.h"

#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QShowEvent>

using namespace std;
using namespace SIM;

ICQSearch::ICQSearch(ICQClient *client, QWidget *parent) : QWidget(parent)
{
	setupUi(this);
    m_client = client;
    m_bAdv	 = false;
    m_id_icq = 0;
    m_id_aim = 0;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
    connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
    if (client->m_bAIM){
        m_adv    = new AIMSearch;
        emit addResult(m_adv);

        edtAOL_UIN->setValidator(new QRegExpValidator(QRegExp("([ -]*[0-9]){4,13}[ -]*"), this));
        edtScreen->setValidator(new QRegExpValidator(QRegExp("[0-9A-Za-z]+"), this));
        connect(grpScreen,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        connect(grpAOL_UIN,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        grpUin->hide();
        grpAOL->hide();
        grpName->hide();
    }else{
        m_adv    = new AdvSearch;
        emit addResult(m_adv);

        edtUIN->setValidator(new QRegExpValidator(QRegExp("([ -]*[0-9]){4,13}[ -]*"), this));
        edtAOL->setValidator(new QRegExpValidator(QRegExp("[0-9A-Za-z]+"), this));
        connect(grpUin,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        connect(grpAOL,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        connect(grpName, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        grpScreen->hide();
        grpAOL_UIN->hide();
    }
    edtMail->setValidator(new EMailValidator(edtMail));
    connect(grpMail, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(btnAdvanced, SIGNAL(clicked()),	this, SLOT(advClick()));
    QIcon is = Icon("1rightarrow");
    btnAdvanced->setIcon(is);
}

ICQSearch::~ICQSearch()
{
    if (m_adv)
        delete m_adv;
}

void ICQSearch::advDestroyed()
{
    m_adv = NULL;
}

void ICQSearch::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    emit setAdd(grpAOL->isChecked() || grpScreen->isChecked());
    if (m_adv && m_bAdv)
        emit showResult(m_adv);
}

void ICQSearch::radioToggled(bool)
{
    setAdv(false);
    emit setAdd(grpAOL->isChecked() || grpScreen->isChecked());
}

void ICQSearch::advClick()
{
    if (!m_bAdv && (m_id_icq || m_id_aim)){
        m_id_icq = 0;
        m_id_aim = 0;
        emit searchDone(this);
    }
    setAdv(!m_bAdv);
}

void ICQSearch::setAdv(bool bAdv)
{
	if (m_bAdv == bAdv)
		return;
	m_bAdv = bAdv;
	QIcon is = Icon(m_bAdv ? "1leftarrow" : "1rightarrow");
    btnAdvanced->setIcon(is);
	if (m_bAdv)
	{
		if (m_client->m_bAIM)
		{
			edtMail->setEnabled(false);
			edtAOL_UIN->setEnabled(false);
			edtScreen->setEnabled(false);
		}
		else
		{
			edtMail->setEnabled(true);
			edtFirst->setEnabled(true);
			edtLast->setEnabled(true);
			edtNick->setEnabled(true);
			lblFirst->setEnabled(true);
			lblLast->setEnabled(true);
			lblNick->setEnabled(true);
			edtUIN->setEnabled(false);
			edtAOL->setEnabled(false);
		}
		emit setAdd(false);
	}
	else
	{
		if (m_client->m_bAIM)
		{
            grpScreen->setChecked( true );
            grpAOL_UIN->setChecked( true );
		}
		else
		{
            grpUin->setChecked( false );
            grpAOL->setChecked( false );
            grpName->setChecked( false );
		}
        grpMail->setChecked( false );
		radioToggled(false);
	}
	emit showResult(m_bAdv ? m_adv : NULL);
}

void ICQSearch::createContact(unsigned tmpFlags, Contact *&contact)
{
    if (m_client->m_bAIM){

        if (grpScreen->isChecked() && !edtScreen->text().isEmpty())

            add(edtScreen->text(), tmpFlags, contact);

        if (grpAOL_UIN->isChecked() && !edtAOL_UIN->text().isEmpty())

            add(extractUIN(edtAOL_UIN->text()), tmpFlags, contact);

    }else{

        if (grpAOL->isChecked() && !edtAOL->text().isEmpty())
            add(edtAOL->text(), tmpFlags, contact);

    }
}

void ICQSearch::add(const QString &screen, unsigned tmpFlags, Contact *&contact)
{
    if (m_client->findContact(screen, NULL, false, contact))
        return;
    m_client->findContact(screen, &screen, true, contact, NULL, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
}

extern const ext_info *p_ages;
extern const ext_info *p_genders;
extern const ext_info *p_languages;
extern const ext_info *p_occupations;
extern const ext_info *p_interests;
extern const ext_info *p_pasts;
extern const ext_info *p_affilations;

void ICQSearch::icq_search()
{
    m_bAdd = false;
    switch (m_type){
    case UIN:
        m_id_icq = m_client->findByUin(m_uin);
        break;
    case Mail:
        m_id_icq = m_client->findByMail(m_mail);
        break;
    case Name:
        m_id_icq = m_client->findWP(m_first, m_last, m_nick,
                                    NULL, 0, 0, 0, NULL, NULL, 0, NULL, NULL, NULL,
                                    0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, false);
        break;
    case Full:
        m_id_icq = m_client->findWP(m_first, m_last, m_nick,
                                    m_mail, m_age, m_gender, m_lang,
                                    m_city, m_state, m_country,
                                    m_company, m_depart, m_position,
                                    m_occupation, m_past, m_past_text,
                                    m_interests, m_interests_text,
                                    m_affilations, m_affilations_text, 0, NULL,
                                    m_keywords, m_bOnline);
        break;
    case None:
        m_id_icq = 0;
        break;
    }
}

const QString ICQSearch::extractUIN(const QString& str)
{
    if (str.isEmpty())
        return QString::null; 
    QString s = str;
    return s.remove(' ').remove('-');
}

void ICQSearch::search()
{
    m_id_icq = 0;
    m_id_aim = 0;
    m_uins.clear();
    m_bAdd = false;
    if (!m_client->m_bAIM && m_bAdv){
        m_type = Full;
        setAdv(false);
        AdvSearch *adv = static_cast<AdvSearch*>(m_adv);
        m_first		= edtFirst->text();
        m_last		= edtLast->text();
        m_nick		= edtNick->text();
        m_mail		= edtMail->text();
        m_age		= getComboValue(adv->cmbAge, p_ages);
        m_gender	= getComboValue(adv->cmbGender, p_genders);
        m_lang		= getComboValue(adv->cmbLang, p_languages);
        m_city		= adv->edtCity->text();
        m_state		= adv->edtState->text();
        m_country	= getComboValue(adv->cmbCountry, getCountries(), getCountryCodes());
        m_company	= adv->edtCompany->text();
        m_depart	= adv->edtDepartment->text();
        m_position	= adv->edtPosition->text();
        m_occupation= getComboValue(adv->cmbOccupation, p_occupations);
        m_past		= getComboValue(adv->cmbPast, p_pasts);
        m_past_text	= adv->edtPast->text();
        m_interests	= getComboValue(adv->cmbInterests, p_interests);
        m_interests_text = adv->edtInterests->text();
        m_affilations	 = getComboValue(adv->cmbAffilation, p_affilations);
        m_affilations_text = adv->edtAffilation->text();
        m_keywords	= adv->edtKeywords->text();
        m_bOnline	= adv->chkOnline->isChecked();
        icq_search();
    }else if (m_client->m_bAIM && m_bAdv){
        setAdv(false);
        AIMSearch *adv = static_cast<AIMSearch*>(m_adv);
        const char *country = NULL;
        int nCountry = getComboValue(adv->cmbCountry, getCountries(), getCountryCodes());
        for (const ext_info *info = getCountryCodes(); info->szName; ++info){
            if (info->nCode == nCountry){
                country = info->szName;
                break;
            }
        }
        m_id_aim = m_client->aimInfoSearch(
                       adv->edtFirst->text(),
                       adv->edtLast->text(),
                       adv->edtMiddle->text(),
                       adv->edtMaiden->text(),
                       country,
                       adv->edtStreet->text(),
                       adv->edtCity->text(),
                       adv->edtNick->text(),
                       adv->edtZip->text(),
                       adv->edtState->text());
    }
	else if (!m_client->m_bAIM && grpUin->isChecked() && !edtUIN->text().isEmpty())
	{
        m_type = UIN;
        m_uin  = extractUIN(edtUIN->text()).toULong();
        icq_search();
    }
	else if (grpMail->isChecked() && !edtMail->text().isEmpty())
	{
        if (!m_client->m_bAIM)
		{
            m_type = Mail;
            m_mail = edtMail->text();
            icq_search();
        }
        m_id_aim = m_client->aimEMailSearch(edtMail->text());
    }
	else if (!m_client->m_bAIM && grpName->isChecked() &&
              (!edtFirst->text().isEmpty() || !edtLast->text().isEmpty() || !edtNick->text().isEmpty())){
        m_type = Name;
        m_first		= edtFirst->text();
        m_last		= edtLast->text();
        m_nick		= edtNick->text();
        icq_search();
        m_id_aim = m_client->aimInfoSearch(edtFirst->text(), edtLast->text(), QString::null, QString::null,
                                           QString::null, QString::null, QString::null, edtNick->text(), QString::null, QString::null);
    }
    if ((m_id_icq == 0) && (m_id_aim == 0))
        return;
    addColumns();
}

void ICQSearch::addColumns()
{
    QStringList columns;
    columns.append(QString::null);
    columns.append(QString::null);
    columns.append("nick");
    columns.append(i18n("Nick"));
    columns.append("first");
    columns.append(i18n("First Name"));
    columns.append("last");
    columns.append(i18n("Last Name"));
    if (m_client->m_bAIM){
        columns.append("city");
        columns.append(i18n("City"));
        columns.append("state");
        columns.append(i18n("State"));
        columns.append("country");
        columns.append(i18n("Country"));
    }else{
        columns.append("gender");
        columns.append(i18n("Gender"));
        columns.append("age");
        columns.append(i18n("Age"));
        columns.append("email");
        columns.append(i18n("E-Mail"));
    }
    emit setColumns(columns, 6, this);
}

void ICQSearch::searchMail(const QString &mail)
{
    if (!m_client->m_bAIM){
        m_type = Mail;
        m_mail = mail;
        icq_search();
    }
    m_id_aim = m_client->aimEMailSearch(mail);
    addColumns();
}

void ICQSearch::searchName(const QString &first, const QString &last, const QString &nick)
{
    if (!m_client->m_bAIM){
        m_type		= Name;
        m_first		= first;
        m_last		= last;
        m_nick		= nick;
        icq_search();
    }
    m_id_aim = m_client->aimInfoSearch(first, last, QString::null, QString::null, QString::null,
                                       QString::null, QString::null, nick, QString::null, QString::null);
    addColumns();
}

void ICQSearch::searchStop()
{
    m_id_icq = 0;
    m_id_aim = 0;
}

bool ICQSearch::processEvent(Event *e)
{
    if ((e->type() == eEventICQSearch) || (e->type() == eEventICQSearchDone)){
        EventSearchInternal *es = static_cast<EventSearchInternal*>(e);
        SearchResult *res = es->searchResult();
        if ((res->id != m_id_aim) && (res->id != m_id_icq) && (res->client != m_client))
            return false;
        if (e->type() == eEventICQSearchDone){
            if (res->id == m_id_icq){
                m_id_icq = 0;
                if (res->data.getUin() && m_bAdd)
                    icq_search();
            }
            if (res->id == m_id_aim)
                m_id_aim = 0;
            if ((m_id_icq == 0) && (m_id_aim == 0))
                emit searchDone(this);
            return false;
        }
        QString icon;
        if (res->data.getUin()){
            icon = "ICQ_";
            switch (res->data.getStatus()){
            case STATUS_ONLINE:
                icon += "online";
                break;
            case STATUS_OFFLINE:
                icon += "offline";
                break;
            default:
                icon += "inactive";
            }
            if (m_uins.indexOf (res->data.getUin()) != -1)
                return false;
            m_bAdd = true;
            m_uins.push_back(res->data.getUin());
        }else{
            icon = "AIM";
        }
        QString gender;
        switch (res->data.getGender()){
        case 1:
            gender = i18n("Female");
            break;
        case 2:
            gender = i18n("Male");
            break;
        }
        QString age;
        if (res->data.getAge())
            age = QString::number(res->data.getAge());
        QStringList l;
        l.append(icon);
        QString key = m_client->screen(&res->data);
        if (res->data.getUin()){
            while (key.length() < 13)
                key = '.' + key;
        }
        l.append(key);
        l.append(m_client->screen(&res->data));;
        if (m_client->m_bAIM){
            QString s;
            l.append(res->data.getNick());
            l.append(res->data.getFirstName());
            l.append(res->data.getLastName());
            l.append(res->data.getCity());
            l.append(res->data.getState());
            if (res->data.getCountry()){
                for (const ext_info *info = getCountries(); info->szName; info++){
                    if (info->nCode == res->data.getCountry()){
                        s = i18n(info->szName);
                        break;
                    }
                }
            }
            l.append(s);
        }else{
            l.append(res->data.getNick());
            l.append(res->data.getFirstName());
            l.append(res->data.getLastName());
            l.append(gender);
            l.append(age);
            l.append(res->data.getEmail());
        }
        emit addItem(l, this);
    }
    return false;
}

void ICQSearch::createContact(const QString &name, unsigned tmpFlags, Contact *&contact)
{
    if (m_client->findContact(name, NULL, false, contact))
        return;
    if (m_client->findContact(name, &name, true, contact, NULL, false) == NULL)
        return;
    contact->setFlags(contact->getFlags() | tmpFlags);
}


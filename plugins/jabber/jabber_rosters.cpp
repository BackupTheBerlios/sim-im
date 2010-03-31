/***************************************************************************
                          jabber_rosters.cpp  -  description
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

#include <QImage>
#include <QFile>
#include <QByteArray>
#include <QDateTime>

#include "html.h"
#include "log.h"
#include "unquot.h"
#include "misc.h"
#include "core_events.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/client.h"

#include "jabberclient.h"
#include "jabber.h"
#include "jabbermessage.h"

using namespace std;
using namespace SIM;

class RostersRequest : public JabberClient::ServerRequest
{
public:
    RostersRequest(JabberClient *client);
    ~RostersRequest();
protected:
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
    virtual void char_data(const QString& str);
    QString     m_jid;
    QString     m_name;
    QString     m_grp;
    QString     m_subscription;
    unsigned    m_subscribe;
    unsigned    m_bSubscription;
    QString    *m_data;
};

RostersRequest::RostersRequest(JabberClient *client)
        : JabberClient::ServerRequest(client, _GET, NULL, NULL)
{
    m_data	= NULL;
    ContactList::ContactIterator itc;
    Contact *contact;
    while ((contact = ++itc) != NULL){
        ClientDataIterator it = contact->clientDataIterator(client);
        JabberUserData *data;
        while ((data = m_client->toJabberUserData(++it)) != NULL)
            data->setChecked(false);
    }
    client->m_bJoin = false;
}

RostersRequest::~RostersRequest()
{
    ContactList::ContactIterator itc;
    Contact *contact;
    list<Contact*> contactRemoved;
    while ((contact = ++itc) != NULL){
        ClientDataIterator it = contact->clientDataIterator(m_client);
        JabberUserData *data;
        list<SIM::IMContact*> dataRemoved;
        while ((data = m_client->toJabberUserData(++it)) != NULL){
            if (!data->isChecked()){
                QString jid = data->getId();
                JabberListRequest *lr = m_client->findRequest(jid, false);
                if (lr && lr->bDelete)
                    m_client->findRequest(jid, true);
                dataRemoved.push_back(data);
            }
        }
        if (dataRemoved.empty())
            continue;
        for (list<SIM::IMContact*>::iterator itr = dataRemoved.begin(); itr != dataRemoved.end(); ++itr)
            contact->freeData(*itr);
        if (contact->size() == 0)
            contactRemoved.push_back(contact);
    }
    for (list<Contact*>::iterator itr = contactRemoved.begin(); itr != contactRemoved.end(); ++itr)
        delete *itr;
    m_client->processList();
    if (m_client->m_bJoin){
        EventJoinAlert(m_client).process();
    }
}

void RostersRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "item"){
        m_subscribe = SUBSCRIBE_NONE;
        m_grp = QString::null;
        m_jid = attrs.value("jid");
        if (m_jid.length() == 0)
            return;
        m_name = attrs.value("name");
        m_subscription  = QString::null;
        m_bSubscription = false;
        QString subscribe = attrs.value("subscription");
        if (subscribe == "none"){
            m_subscribe = SUBSCRIBE_NONE;
        }else if (subscribe == "from"){
            m_subscribe = SUBSCRIBE_FROM;
        }else if (subscribe == "to"){
            m_subscribe = SUBSCRIBE_TO;
        }else if (subscribe == "both"){
            m_subscribe = SUBSCRIBE_BOTH;
        }else{
            log(L_WARN, "Unknown attr subscribe=%s", qPrintable(subscribe));
        }
        return;
    }
    if (el == "group"){
        m_grp = QString::null;
        m_data = &m_grp;
        return;
    }
    if (el == "subscription"){
        m_bSubscription = true;
        m_subscription = QString::null;
        m_data = &m_subscription;
        return;
    }
}

void RostersRequest::element_end(const QString& el)
{
    if (el == "group"){
        m_data = NULL;
        return;
    }
    if (el == "item"){
        if ( m_jid.indexOf('/') >= 0 )
	{
	   // If roster contains two contacts
	   // user@jabber.ru and user@jabber.ru/some_resource
	   // than it will be impossible, to move user@jabber.ru from one group to another because
	   // after reloading sim it will be merged with user@jabber.ru/some_resource
	   // because sim-im see no difference between them :-/
	   // So the simplest way to fix this bus is to ignore roster records with explicit resource
	   // This is a nasty hack, but I see no way to fix it without rewriting all jabber stuff :-(
	   // N. Shaplov
           log(L_DEBUG,"Ignoring contact with explicit resource: %s", qPrintable(m_jid));
	   return;
	}    
        bool bChanged = false;
        JabberListRequest *lr = m_client->findRequest(m_jid, false);
        Contact *contact;
        QString resource;
        JabberUserData *data = m_client->findContact(m_jid, m_name, false, contact, resource);
        if (data == NULL){
            if (lr && lr->bDelete){
                m_client->findRequest(m_jid, true);
            }else{
                bChanged = true;
                data = m_client->findContact(m_jid, m_name, true, contact, resource);
                if (m_bSubscription){
                    contact->setFlags(CONTACT_TEMP);
                    EventContact eContact(contact, EventContact::eChanged);
                    eContact.process();
                    m_client->auth_request(m_jid, MessageAuthRequest, m_subscription, true);
                    data = m_client->findContact(m_jid, m_name, false, contact, resource);
                }
            }
        }
        if (data == NULL)
            return;
        if (data->getSubscribe() != m_subscribe){
            bChanged = true;
            data->setSubscribe(m_subscribe);
        }
        data->setGroup(m_grp);
        data->setChecked(true);
        if (lr == NULL){
            unsigned grp = 0;
            if (!m_grp.isEmpty()){
                Group *group = NULL;
                ContactList::GroupIterator it;
                while ((group = ++it) != NULL){
                    if (m_grp == group->getName()){
                        grp = group->id();
                        break;
                    }
                }
                if (group == NULL){
                    group = getContacts()->group(0, true);
                    group->setName(m_grp);
                    grp = group->id();
                    EventGroup e(group, EventGroup::eChanged);
                    e.process();
                }
            }
            if (contact->getGroup() != (int)grp){
                if (grp == 0){
                    void *d = NULL;
                    ClientDataIterator it_d = contact->clientDataIterator();
                    while ((d = ++it_d) != NULL){
                        if (d != data)
                            break;
                    }
                    if (d){
                        grp = contact->getGroup();
                        Group *group = getContacts()->group(grp);
                        if (group)
                            m_client->listRequest(data, contact->getName(), group->getName(), false);
                    }
                }
                contact->setGroup(grp);
                bChanged = true;
            }
        }
        if (bChanged){
            EventContact e(contact, EventContact::eChanged);
            e.process();
        }
    }
}

void RostersRequest::char_data(const QString& str)
{
    if (m_data != NULL)
        *m_data += str;
}

void JabberClient::rosters_request()
{
    RostersRequest *req = new RostersRequest(this);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:roster");
    req->send();
    m_requests.push_back(req);
}

class InfoRequest : public JabberClient::ServerRequest
{
public:
    InfoRequest(JabberClient *client, JabberUserData *data, bool bVCard);
    ~InfoRequest();
protected:
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
    virtual void char_data(const QString& str);
    QString	m_jid;
    QString  m_node;
    QString	m_host;
    bool	m_bStarted;
    QString  m_firstName;
    QString	m_nick;
    QString	m_desc;
    QString	m_email;
    QString	m_bday;
    QString	m_url;
    QString	m_orgName;
    QString	m_orgUnit;
    QString	m_title;
    QString	m_role;
    QString	m_phone;
    QString	m_street;
    QString  m_ext;
    QString	m_city;
    QString	m_region;
    QString	m_pcode;
    QString	m_country;
    QString	m_photo;    // a bas64 encoded string
    QString	m_logo;     // a bas64 encoded string
    bool	m_bPhoto;
    bool	m_bLogo;
    bool	m_bVCard;

    QString	*m_data;
};

extern DataDef jabberUserData[];

InfoRequest::InfoRequest(JabberClient *client, JabberUserData *data, bool bVCard)
        : JabberClient::ServerRequest(client, _GET, NULL, client->buildId(data))
{
    m_jid   = data->getId();
    m_node  = data->getNode();
    m_bStarted = false;
    m_data  = NULL;
    m_bPhoto = false;
    m_bLogo  = false;
    m_bVCard = bVCard;
}

InfoRequest::~InfoRequest()
{
    if (m_bStarted){
        Contact *contact = NULL;
        JabberUserData *data;
        JabberUserData u_data(SIM::ClientPtr(0));
        if (m_bVCard){
            //load_data(jabberUserData, &u_data, NULL);
            data = &u_data;
            data->setId(m_jid);
            data->setNode(m_node);
        }else{
            if (m_jid == m_client->data.owner.getId()){
                data = &m_client->data.owner;
            }else{
                QString jid = m_jid;
                if (jid.indexOf('@') == -1){
                    jid += '@';
                    jid += m_host;
                }
                QString resource;
                data = m_client->findContact(jid, QString::null, false, contact, resource);
                if (data == NULL)
                    return;
            }
        }
        bool bChanged = false;
        if(data->getFirstName() != m_firstName) {
            bChanged = true;
            data->setFirstName(m_firstName);
        }
        if(data->getNick() != m_nick) {
            bChanged = true;
            data->setNick(m_nick);
        }
        if(data->getDesc() != m_desc) {
            bChanged = true;
            data->setDesc(m_desc);
        }
        if(data->getBirthday() != m_bday) {
            bChanged = true;
            data->setBirthday(m_bday);
        }
        if(data->getUrl() != m_url) {
            bChanged = true;
            data->setUrl(m_url);
        }
        if(data->getOrgName() != m_orgName) {
            bChanged = true;
            data->setOrgName(m_orgName);
        }
        if(data->getOrgUnit() != m_orgUnit) {
            bChanged = true;
            data->setOrgUnit(m_orgUnit);
        }
        if(data->getTitle() != m_title) {
            bChanged = true;
            data->setTitle(m_title);
        }
        if(data->getRole() != m_role) {
            bChanged = true;
            data->setRole(m_role);
        }
        if(data->getStreet() != m_street) {
            bChanged = true;
            data->setStreet(m_street);
        }
        if(data->getExtAddr() != m_ext) {
            bChanged = true;
            data->setExtAddr(m_ext);
        }
        if(data->getCity() != m_city) {
            bChanged = true;
            data->setCity(m_city);
        }
        if(data->getRegion() != m_region) {
            bChanged = true;
            data->setRegion(m_region);
        }
        if(data->getPCode() != m_pcode) {
            bChanged = true;
            data->setPCode(m_pcode);
        }
        if(data->getCountry() != m_country) {
            bChanged = true;
            data->setCountry(m_country);
        }
        if(data->getEmail() != m_email) {
            bChanged = true;
            data->setEmail(m_email);
        }
        if(data->getPhone() != m_phone) {
            bChanged = true;
            data->setPhone(m_phone);
        }

        if (m_bVCard){
            EventVCard(data).process();
            //free_data(jabberUserData, &u_data);
            return;
        }
        QImage photo;
        if (m_photo.length()){
            QString fName = m_client->photoFile(data);
            QFile f(fName);
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                QByteArray cstr = m_photo.toAscii();   // ok, base64 encoded
                f.write(QByteArray::fromBase64(cstr));
                f.close();
                photo.load(fName);
            }else{
                log(L_ERROR, "Can't create %s", qPrintable(fName));
            }
        }
        if (photo.width() && photo.height()){
            if ((photo.width() != (int)(data->getPhotoWidth())) ||
                    (photo.height() != (int)(data->getPhotoHeight())))
                bChanged = true;
            data->setPhotoWidth(photo.width());
            data->setPhotoHeight(photo.height());
            if (m_jid == m_client->data.owner.getId())
                m_client->setPhoto(m_client->photoFile(data));
        }else{
            if (data->getPhotoWidth() || data->getPhotoHeight())
                bChanged = true;
            data->setPhotoWidth(0);
            data->setPhotoHeight(0);
        }

        QImage logo;
        if (m_logo.length()){
            QString fName = m_client->logoFile(data);
            QFile f(fName);
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)){
                QByteArray cstr = m_logo.toAscii();   // ok, base64 encoded
                f.write(QByteArray::fromBase64(cstr));
                f.close();
                logo.load(fName);
            }else{
                log(L_ERROR, "Can't create %s", qPrintable(fName));
            }
        }
        if (logo.width() && logo.height()){
            if ((logo.width() != (int)(data->getLogoWidth())) ||
                    (logo.height() != (int)(data->getLogoHeight())))
                bChanged = true;
            data->setLogoWidth(logo.width());
            data->setLogoHeight(logo.height());
            if (m_jid == m_client->data.owner.getId())
                m_client->setLogo(m_client->logoFile(data));
        }else{
            if (data->getLogoWidth() || data->getLogoHeight())
                bChanged = true;
            data->setLogoWidth(0);
            data->setLogoHeight(0);
        }

        if (bChanged){
            if (contact){
                m_client->setupContact(contact, data);
                EventContact(contact, EventContact::eChanged).process();
            }else{
                EventClientChanged(m_client).process();
            }
        }
    }
}

void InfoRequest::element_start(const QString& el, const QXmlAttributes&)
{
    m_data = NULL;
    if (el == "vcard"){
        m_bStarted = true;
        return;
    }
    if (el == "nickname"){
        m_data = &m_nick;
        return;
    }
    if (el == "fn"){
        m_data = &m_firstName;
        return;
    }
    if (el == "desc"){
        m_data = &m_desc;
        return;
    }
    if (el == "email"){
        m_data = &m_email;
        return;
    }
    if (el == "bday"){
        m_data = &m_bday;
        return;
    }
    if (el == "url"){
        m_data = &m_url;
        return;
    }
    if (el == "orgname"){
        m_data = &m_orgName;
        return;
    }
    if (el == "orgunit"){
        m_data = &m_orgUnit;
        return;
    }
    if (el == "title"){
        m_data = &m_title;
        return;
    }
    if (el == "role"){
        m_data = &m_role;
        return;
    }
    if (el == "voice"){
        m_data = &m_phone;
        return;
    }
    if (el == "street"){
        m_data = &m_street;
        return;
    }
    if (el == "extadd"){
        m_data = &m_ext;
        return;
    }
    if (el == "locality"){
        m_data = &m_city;
        return;
    }
    if (el == "region"){
        m_data = &m_region;
        return;
    }
    if (el == "pcode"){
        m_data = &m_pcode;
        return;
    }
    if (el == "country"){
        m_data = &m_country;
        return;
    }
    if (el == "photo"){
        m_bPhoto = true;
        return;
    }
    if (el == "logo"){
        m_bLogo = true;
        return;
    }
    if (el == "binval"){
        if (m_bPhoto)
            m_data = &m_photo;
        if (m_bLogo)
            m_data = &m_logo;
    }
}

void InfoRequest::element_end(const QString& el)
{
    m_data  = NULL;
    if (el == "photo"){
        m_bPhoto = false;
        return;
    }
    if (el == "logo"){
        m_bLogo = false;
        return;
    }
	if(el == "vcard")
	{
		EventClientChanged(m_client).process();
	}
}

void InfoRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

void JabberClient::info_request(JabberUserData *user_data, bool bVCard)
{
    if (getState() != Connected)
        return;
    if (user_data == NULL)
        user_data = &data.owner;
    InfoRequest *req = new InfoRequest(this, user_data, bVCard);
    req->start_element("vCard");
    req->add_attribute("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
    req->add_attribute("xmlns", "vcard-temp");
    req->add_attribute("version", "2.0");
    if (!user_data->getNode().isEmpty())
        req->add_attribute("node", user_data->getNode());
    req->send();
    m_requests.push_back(req);
}

class SetInfoRequest : public JabberClient::ServerRequest
{
public:
    SetInfoRequest(JabberClient *client, JabberUserData *data);
protected:
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
};

SetInfoRequest::SetInfoRequest(JabberClient *client, JabberUserData *data)
        : JabberClient::ServerRequest(client, _SET, NULL, client->buildId(data))
{
}

void SetInfoRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "iq"){
        QString type = attrs.value("type");
        if (type == "result")
            m_client->setInfoUpdated(false);
    }
}

void JabberClient::setClientInfo(IMContact* _data)
{
    if(_data->getSign() != JABBER_SIGN)
        return;
    JabberUserData *data = static_cast<JabberUserData*>(_data);  // FIXME unsafe type conversion
    if (data != &this->data.owner) {
        this->data.owner.setFirstName(data->getFirstName());
        this->data.owner.setNick(data->getNick());
        this->data.owner.setDesc(data->getDesc());
        this->data.owner.setBirthday(data->getBirthday());
        this->data.owner.setUrl(data->getUrl());
        this->data.owner.setOrgName(data->getOrgName());
        this->data.owner.setOrgUnit(data->getOrgUnit());
        this->data.owner.setTitle(data->getTitle());
        this->data.owner.setRole(data->getRole());
        this->data.owner.setStreet(data->getStreet());
        this->data.owner.setExtAddr(data->getExtAddr());
        this->data.owner.setCity(data->getCity());
        this->data.owner.setRegion(data->getRegion());
        this->data.owner.setPCode(data->getPCode());
        this->data.owner.setCountry(data->getCountry());
    }
    setInfoUpdated(true);
    if (getState() != Connected)
        return;
    SetInfoRequest *req = new SetInfoRequest(this, &this->data.owner);
    req->start_element("vCard");
    req->add_attribute("prodid", "-//HandGen//NONSGML vGen v1.0//EN");
    req->add_attribute("xmlns", "vcard-temp");
    req->add_attribute("version", "2.0");
    req->add_attribute("node", data->getNode());
    req->text_tag("FN", data->getFirstName());
    req->text_tag("NICKNAME", data->getNick());
    req->text_tag("DESC", data->getDesc());
    QString mails = getContacts()->owner()->getEMails();
    while (mails.length()){
        QString mailItem = getToken(mails, ';', false);
        QString mail = getToken(mailItem, '/');
        if (mailItem.length())
            continue;
        req->text_tag("EMAIL", mail);
        break;
    }
    req->text_tag("BDAY", data->getBirthday());
    req->text_tag("URL", data->getUrl());
    req->start_element("ORG");
    req->text_tag("ORGNAME", data->getOrgName());
    req->text_tag("ORGUNIT", data->getOrgUnit());
    req->end_element();
    req->text_tag("TITLE", data->getTitle());
    req->text_tag("ROLE", data->getRole());
    QString phone;
    QString phones = getContacts()->owner()->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        QString phoneValue = getToken(phoneItem, '/', false);
        if (phoneItem.length())
            continue;
        QString number = getToken(phoneValue, ',');
        QString type = getToken(phoneValue, ',');
        if (type == "Hone Phone"){
            phone = number;
            break;
        }
    }
    if (phone.length()){
        req->start_element("TEL");
        req->start_element("HOME");
        req->end_element();
        req->text_tag("VOICE", phone);
        req->end_element();
    }
    req->start_element("ADDR");
    req->start_element("HOME");
    req->end_element();
    req->text_tag("STREET", data->getStreet());
    req->text_tag("EXTADD", data->getExtAddr());
    req->text_tag("LOCALITY", data->getCity());
    req->text_tag("REGION", data->getRegion());
    req->text_tag("PCODE", data->getPCode());
    req->text_tag("COUNTRY", data->getCountry());
    req->end_element();
    if (!getPhoto().isEmpty()){
        QFile img(getPhoto());
        if (img.open(QIODevice::ReadOnly)){
            Buffer b;
            b.init(img.size());
            img.read(b.data(), b.size());
            QByteArray packed = b.toBase64();
            req->start_element("PHOTO");
            req->text_tag("BINVAL", packed);
            req->end_element();
        }
    }
    if (!getLogo().isEmpty()){
        QFile img(getLogo());
        if (img.open(QIODevice::ReadOnly)){
            Buffer b;
            b.init(img.size());
            img.read(b.data(), b.size());
            QByteArray packed = b.toBase64();
            req->start_element("LOGO");
            req->text_tag("BINVAL", packed.data());
            req->end_element();
        }
    }
    req->send();
    m_requests.push_back(req);
}

class AddRequest : public JabberClient::ServerRequest
{
public:
    AddRequest(JabberClient *client, const QString &jid, unsigned grp);
protected:
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    QString m_jid;
    unsigned m_grp;
};

AddRequest::AddRequest(JabberClient *client, const QString &jid, unsigned grp)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
{
    m_jid = jid;
    m_grp = grp;
}

void AddRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "iq"){
        QString type = attrs.value("type");
        if (type == "result"){
            Contact *contact;
            QString resource;
            JabberUserData *data = m_client->findContact(m_jid, QString::null, true, contact, resource);
            if (data && (contact->getGroup() != (int)m_grp)){
                contact->setGroup(m_grp);
                EventContact e(contact, EventContact::eChanged);
                e.process();
            }
        }
    }
}

bool JabberClient::add_contact(const char *_jid, unsigned grp)
{
    Contact *contact;
    QString resource;
    QString jid = QString::fromUtf8(_jid);
    if (findContact(jid, QString::null, false, contact, resource)){
        EventContact e(contact, EventContact::eChanged);
        e.process();
        return false;
    }
    AddRequest *req = new AddRequest(this, jid, grp);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:roster");
    req->start_element("item");
    req->add_attribute("jid", jid);
    Group *g = NULL;
    if (grp)
        g = getContacts()->group(grp);
    if (g)
        req->text_tag("group", g->getName());
    req->send();
    m_requests.push_back(req);
    return true;
}

JabberClient::PresenceRequest::PresenceRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
}

//static unsigned get_number(QString &s, unsigned digits)
//{
//    if ((unsigned)s.length() < digits){
//        s = QString::null;
//        return 0;
//    }
//    QString p = s.left(digits);
//    s = s.mid(digits);
//    return p.toUInt();
//}

static QDateTime fromDelay(const QString &t)
{
    QDateTime dt(QDateTime::currentDateTime());
    QRegExp reg("^(\\d{4})-?(\\d{2})-?(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})(.(\\d{3}))?((Z)|([-+]\\d{2}:\\d{2}))?$");
    do {
        if( reg.indexIn(t) == -1 )
            break;
        int numcap = reg.numCaptures();
        if( numcap < 6 )
            break;
        QDate date;
        date.setYMD(
            reg.cap(1).toUInt(),
            reg.cap(2).toUInt(),
            reg.cap(3).toUInt()
        );
        QTime time;
        time.setHMS(
            reg.cap(4).toUInt(),
            reg.cap(5).toUInt(),
            reg.cap(6).toUInt()
        );
        dt.setDate( date );
        dt.setTime( time );
        if( numcap < 9 )
            break;
        QString sTZD = reg.cap(9);
        if( sTZD == "Z" ) {
            dt.setTimeSpec( Qt::UTC );
        }
        else {
            dt.setTimeSpec( Qt::OffsetFromUTC );
            int secs = QTime::fromString(sTZD.right(5),"hh:mm").secsTo(QTime(0,0,0,0));
            if( sTZD[0] == '+' )
                secs = -secs;
            dt.setUtcOffset( secs );
            dt.toLocalTime();
        }
    } while( false );

    return dt;
}

JabberClient::PresenceRequest::~PresenceRequest()
{
    unsigned status = STATUS_UNKNOWN;
    bool bInvisible = false;
    // RFC 3921 "XMPP IM": 2.2.1. Types of Presence
    if (m_type == "unavailable"){
        status = STATUS_OFFLINE;
    }else if (m_type == "subscribe"){
        m_client->auth_request(m_from, MessageAuthRequest, m_status, true);
    }else if (m_type == "subscribed"){
        m_client->auth_request(m_from, MessageAuthGranted, m_status, true);
    }else if (m_type == "unsubscribe"){
        m_client->auth_request(m_from, MessageRemoved, m_status, true);
    }else if (m_type == "unsubscribed"){
        m_client->auth_request(m_from, MessageAuthRefused, m_status, true);
    }else if (m_type == "probe"){
        // server want to to know if we're living
        m_client->ping();
    }else if (m_type == "error"){
        log(L_DEBUG, "An error has occurred regarding processing or delivery of a previously-sent presence stanza");
    }else if (m_type.length() == 0){
        // RFC 3921 "XMPP IM": 2.2.2.1. Show
        status = STATUS_ONLINE;
        if (m_show == "away"){
            status = STATUS_AWAY;
        }else if (m_show == "chat"){
            status = STATUS_FFC;
        }else if (m_show == "xa"){
            status = STATUS_NA;
        }else if (m_show == "dnd"){
            status = STATUS_DND;
		}else if (m_show == "occupied"){
            status = STATUS_OCCUPIED;
        }else if (m_show == "online"){
            status = STATUS_ONLINE;
        }else if (m_show.isEmpty()){
            // RFC 3921 "XMPP IM": 2.2.2.2. Status
            status = STATUS_ONLINE;
            if (m_status == "Online"){
                status = STATUS_ONLINE;
            }else if (m_status == "Disconnected"){
                status = STATUS_OFFLINE;
            }else if (m_status == "Connected"){
                status = STATUS_ONLINE;
            }else if (m_status == "Invisible"){
                status = STATUS_ONLINE;
                bInvisible = true;
            }else if (!m_status.isEmpty()){
                status = STATUS_ONLINE;
            }
        }else{
            log(L_DEBUG, "Unsupported available status %s", qPrintable(m_show));
        }
    }else{
        log(L_DEBUG, "Unsupported presence type %s", qPrintable(m_type));
    }
    QDateTime time1(QDateTime::currentDateTime());
    QDateTime time2;
    if (!m_stamp1.isEmpty())
        time1 = fromDelay(m_stamp1);
    if (!m_stamp2.isEmpty()){
        time2 = fromDelay(m_stamp2);
        if (time2 > time1){
            QDateTime t = time1;
            time1 = time2;
            time2 = t;
        }
    }

    if (status != STATUS_UNKNOWN){
        Contact *contact;
        QString resource;
        JabberUserData *data = m_client->findContact(m_from, QString::null, false, contact, resource);
        if (data){
            unsigned i;
            for (i = 1; i <= data->getNResources(); i++){
                if (resource == data->getResource(i))
                    break;
            }
            bool bChanged = false;
            if (status == STATUS_OFFLINE){
                if (i <= data->getNResources()){
                    bChanged = true;
                    vector<QString> resources;
                    vector<QString> resourceReply;
                    vector<QString> resourceStatus;
                    vector<QString> resourceStatusTime;
                    vector<QString> resourceOnlineTime;
                    vector<QString> resourceClientName;
                    vector<QString> resourceClientVersion;
                    vector<QString> resourceClientOS;
                    for (unsigned n = 1; n <= data->getNResources(); n++){
                        if (i == n)
                            continue;
                        resources.push_back(data->getResource(n));
                        resourceReply.push_back(data->getResourceReply(n));
                        resourceStatus.push_back(data->getResourceStatus(n));
                        resourceStatusTime.push_back(data->getResourceStatusTime(n));
                        resourceOnlineTime.push_back(data->getResourceOnlineTime(n));
                        resourceClientName.push_back(data->getResourceClientName(n));
                        resourceClientVersion.push_back(data->getResourceClientVersion(n));
                        resourceClientOS.push_back(data->getResourceClientOS(n));
                    }
                    data->clearResources();
                    data->clearResourceReplies();
                    data->clearResourceStatuses();
                    data->clearResourceStatusTimes();
                    data->clearResourceOnlineTimes();
                    data->clearResourceClientNames();
                    data->clearResourceClientVersions();
                    data->clearResourceClientOSes();
                    for (i = 0; i < resources.size(); i++){
                        data->setResource(i + 1, resources[i]);
                        data->setResourceReply(i + 1, resourceReply[i]);
                        data->setResourceStatus(i + 1, resourceStatus[i]);
                        data->setResourceStatusTime(i + 1, resourceStatusTime[i]);
                        data->setResourceOnlineTime(i + 1, resourceOnlineTime[i]);
                        data->setResourceClientName(i + 1, resourceClientName[i]);
                        data->setResourceClientVersion(i + 1, resourceClientVersion[i]);
                        data->setResourceClientOS(i + 1, resourceClientOS[i]);
                    }
                    data->setNResources(resources.size());
                }
                if (data->getNResources() == 0)
                    data->setAutoReply(m_status);
            }else{
                if (i > data->getNResources()){
                    bChanged = true;
                    data->setNResources(i);
                    data->setResource(i, resource);
                    data->setResourceOnlineTime(i, QString::number(!time2.isNull() ? time2.toTime_t() : time1.toTime_t()));
                    if (m_client->getUseVersion())
                        m_client->versionInfo(m_from);
                }
                if (QString::number(status) != data->getResourceStatus(i)){
                    bChanged = true;
                    data->setResourceStatus(i, QString::number(status));
                    data->setResourceStatusTime(i, QString::number(time1.toTime_t()));
                }
                if (m_status != data->getResourceReply(i)){
                    bChanged = true;
                    data->setResourceReply(i, m_status);
                }
            }
            bool bOnLine = false;
            status = STATUS_OFFLINE;
            for (i = 1; i <= data->getNResources(); i++){
                unsigned rStatus = data->getResourceStatus(i).toUInt();
                if (rStatus > status)
                    status = rStatus;
            }
            if (data->getStatus() != status){
                bChanged = true;
                if ((status == STATUS_ONLINE) &&
                        (((int)(time1.toTime_t() - m_client->data.owner.getOnlineTime()) > 60) ||
                         (data->getStatus() != STATUS_OFFLINE)))
                    bOnLine = true;
                if (data->getStatus() == STATUS_OFFLINE){
                    data->setOnlineTime(time1.toTime_t());
                    data->setRichText(true);
                }
                if (status == STATUS_OFFLINE && data->isTyping()){
                    data->setTyping(false);
                    EventContact e(contact, EventContact::eStatus);;
                    e.process();
                }
                data->setStatus(status);
                data->setStatusTime(time1.toTime_t());
            }
            if (data->isInvisible() != bInvisible){
                data->setInvisible(bInvisible);
                bChanged = true;
            }
            if (bChanged){
                StatusMessage *m = new StatusMessage();
                m->setContact(contact->id());
                m->setClient(m_client->dataName(data));
                m->setFlags(MESSAGE_RECEIVED);
                m->setStatus(status);
                EventMessageReceived e(m);
                if(!e.process())
                    delete m;
            }
            if (bOnLine && !contact->getIgnore() && !m_client->isAgent(data->getId())){
                EventContact e(contact, EventContact::eOnline);
                e.process();
            }
        }
    }
}

void JabberClient::PresenceRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "presence"){
        m_from = attrs.value("from");
        m_type = attrs.value("type");
    }
    // XEP-0203: Delayed Delivery
    if (el == "delay"){
        if (attrs.value("xmlns") == "http://www.xmpp.org/extensions/xep-0203.html#ns"){
            QString stamp = attrs.value("stamp");
            if (!stamp.isEmpty()){
                if (m_stamp1.isEmpty()){
                    m_stamp1 = stamp;
                }else if (m_stamp2.isEmpty()){
                    m_stamp2 = stamp;
                }
            }
        }
    } else
    if (el == "x"){
        // XEP-0091: Delayed Delivery
        if (attrs.value("xmlns") == "jabber:x:delay"){
            QString stamp = attrs.value("stamp");
            if (!stamp.isEmpty()){
                if (m_stamp1.isEmpty()){
                    m_stamp1 = stamp;
                }else if (m_stamp2.isEmpty()){
                    m_stamp2 = stamp;
                }
            }
        }
    }
    m_data = QString::null;
}

void JabberClient::PresenceRequest::element_end(const QString& el)
{
    if (el == "show"){
        m_show = m_data;
    }else if (el == "status"){
        m_status = m_data;
    }
}

void JabberClient::PresenceRequest::char_data(const QString& str)
{
    m_data += str;
}

JabberClient::IqRequest::IqRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
    m_data = NULL;
    m_file_size = 0;
}

JabberClient::IqRequest::~IqRequest()
{
    JabberFileMessage *msg = NULL;
    if (m_query == "jabber:iq:oob"){
        QString proto = m_url.left(7);
        if (proto != "http://"){
            log(L_WARN, "Unknown protocol");
            return;
        }
        m_url = m_url.mid(7);
        int n = m_url.indexOf(':');
        if (n < 0){
            log(L_WARN, "Port not found");
            return;
        }
        QString host = m_url.left(n);
        unsigned short port = (unsigned short)m_url.mid(n + 1).toLong();
        n = m_url.indexOf('/');
        if (n < 0){
            log(L_WARN, "File not found");
            return;
        }
        QString file = m_url.mid(n + 1);
        msg = new JabberFileMessage;
        msg->setDescription(file);
        msg->setText(m_descr);
        msg->setHost(host);
        msg->setPort(port);
    }else if (!m_file_name.isEmpty()){
        msg = new JabberFileMessage;
        msg->setDescription(m_file_name);
        msg->setSize(m_file_size);
    }
    if (msg){
        Contact *contact;
        QString resource;
        JabberUserData *data = m_client->findContact(m_from, QString::null, false, contact, resource);
        if (data == NULL){
            data = m_client->findContact(m_from, QString::null, true, contact, resource);
            if (data == NULL)
                return;
            contact->setFlags(CONTACT_TEMP);
        }
        msg->setFrom(m_from);
        msg->setID(m_id);
        msg->setFlags(MESSAGE_RECEIVED | MESSAGE_TEMP);
        msg->setClient(m_client->dataName(data));
        msg->setContact(contact->id());
        m_client->m_ackMsg.push_back(msg);
        EventMessageReceived e(msg);
        if (e.process()){
            for (list<Message*>::iterator it = m_client->m_ackMsg.begin(); it != m_client->m_ackMsg.end(); ++it){
                if ((*it) == msg){
                    m_client->m_ackMsg.erase(it);
                    break;
                }
            }
        }
    }
}

void JabberClient::IqRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "iq"){
        m_from = attrs.value("from");
        m_id   = attrs.value("id");
        m_type = attrs.value("type");
        return;
    }
    if (el == "query"){
        m_query = attrs.value("xmlns");
    }
    if ( (el == "item") && (m_query == "jabber:iq:roster") ){
                QString jid = attrs.value("jid");
                QString subscription = attrs.value("subscription");
                QString name = attrs.value("name");
                if (!subscription.isEmpty()){
                    unsigned subscribe = SUBSCRIBE_NONE;
                    if (subscription == "none"){
                        subscribe = SUBSCRIBE_NONE;
                    }else if (subscription == "to"){
                        subscribe = SUBSCRIBE_TO;
                    }else if (subscription == "from"){
                        subscribe = SUBSCRIBE_FROM;
                    }else if (subscription == "both"){
                        subscribe = SUBSCRIBE_BOTH;
                    }else if (subscription == "remove"){
                    }else{
                        log(L_DEBUG, "Unknown value subscription=%s", qPrintable(subscription));
                    }
                    Contact *contact;
                    QString resource;
                    JabberUserData *data = m_client->findContact(jid, name, false, contact, resource);
                    if ((data == NULL) && (subscribe != SUBSCRIBE_NONE)){
                        data = m_client->findContact(jid, name, true, contact, resource);
                    }
                    if (data && (data->getSubscribe() != subscribe)){
                        data->setSubscribe(subscribe);
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                        if (m_client->getAutoSubscribe() && ((subscribe & SUBSCRIBE_FROM) == 0)){
                            AuthMessage *msg = new AuthMessage(MessageAuthRequest);
                            msg->setContact(contact->id());
                            msg->setFlags(MESSAGE_NOHISTORY);
                            m_client->send(msg, data);
                        }
                    }
                }
    }
        // XEP-0092: Software Version
    if ( (el == "query") && (m_query == "jabber:iq:version") ){
            if (m_type == "get" && m_client->getUseVersion()){
                // send our version
                JabberClient::ServerRequest *req = new JabberClient::ServerRequest(m_client, JabberClient::ServerRequest::_RESULT, QString(), m_from, m_id);
                req->start_element("query");
                req->add_attribute("xmlns", "jabber:iq:version");
                req->text_tag("name", PACKAGE);
                req->text_tag("version", VERSION);
                QString version = get_os_version();
                req->text_tag("os", version);
                req->send();
                m_client->m_requests.push_back(req);
            }
    }
    if (el == "url")
        m_data = &m_url;
    if (el == "desc")
        m_data = &m_descr;
    if (el == "file"){
        m_file_name = attrs.value("name");
        m_file_size = attrs.value("size").toUInt();
    }
}

void JabberClient::IqRequest::element_end(const QString&)
{
    m_data = NULL;
}

void JabberClient::IqRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

JabberClient::StreamErrorRequest::StreamErrorRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
}

JabberClient::StreamErrorRequest::~StreamErrorRequest()
{
    m_client->socket()->error_state(m_descr);
}

void JabberClient::StreamErrorRequest::element_start(const QString& el, const QXmlAttributes&)
{
    if (el == "text"){
        m_data = &m_descr;
        return;
    }
}

void JabberClient::StreamErrorRequest::element_end(const QString&)
{
    m_data = NULL;
}

void JabberClient::StreamErrorRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

class JabberBgParser : public HTMLParser
{
public:
    JabberBgParser();
    QString parse(const QString &text);
    unsigned bgColor;
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &attrs);
    virtual void tag_end(const QString &tag);
    QString res;
};

JabberBgParser::JabberBgParser()
{
    bgColor = 0xFFFFFF;
}

QString JabberBgParser::parse(const QString &text)
{
    res = QString::null;
    HTMLParser::parse(text);
    return res;
}

void JabberBgParser::text(const QString &text)
{
    res += quoteString(text);
}

void JabberBgParser::tag_start(const QString &tag, const list<QString> &attrs)
{
    if (tag == "body"){
        for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name.toLower() == "bgcolor"){
                QColor c(value);
                bgColor = c.rgb();
            }
        }
        return;
    }
    res += '<';
    res += tag;
    for (list<QString>::const_iterator it = attrs.begin(); it != attrs.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        res += ' ';
        res += name;
        if (name == "style"){
            list<QString> styles = parseStyle(value);
            for (list<QString>::iterator it = styles.begin(); it != styles.end(); ++it){
                QString name = *it;
                ++it;
                QString value = *it;
                if (name == "background-color"){
                    QColor c;
                    c.setNamedColor(value);
                    bgColor = c.rgb() & 0xFFFFFF;
                }
            }
        }
        if (!value.isEmpty()){
            res += "=\'";
            res += quoteString(value);
            res += "\'";
        }
    }
    res += '>';
}

void JabberBgParser::tag_end(const QString &tag)
{
    if (tag == "body"){
        return;
    }
    res += "</";
    res += tag;
    res += '>';
}

JabberClient::MessageRequest::MessageRequest(JabberClient *client)
        : ServerRequest(client, NULL, NULL, NULL)
{
    m_data = NULL;
    m_errorCode = 0;
    m_bBody		= false;
    m_bCompose	= false;
    m_bEvent	= false;
    m_bRichText	= false;
	m_bEnc		= false;
    m_bRosters	= false;
    m_bError	= false;
}

JabberClient::MessageRequest::~MessageRequest()
{
    if (m_from.isEmpty())
        return;
    Contact *contact;
    QString resource;
    JabberUserData *data = m_client->findContact(m_from, QString::null, false, contact, resource);
    if (data == NULL){
        data = m_client->findContact(m_from, QString::null, true, contact, resource);
        if (data == NULL)
            return;
        contact->setFlags(CONTACT_TEMP);
    }
    Message *msg = NULL;

    if (!m_bError){
        // JEP-0022 composing event handling
        if (m_bBody){
            // Msg contains normal message.
            // <composing/> here means "send me composing events, please", so we should do it.
            // But if that tag is absent, we must not send them.
            data->setSendTypingEvents(m_bCompose);
            data->setTypingId(m_bCompose ? m_id : QString::null);

            // also, incoming message implicitly means that user has stopped typing
            if (data->isTyping()){
                data->setTyping(false);
                EventContact e(contact, EventContact::eStatus);;
                e.process();
            }
        }
        else{
            // Msg has no body ==> it is event message.
            // Presence of <composing/> here means "I'm typing", absence - "I'm not typing anymore".
            data->setTyping(m_bCompose);
            EventContact e(contact, EventContact::eStatus);;
            e.process();
        }
    }

    if (m_errorCode || !m_error.isEmpty()){
        if (!m_bEvent){
            JabberMessageError *m = new JabberMessageError;
            m->setError(m_error);
            m->setCode(m_errorCode);
            msg = m;
        }
    }else if (m_bBody){
        if (!m_contacts.isEmpty()){
            msg = new ContactsMessage;
            static_cast<ContactsMessage*>(msg)->setContacts(m_contacts);
        }else if (m_subj.isEmpty()){
            msg = new Message(MessageGeneric);
        }else{
            JabberMessage *m = new JabberMessage;
            m->setSubject(m_subj);
            msg = m;
        }
    }
    if (msg == NULL)
        return;
    if (m_bBody && m_contacts.isEmpty()){
        if (!m_enc.isEmpty()){
            data->setRichText(false);
			msg->setText(m_enc);
		}else if (m_richText.isEmpty()){
            data->setRichText(false);
            msg->setText(m_body);
        }else{
            JabberBgParser p;
            msg->setText(p.parse(m_richText));
            msg->setFlags(MESSAGE_RICHTEXT);
            msg->setBackground(p.bgColor);
        }
        if (m_targets.size()){
            if ((msg->getFlags() & MESSAGE_RICHTEXT) == 0){
                msg->setText(quoteString(msg->getText()));
                msg->setFlags(MESSAGE_RICHTEXT);
            }
            QString text = msg->getText();
            for (unsigned i = 0; i < m_targets.size(); i++){
                text += "<br><a href=\"";
                text += quoteString(m_targets[i]);
                text += "\">";
                text += quoteString(m_descs[i]);
                text += "</a>";
            }
        }
    }else{
        msg->setText(m_body);
    }
    msg->setFlags(msg->getFlags() | MESSAGE_RECEIVED);
    msg->setClient(m_client->dataName(data));
    msg->setContact(contact->id());
    EventMessageReceived e(msg);
    if (!e.process())
        delete msg;
}

void JabberClient::MessageRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (m_bRichText){
        *m_data += '<';
        *m_data += el;
        for (int i = 0; i < attrs.count(); i++){
            *m_data += ' ';
            *m_data += attrs.qName(i);
            QChar ch('\'');
            if(attrs.value(i).contains(QChar('\'')))
                ch = QChar('"');
            *m_data += "=";
            *m_data += ch;
            *m_data += attrs.value(i);
            *m_data += ch;
        }
        *m_data += '>';
        return;
    }
    m_data = NULL;
    if (el == "message"){
        m_from = attrs.value("from");
        m_id = attrs.value("id");
        if (attrs.value("type") == "error")
            m_bError = true;
        return;
    }
    if (el == "body"){
        m_data = &m_body;
        m_bBody = true;
        return;
    }
    if (el == "subject"){
        m_data = &m_subj;
        return;
    }
    if (el == "error"){
        m_errorCode = attrs.value("code").toUInt();
        m_data = &m_error;
        return;
    }
    if (m_bEvent){
        // Parsing <x xmlns='jabber:x:event'> tag, which contains JEP-0022 event info
        if (el == "composing"){
            m_bCompose = true;
			return;
		}
    }
    if (el == "url-data"){
        m_target = attrs.value("target");
        m_desc = QString::null;
        return;
    }
    if (el == "desc"){
        m_data = &m_desc;
        return;
    }
    if (m_bRosters && el == "item"){
        QString jid  = attrs.value("jid");
        QString name = attrs.value("name");
        if (!jid.isEmpty()){
            if (!m_contacts.isEmpty())
                m_contacts += ';';
            m_contacts += "jabber:";
            m_contacts += jid;
            if (name.isEmpty()){
                int n = jid.indexOf('@');
                if (n >= 0){
                    name = jid.left(n);
                }else{
                    name = jid;
                }
            }
            m_contacts += '/';
            m_contacts += name;
            m_contacts += ',';
            m_contacts += name;
            m_contacts += " (";
            m_contacts += jid;
            m_contacts += ')';
        }
        return;
    }
    if (el == "x"){
        if (attrs.value("xmlns") == "jabber:x:event")
            m_bEvent = true;
        else if (attrs.value("xmlns") == "jabber:x:roster")
            m_bRosters = true;
		else if (attrs.value("xmlns") == "jabber:x:encrypted"){
		    m_data = &m_enc;
		    *m_data += "-----BEGIN PGP MESSAGE-----\n\n";
		    m_bEnc = true;
		}
        return;
    }
    if (el == "html"){
        if (attrs.value("xmlns") == "http://jabber.org/protocol/xhtml-im"){
            m_bRichText = true;
            m_data = &m_richText;
        }
        return;
    }
}

void JabberClient::MessageRequest::element_end(const QString& el)
{
    if (m_bRichText){
        if (el == "html"){
            m_bRichText = false;
            m_data = NULL;
            return;
        }
        *m_data += "</";
        *m_data += el;
        *m_data += '>';
        return;
    } else
    if (el == "x") {
        if (m_bEnc){
            m_bEnc = false;
            *m_data += "\n-----END PGP MESSAGE-----\n";
        }else
            m_bRosters = false;
    } else
    if (el == "url-data"){
        if (!m_target.isEmpty()){
            if (m_desc.isEmpty())
                m_desc = m_target;
            m_targets.push_back(m_target);
            m_descs.push_back(m_desc);
        }
        m_target = QString::null;
        m_desc = QString::null;
    }
    m_data = NULL;
}

void JabberClient::MessageRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

class AgentRequest : public JabberClient::ServerRequest
{
public:
    AgentRequest(JabberClient *client, const QString &jid);
    ~AgentRequest();
protected:
    JabberAgentsInfo	data;
    QString m_data;
    QString m_jid;
    bool   m_bError;
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
    virtual void char_data(const QString& str);
};

class AgentsDiscoRequest : public JabberClient::ServerRequest
{
public:
    AgentsDiscoRequest(JabberClient *client);
protected:
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
};

class AgentDiscoRequest : public JabberClient::ServerRequest
{
public:
    AgentDiscoRequest(JabberClient *client, const QString &jid);
    ~AgentDiscoRequest();
protected:
    JabberAgentsInfo	data;
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    bool m_bError;
};

static DataDef jabberAgentsInfo[] =
    {
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

AgentDiscoRequest::AgentDiscoRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    load_data(jabberAgentsInfo, &data, NULL);
    data.ID.str() = jid;
    m_bError = false;
}

AgentDiscoRequest::~AgentDiscoRequest()
{
    if (data.Name.str().isEmpty()){
        QString jid = data.ID.str();
        int n = jid.indexOf('.');
        if (n > 0){
            jid = jid.left(n);
            data.Name.str() = jid;
        }
    }
    if (m_bError){
        data.Register.asBool() = true;
        data.Search.asBool()   = true;
    }
    if (!data.Name.str().isEmpty()){
        data.VHost.str() = m_client->VHost();
        data.Client = m_client;
// unhandled ...
//        Event e(EventAgentFound, &data);
//        e.process();
    }
    free_data(jabberAgentsInfo, &data);
}

void AgentDiscoRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "error"){
        m_bError = true;
        return;
    }
    if (el == "identity"){
        data.Name.str() = attrs.value("name");
        return;
    }
    if (el == "feature"){
        QString s = attrs.value("var");
        if (s == "jabber:iq:register")
            data.Register.asBool() = true;
        if (s == "jabber:iq:search")
            data.Search.asBool()   = true;
    }
}

AgentsDiscoRequest::AgentsDiscoRequest(JabberClient *client)
        : ServerRequest(client, _GET, NULL, client->VHost())
{
}

void AgentsDiscoRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el != "item")
        return;
    QString jid = attrs.value("jid");
    if (!jid.isEmpty()){
        AgentDiscoRequest *req = new AgentDiscoRequest(m_client, jid);
        req->start_element("query");
        req->add_attribute("xmlns", "http://jabber.org/protocol/disco#info");
        req->send();
        m_client->m_requests.push_back(req);
    }
}

AgentRequest::AgentRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    load_data(jabberAgentsInfo, &data, NULL);
    m_bError = false;
    m_jid    = jid;
}

AgentRequest::~AgentRequest()
{
    free_data(jabberAgentsInfo, &data);
    if (m_bError){
        AgentsDiscoRequest *req = new AgentsDiscoRequest(m_client);
        req->start_element("query");
        req->add_attribute("xmlns", "http://jabber.org/protocol/disco#items");
        req->send();
        m_client->m_requests.push_back(req);
    }
}

void AgentRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "agent"){
        free_data(jabberAgentsInfo, &data);
        load_data(jabberAgentsInfo, &data, NULL);
        m_data = attrs.value("jid");
        data.ID.str() = m_data;
    }else if (el == "search"){
        data.Search.asBool() = true;
    }else if (el == "register"){
        data.Register.asBool() = true;
    }else if (el == "error"){
        m_bError = true;
    }
    m_data = QString::null;
}

void AgentRequest::element_end(const QString& el)
{
    if (el == "agent"){
        if (!data.ID.str().isEmpty()){
            data.VHost.str() = m_client->VHost();
            data.Client = m_client;
// unhandled ...
//            Event e(EventAgentFound, &data);
//            e.process();
        }
    }else if (el == "name"){
        data.Name.str() = m_data;
    }
}

void AgentRequest::char_data(const QString& str)
{
    m_data += str;
}

QString JabberClient::get_agents(const QString &jid)
{
    AgentRequest *req = new AgentRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:agents");
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class AgentInfoRequest : public JabberClient::ServerRequest
{
public:
    AgentInfoRequest(JabberClient *client, const QString &jid);
    ~AgentInfoRequest();
protected:
    JabberAgentInfo		data;
    bool   m_bOption;
    QString m_data;
    QString m_jid;
    QString m_error;
    bool   m_bError;
    unsigned m_error_code;
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
    virtual void char_data(const QString& str);
};

static DataDef jabberAgentInfo[] =
    {
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };


AgentInfoRequest::AgentInfoRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _GET, NULL, jid)
{
    m_jid = jid;
    m_bOption = false;
    m_error_code = 0;
    m_bError = false;
    load_data(jabberAgentInfo, &data, NULL);
}

AgentInfoRequest::~AgentInfoRequest()
{
    free_data(jabberAgentInfo, &data);
    load_data(jabberAgentInfo, &data, NULL);
    data.ID.str() = m_jid;
    data.ReqID.str() = m_id;
    data.nOptions.asULong() = m_error_code;
    data.Label.str() = m_error;
    EventAgentInfo(&data).process();
    free_data(jabberAgentInfo, &data);
}

void AgentInfoRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "error"){
        m_bError = true;
        m_error_code = attrs.value("code").toUInt();
    }
    if (m_bError)
        return;
    if (el == "field"){
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
        data.ID.str() = m_jid;
        m_data = attrs.value("var");
        data.Field.str() = m_data;
        m_data = attrs.value("type");
        data.Type.str() = m_data;
        m_data = attrs.value("label");
        data.Label.str() = m_data;
    }
    if (el == "option"){
        m_bOption = true;
        m_data = attrs.value("label");
        set_str(&data.OptionLabels, data.nOptions.toULong(), m_data);
    }
    if (el == "x"){
        data.VHost.str() = m_client->VHost();
        data.Type.str() = "x";
        data.ReqID.str() = m_id;
        data.ID.str() = m_jid;
        EventAgentInfo(&data).process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
    }
    m_data = QString::null;
}

void AgentInfoRequest::element_end(const QString& el)
{
    if (el == "error"){
        m_error = m_data;
        m_data  = QString::null;
        m_bError = false;
        return;
    }
    if (m_bError)
        return;
    if (el == "desc"){
        data.Desc.str() = m_data;
        return;
    }
    if (el == "field"){
        if (!data.Field.str().isEmpty()){
            data.VHost.str() = m_client->VHost();
            data.ReqID.str() = m_id;
            data.ID.str() = m_jid;
            EventAgentInfo(&data).process();
            free_data(jabberAgentInfo, &data);
            load_data(jabberAgentInfo, &data, NULL);
        }
    }else if (el == "option"){
        m_bOption = false;
        QString str = get_str(data.Options, data.nOptions.toULong());
        if (!str.isEmpty())
            data.nOptions.asULong()++;
    }else if (el == "value"){
        if (m_bOption){
            set_str(&data.Options, data.nOptions.toULong(), m_data);
        }else{
            data.Value.str() = m_data;
        }
    }else if (el == "required"){
        data.bRequired.asBool() = true;
    }else if (el == "key" || el == "instructions"){
        data.Value.str() = m_data;
        data.ID.str() = m_jid;
        data.ReqID.str() = m_id;
        data.Type.str() = el;
        EventAgentInfo(&data).process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
    }else if (el != "error" && el != "iq" && el != "query" && el != "x"){
        data.Value.str() = m_data;
        data.ID.str() = m_jid;
        data.ReqID.str() = m_id;
        data.Type.str() = el;
        EventAgentInfo(&data).process();
        free_data(jabberAgentInfo, &data);
        load_data(jabberAgentInfo, &data, NULL);
    }
}

void AgentInfoRequest::char_data(const QString& str)
{
    m_data += str;
}

QString JabberClient::get_agent_info(const QString &jid, const QString &node, const QString &type)
{
    AgentInfoRequest *req = new AgentInfoRequest(this, jid);
    req->start_element("query");
    QString xmlns = "jabber:iq:";
    xmlns += type;
    req->add_attribute("xmlns", xmlns);
    req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

typedef map<my_string, QString> VALUE_MAP;

class SearchRequest : public JabberClient::ServerRequest
{
public:
    SearchRequest(JabberClient *client, const QString &jid);
    ~SearchRequest();
protected:
    JabberSearchData data;
    QString m_data;
    QString m_attr;
    list<QString> m_fields;
    VALUE_MAP    m_values;
    bool m_bReported;
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
    virtual void char_data(const QString& str);
};

static DataDef jabberSearchData[] =
    {
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },
        { "", DATA_STRLIST, 1, 0 },
        { "", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };


SearchRequest::SearchRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    load_data(jabberSearchData, &data, NULL);
    m_bReported = false;
}

SearchRequest::~SearchRequest()
{
    EventSearchDone(m_id).process();
    free_data(jabberSearchData, &data);
}

void SearchRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "reported"){
        m_bReported = true;
    }else if (el == "item"){
        free_data(jabberSearchData, &data);
        load_data(jabberSearchData, &data, NULL);
        m_data = attrs.value("jid");
        data.JID.str() = m_data;
    }else if (el == "field"){
        QString var = attrs.value("var");
        if (m_bReported){
            if (!var.isEmpty() && (var != "jid")){
                QString label = attrs.value("label");
                if (label.isEmpty())
                    label = var;
                m_values.insert(VALUE_MAP::value_type(var, label));
                m_fields.push_back(var);
            }
        }else{
            m_attr = var;
        }
    }
    m_data = QString::null;
}

void SearchRequest::element_end(const QString& el)
{
    if (el == "reported"){
        m_bReported = false;
        free_data(jabberSearchData, &data);
        load_data(jabberSearchData, &data, NULL);
        for (list<QString>::iterator it = m_fields.begin(); it != m_fields.end(); ++it){
            QString value;
            VALUE_MAP::iterator itv = m_values.find((*it));
            if (itv != m_values.end())
                value = (*itv).second;
            set_str(&data.Fields, data.nFields.toULong() * 2, value);
            set_str(&data.Fields, data.nFields.toULong() * 2 + 1, value);
            data.nFields.asULong()++;
        }
        data.ID.str() = m_id;
        EventSearch(&data).process();
        m_values.clear();
    }else if (el == "item"){
        if (!data.JID.str().isEmpty()){
            for (list<QString>::iterator it = m_fields.begin(); it != m_fields.end(); ++it){
                VALUE_MAP::iterator itv = m_values.find((*it));
                if (itv != m_values.end()){
                    QString val = (*itv).second;
                    set_str(&data.Fields, data.nFields.toULong(), val);
                }
                data.nFields.asULong()++;
            }
            data.ID.str() = m_id;
            EventSearch(&data).process();
            m_values.clear();
        }
    }else if (el == "value" || el == "field"){
        if (!m_attr.isEmpty() && !m_data.isEmpty()){
            if (m_attr == "jid"){
                data.JID.str() = m_data;
            }else{
                m_values.insert(VALUE_MAP::value_type(m_attr, m_data));
            }
        }
        m_attr = QString::null;
    }else if (el == "first"){
        data.First.str() = m_data;
    }else if (el == "last"){
        data.Last.str() = m_data;
    }else if (el == "nick"){
        data.Nick.str() = m_data;
    }else if (el == "email"){
        data.EMail.str() = m_data;
    }else if (el == "status"){
        data.Status.str() = m_data;
    }
}

void SearchRequest::char_data(const QString& str)
{
    m_data += str;
}

QString JabberClient::search(const QString &jid, const QString &node, const QString &condition)
{
    SearchRequest *req = new SearchRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:search");
    req->add_attribute("node", node);
    req->add_condition(condition, false);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

#if 0
I18N_NOOP("Password does not match");
I18N_NOOP("Low level network error");
#endif

class RegisterRequest : public JabberClient::ServerRequest
{
public:
    RegisterRequest(JabberClient *client, const QString &jid);
    ~RegisterRequest();
protected:
    QString   m_error;
    QString  *m_data;
    unsigned m_error_code;
    virtual void element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void element_end(const QString& el);
    virtual void char_data(const QString& str);
};

RegisterRequest::RegisterRequest(JabberClient *client, const QString &jid)
        : ServerRequest(client, _SET, NULL, jid)
{
    m_data = NULL;
    m_error_code = (unsigned)(-1);
}

RegisterRequest::~RegisterRequest()
{
    agentRegisterInfo ai;
    ai.id = m_id;
    ai.err_code = m_error_code;
    ai.error = m_error;
    EventAgentRegister(&ai).process();
}

void RegisterRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "error"){
        m_error_code = attrs.value("code").toUInt();
        if (m_error_code == 0)
            m_error_code = (unsigned)(-1);
        m_data = &m_error;
        return;
    }
    if (el == "iq"){
        QString type = attrs.value("type");
        if (type == "result")
            m_error_code = 0;
    }
}

void RegisterRequest::element_end(const QString&)
{
    m_data = NULL;
}

void RegisterRequest::char_data(const QString& str)
{
    if (m_data != NULL)
        *m_data += str;
}

QString JabberClient::process(const QString &jid, const QString &node, const QString &condition, const QString &type)
{
    RegisterRequest *req = new RegisterRequest(this, jid);
    req->start_element("query");
    QString xmlns = "jabber:iq:" + type;
    req->add_attribute("xmlns", xmlns);
    bool bData = (type == "data");
    if (bData)
        req->add_attribute("type", "submit");
    req->add_attribute("node", node);
    req->add_condition(condition, bData);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

void JabberClient::processList()
{
    if (getState() != Connected)
        return;
    for (list<JabberListRequest>::iterator it = m_listRequests.begin(); it != m_listRequests.end(); ++it){
        JabberListRequest &r = (*it);
        JabberClient::ServerRequest *req = new JabberClient::ServerRequest(this, JabberClient::ServerRequest::_SET, NULL, NULL);
        req->start_element("query");
        req->add_attribute("xmlns", "jabber:iq:roster");
        req->start_element("item");
        req->add_attribute("jid", r.jid);
        if (it->bDelete)
            req->add_attribute("subscription", "remove");
        if (!it->name.isEmpty())
            req->add_attribute("name", r.name);
        if (!it->bDelete)
            req->text_tag("group", r.grp);
        req->send();
        m_requests.push_back(req);
    }
    m_listRequests.clear();
}

class SendFileRequest : public JabberClient::ServerRequest
{
public:
    SendFileRequest(JabberClient *client, const QString &jid, FileMessage *msg);
    ~SendFileRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void	element_end(const QString& el);
    virtual	void	char_data(const QString& str);
    FileMessage *m_msg;
};

SendFileRequest::SendFileRequest(JabberClient *client, const QString &jid, FileMessage *msg)
        : JabberClient::ServerRequest(client, _SET, NULL, jid)
{
    m_msg = msg;
}

SendFileRequest::~SendFileRequest()
{
}

void SendFileRequest::element_start(const QString&, const QXmlAttributes&)
{
}

void SendFileRequest::element_end(const QString&)
{
}

void SendFileRequest::char_data(const QString&)
{
}

void JabberClient::sendFileRequest(FileMessage *msg, unsigned short, JabberUserData *data, const QString &fname, unsigned size)
{
    QString jid = data->getId();
    if (msg->getResource().isEmpty()){
        if (!data->getResource().isEmpty()){
            jid += '/';
            jid += data->getResource();
        }
    }else{
        jid += '/';
        jid += msg->getResource();
    }
    SendFileRequest *req = new SendFileRequest(this, jid, msg);
    req->start_element("si");
    req->add_attribute("xmlns", "http://jabber.org/protocol/si");
    req->add_attribute("profile", "http://jabber.org/protocol/si/profile/file-transfer");
    req->add_attribute("id", get_unique_id());
    req->start_element("file");
    req->add_attribute("xmlns", "http://jabber.org/protocol/si/profile/file-transfer");
    req->add_attribute("size", QString::number(size));
    req->add_attribute("name", fname);
    req->start_element("range");
    req->end_element();
    req->end_element();
    req->start_element("feature");
    req->add_attribute("xmlns", "http://jabber.org/protocol/feature-neg");
    req->start_element("x");
    req->add_attribute("xmlns", "jabber:x:data");
    req->add_attribute("type", "form");
    req->start_element("field");
    req->add_attribute("type", "list-single");
    req->add_attribute("var", "stream-method");
    req->start_element("option");
    req->text_tag("value", "http://jabber.org/protocol/bytestreams");
#if 0
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:oob");
    QString url  = "http://";
    struct in_addr addr;
    addr.s_addr = socket()->localHost();
    url += inet_ntoa(addr);
    url += ':';
	url += QString::number(port);
    url += '/';
    url += fname;
    QString desc;
    desc = msg->getText();
    req->text_tag("url", url);
    req->text_tag("desc", desc);
#endif
    req->send();
    m_requests.push_back(req);
}

void JabberClient::sendFileAccept(FileMessage *msg, JabberUserData *data)
{
    JabberFileMessage *m = static_cast<JabberFileMessage*>(msg);
    QString jid = data->getId();
    if (msg->getResource().isEmpty()){
        if (!data->getResource().isEmpty()){
            jid += '/';
            jid += data->getResource();
        }
    }else{
        jid += '/';
        jid += msg->getResource();
    }
    ServerRequest req(this, ServerRequest::_RESULT, NULL, jid, m->getID());
    req.start_element("si");
    req.add_attribute("xmlns", "http://jabber.org/protocol/si");
    req.start_element("feature");
    req.add_attribute("xmlns", "http://jabber.org/protocol/feature-neg");
    req.start_element("x");
    req.add_attribute("xmlns", "jabber:x:data");
    req.add_attribute("type", "submit");
    req.start_element("field");
    req.add_attribute("var", "stream-method");
    req.text_tag("value", "http://jabber.org/protocol/bytestreams");
    req.send();
}

class DiscoItemsRequest : public JabberClient::ServerRequest
{
public:
    DiscoItemsRequest(JabberClient *client, const QString &jid);
    ~DiscoItemsRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void	element_end(const QString& el);
    virtual	void	char_data(const QString& str);
    QString			*m_data;
    QString			m_error;
    unsigned		m_code;
};

DiscoItemsRequest::DiscoItemsRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_code = 0;
}

DiscoItemsRequest::~DiscoItemsRequest()
{
    DiscoItem item;
    item.id		= m_id;
    if (m_code){
        item.name	= m_error;
        item.node	= QString::number(m_code);
    }
    EventDiscoItem(&item).process();
}

void DiscoItemsRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "item"){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= attrs.value("jid");
        item.name	= attrs.value("name");
        item.node	= attrs.value("node");
        if (!item.jid.isEmpty()){
            EventDiscoItem(&item).process();
        }
    }
    if (el == "error"){
        m_code = attrs.value("code").toUInt();
        m_data = &m_error;
    }
}

void DiscoItemsRequest::element_end(const QString& el)
{
    if (el == "error")
        m_data = NULL;
}

void DiscoItemsRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

QString JabberClient::discoItems(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return QString::null;
    DiscoItemsRequest *req = new DiscoItemsRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/disco#items");
    req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class DiscoInfoRequest : public JabberClient::ServerRequest
{
public:
    DiscoInfoRequest(JabberClient *client, const QString &jid);
    ~DiscoInfoRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void	element_end(const QString& el);
    virtual	void	char_data(const QString& str);
    QString			*m_data;
    QString			m_error;
    QString			m_features;
    QString			m_name;
    QString			m_type;
    QString			m_category;
    unsigned		m_code;
};

DiscoInfoRequest::DiscoInfoRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_code = 0;
}

DiscoInfoRequest::~DiscoInfoRequest()
{
    if (m_code == 0){
        DiscoItem item;
        item.id         = m_id;
        item.jid        = "info";
        item.name       = m_name;
        item.category	= m_category;
        item.type       = m_type;
        item.features	= m_features;
        EventDiscoItem(&item).process();
    }
    DiscoItem item;
    item.id		= m_id;
    if (m_code){
        item.name	= m_error;
        item.node	= QString::number(m_code);
    }
    EventDiscoItem(&item).process();
}

void DiscoInfoRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "identity"){
        m_category	= attrs.value("category");
        m_name		= attrs.value("name");
        m_type		= attrs.value("type");
    }
    if (el == "feature"){
        QString feature = attrs.value("var");
        if (!feature.isEmpty()){
            if (!m_features.isEmpty())
                m_features += '\n';
            m_features += feature;
        }
    }
    if (el == "error"){
        m_code = attrs.value("code").toUInt();
        m_data = &m_error;
    }
}

void DiscoInfoRequest::element_end(const QString& el)
{
    if (el == "error")
        m_data = NULL;
}

void DiscoInfoRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

QString JabberClient::discoInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return QString::null;
    DiscoInfoRequest *req = new DiscoInfoRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/disco#info");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class BrowseRequest : public JabberClient::ServerRequest
{
public:
    BrowseRequest(JabberClient *client, const QString &jid);
    ~BrowseRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void	element_end(const QString& el);
    virtual	void	char_data(const QString& str);
    QString			*m_data;
    QString			m_jid;
    QString			m_error;
    QString			m_name;
    QString			m_type;
    QString			m_category;
    QString			m_features;
    QString			m_ns;
    unsigned		m_code;
};

BrowseRequest::BrowseRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data	= NULL;
    m_code	= 0;
    m_jid	= jid;
}

BrowseRequest::~BrowseRequest()
{
    if (!m_jid.isEmpty() && !m_name.isEmpty() && (m_code == 0)){
        DiscoItem item;
        item.id     = m_id;
        item.jid        = m_jid;
        item.name       = m_name;
        item.type       = m_type;
        item.category   = m_category;
        item.features   = m_features.toUtf8();
        EventDiscoItem(&item).process();
    }
    DiscoItem item;
    item.id		= m_id;
    if (m_code){
        item.name	= m_error;
        item.node	= QString::number(m_code);
    }
    EventDiscoItem(&item).process();
}

void BrowseRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "error"){
        m_code = attrs.value("code").toUInt();
        m_data = &m_error;
    }
    if (el == "item" || el == "service" || el == "agent" || el == "headline"){
        if (!m_jid.isEmpty() && !m_name.isEmpty()){
            DiscoItem item;
            item.id         = m_id;
            item.jid        = m_jid;
            item.name       = m_name;
            item.type       = m_type;
            item.category   = m_category;
            item.features   = m_features;
            EventDiscoItem(&item).process();
        }
        m_jid		= attrs.value("jid");
        m_name		= attrs.value("name");
        m_type		= attrs.value("type");
        m_category	= attrs.value("category");
        if (el == "headline")
            m_category = "headline";
        m_features	= QString::null;
    }
    if (el == "query"){
        m_name		= attrs.value("name");
        m_type		= attrs.value("type");
        m_category	= attrs.value("category");
    }
    if (el == "ns")
        m_data  = &m_ns;
}

void BrowseRequest::element_end(const QString& el)
{
    if (el == "error")
        m_data = NULL;
    if (el == "ns" && !m_ns.isEmpty()){
        if (!m_features.isEmpty())
            m_features += '\n';
        m_features += m_ns;
        m_ns = QString::null;
        m_data = NULL;
    }
    if ((el == "item" || el == "service" ||
            el == "agent" || el == "headline")
            && !m_jid.isEmpty()){
        DiscoItem item;
        item.id         = m_id;
        item.jid        = m_jid;
        item.name       = m_name;
        item.type       = m_type;
        item.category   = m_category;
        item.features   = m_features;
        EventDiscoItem(&item).process();
        m_jid = QString::null;
    }
}

void BrowseRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

QString JabberClient::browse(const QString &jid)
{
    if (getState() != Connected)
        return QString::null;
    BrowseRequest *req = new BrowseRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:browse");
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class VersionInfoRequest : public JabberClient::ServerRequest
{
public:
    VersionInfoRequest(JabberClient *client, const QString &jid, const QString &node);
    ~VersionInfoRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void	element_end(const QString& el);
    virtual	void	char_data(const QString& str);
    QString			*m_data;
    QString			m_jid;
    QString			m_node;
    QString			m_name;
    QString			m_version;
    QString			m_os;
};

VersionInfoRequest::VersionInfoRequest(JabberClient *client, const QString &jid, const QString &node)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_jid = jid;
    m_node = node;
}

VersionInfoRequest::~VersionInfoRequest()
{
    ClientVersionInfo info;
    info.jid = m_jid;
    info.node = m_node;
    info.name = m_name;
    info.version = m_version;
    info.os = m_os;
    EventClientVersion(&info).process();
}

void VersionInfoRequest::element_start(const QString& el, const QXmlAttributes&)
{
    if (el == "name")
        m_data = &m_name;
    if (el == "version")
        m_data = &m_version;
    if (el == "os")
        m_data = &m_os;
}

void VersionInfoRequest::element_end(const QString&)
{
    m_data = NULL;
}

void VersionInfoRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

QString JabberClient::versionInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return QString::null;
    VersionInfoRequest *req = new VersionInfoRequest(this, jid, node);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:version");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class TimeInfoRequest : public JabberClient::ServerRequest
{
public:
    TimeInfoRequest(JabberClient *client, const QString &jid);
    ~TimeInfoRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    virtual void	element_end(const QString& el);
    virtual	void	char_data(const QString& str);
    QString			*m_data;
    QString			m_jid;
    QString			m_utc;
    QString			m_tz;
    QString			m_display;
};

TimeInfoRequest::TimeInfoRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_data = NULL;
    m_jid = jid;
}

TimeInfoRequest::~TimeInfoRequest()
{
    ClientTimeInfo info;
    info.jid = m_jid;
    info.utc = m_utc;
    info.tz = m_tz;
    info.display = m_display;
    EventClientTimeInfo(&info).process();
}

void TimeInfoRequest::element_start(const QString& el, const QXmlAttributes&)
{
    if (el == "utc")
        m_data = &m_utc;
    if (el == "tz")
        m_data = &m_tz;
    if (el == "display")
        m_data = &m_display;
}

void TimeInfoRequest::element_end(const QString&)
{
    m_data = NULL;
}

void TimeInfoRequest::char_data(const QString& str)
{
    if (m_data)
        *m_data += str;
}

QString JabberClient::timeInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return QString::null;
    TimeInfoRequest *req = new TimeInfoRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:time");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class LastInfoRequest : public JabberClient::ServerRequest
{
public:
    LastInfoRequest(JabberClient *client, const QString &jid);
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    QString			m_jid;
};

LastInfoRequest::LastInfoRequest(JabberClient *client, const QString &jid)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_jid = jid;
}

void LastInfoRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "query"){
        ClientLastInfo info;
        info.jid = m_jid;
        info.seconds = attrs.value("seconds").toUInt();
        EventClientLastInfo(&info).process();
    }
}

QString JabberClient::lastInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return QString::null;
    LastInfoRequest *req = new LastInfoRequest(this, jid);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:last");
    req->add_attribute("node", node);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

class StatRequest : public JabberClient::ServerRequest
{
public:
    StatRequest(JabberClient *client, const QString &jid, const QString &id);
    ~StatRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    QString	m_id;
};

StatRequest::StatRequest(JabberClient *client, const QString &jid, const QString &id)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_id = id;
}

StatRequest::~StatRequest()
{
    DiscoItem item;
    item.id     = m_id;
    item.jid    = QString::null;
    EventDiscoItem(&item).process();
}

void StatRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "stat"){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= attrs.value("name");
        item.name	= attrs.value("units");
        item.node	= attrs.value("value");
        EventDiscoItem(&item).process();
    }
}

class StatItemsRequest : public JabberClient::ServerRequest
{
public:
    StatItemsRequest(JabberClient *client, const QString &jid, const QString &node);
    ~StatItemsRequest();
protected:
    virtual void	element_start(const QString& el, const QXmlAttributes& attrs);
    list<QString>    m_stats;
    QString			m_jid;
    QString			m_node;
};

StatItemsRequest::StatItemsRequest(JabberClient *client, const QString &jid, const QString &node)
        : JabberClient::ServerRequest(client, _GET, NULL, jid)
{
    m_jid  = jid;
    m_node = node;
}

StatItemsRequest::~StatItemsRequest()
{
    if (m_stats.empty()){
        DiscoItem item;
        item.id		= m_id;
        item.jid	= QString::null;
        EventDiscoItem(&item).process();
        return;
    }
    StatRequest *req = new StatRequest(m_client, m_jid, m_id);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/stats");
    req->add_attribute("node", m_node);
    m_client->addLang(req);
    for (list<QString>::iterator it = m_stats.begin(); it != m_stats.end(); ++it){
        req->start_element("stat");
        req->add_attribute("name", (*it));
        req->end_element();
    }
    req->send();
    m_client->m_requests.push_back(req);
}

void StatItemsRequest::element_start(const QString& el, const QXmlAttributes& attrs)
{
    if (el == "stat"){
        QString name = attrs.value("name");
        if (!name.isEmpty())
            m_stats.push_back(name);
    }
}

QString JabberClient::statInfo(const QString &jid, const QString &node)
{
    if (getState() != Connected)
        return QString::null;
    StatItemsRequest *req = new StatItemsRequest(this, jid, node);
    req->start_element("query");
    req->add_attribute("xmlns", "http://jabber.org/protocol/stats");
    req->add_attribute("node", node);
    addLang(req);
    req->send();
    m_requests.push_back(req);
    return req->m_id;
}

static char XmlLang[] = I18N_NOOP("Please translate this to short language name like 'ru' or 'de'. Do not translate this sentence!)");

void JabberClient::addLang(ServerRequest *req)
{
    QString s = i18n(XmlLang);
    if (s == XmlLang)
        return;
    req->add_attribute("xml:lang", s);
}

class ChangePasswordRequest : public JabberClient::ServerRequest
{
public:
    ChangePasswordRequest(JabberClient *client, const QString &password);
    ~ChangePasswordRequest();
protected:
    QString	m_password;
};

ChangePasswordRequest::ChangePasswordRequest(JabberClient *client, const QString &password)
        : JabberClient::ServerRequest(client, _SET, NULL, NULL)
        , m_password(password)
{}

ChangePasswordRequest::~ChangePasswordRequest()
{
    m_client->setPassword(m_password);
}

void JabberClient::changePassword(const QString &password)
{
    if (getState() != Connected)
        return;
    QString id_name = data.owner.getId();
	int pos = id_name.indexOf('@');

	if(pos != -1)
		id_name = id_name.left(pos);

    ChangePasswordRequest *req = new ChangePasswordRequest(this, password);
    req->start_element("query");
    req->add_attribute("xmlns", "jabber:iq:register");
    req->text_tag("username", id_name);
    req->text_tag("password", password);
    m_requests.push_back(req);
    req->send();
}



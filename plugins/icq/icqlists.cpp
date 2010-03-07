/***************************************************************************
                          icqlists.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "icqclient.h"
#include "icqmessage.h"
#include "core_events.h"
#include "contacts/contact.h"
#include "contacts/group.h"

#ifndef WIN32
#include <netinet/in.h>
#else
#include <winsock.h>   // htons
#endif
#include <stdio.h>
#include <vector>

#include <QTimer>
#include <QBuffer>
#include <QImage>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDateTime>

#include "log.h"

using namespace std;
using namespace SIM;

const unsigned short ICQ_SNACxLISTS_ERROR          = 0x0001;
const unsigned short ICQ_SNACxLISTS_REQxRIGHTS     = 0x0002;
const unsigned short ICQ_SNACxLISTS_RIGHTS         = 0x0003;
const unsigned short ICQ_SNACxLISTS_REQxROSTER     = 0x0005;
const unsigned short ICQ_SNACxLISTS_ROSTER         = 0x0006;
const unsigned short ICQ_SNACxLISTS_ACTIVATE       = 0x0007;
const unsigned short ICQ_SNACxLISTS_CREATE         = 0x0008;
const unsigned short ICQ_SNACxLISTS_UPDATE         = 0x0009;
const unsigned short ICQ_SNACxLISTS_DELETE         = 0x000A;
const unsigned short ICQ_SNACxLISTS_DONE           = 0x000E;
const unsigned short ICQ_SNACxLISTS_ROSTERxOK      = 0x000F;
const unsigned short ICQ_SNACxLISTS_EDIT           = 0x0011;
const unsigned short ICQ_SNACxLISTS_SAVE           = 0x0012;
const unsigned short ICQ_SNACxLISTS_FUTURE_AUTH    = 0x0014;
const unsigned short ICQ_SNACxLISTS_FUTURE_GRANT   = 0x0015;
const unsigned short ICQ_SNACxLISTS_REQUEST_AUTH   = 0x0018;
const unsigned short ICQ_SNACxLISTS_AUTHxREQUEST   = 0x0019;
const unsigned short ICQ_SNACxLISTS_AUTHxSEND	   = 0x001A;
const unsigned short ICQ_SNACxLISTS_AUTH           = 0x001B;
const unsigned short ICQ_SNACxLISTS_ADDED          = 0x001C;

const unsigned short ICQ_USER                   = 0x0000;
const unsigned short ICQ_GROUPS                 = 0x0001;
const unsigned short ICQ_VISIBLE_LIST           = 0x0002;
const unsigned short ICQ_INVISIBLE_LIST         = 0x0003;
const unsigned short ICQ_INVISIBLE_STATE        = 0x0004;
const unsigned short ICQ_PRESENCE_INFO          = 0x0005;
const unsigned short ICQ_SHORTCUT_BAR           = 0x0009;
const unsigned short ICQ_IGNORE_LIST            = 0x000E;
const unsigned short ICQ_LAST_UPDATE            = 0x000F;
const unsigned short ICQ_NON_IM                 = 0x0010;
const unsigned short ICQ_UNKNOWN                = 0x0011;
const unsigned short ICQ_IMPORT_TIME            = 0x0013;
const unsigned short ICQ_BUDDY_CHECKSUM         = 0x0014;
const unsigned short ICQ_UNKNOWN2               = 0x0019;
const unsigned short ICQ_UNKNOWN3               = 0x001A;
const unsigned short ICQ_AWAITING_AUTH          = 0x001B;

const unsigned short TLV_WAIT_AUTH  = 0x0066;
const unsigned short TLV_UNKNOWN2   = 0x006d;
const unsigned short TLV_UNKNOWN3   = 0x006e;
const unsigned short TLV_SUBITEMS   = 0x00C8;
const unsigned short TLV_SHORTCUT_BAR = 0x00cd;
const unsigned short TLV_TIME       = 0x00D4;
const unsigned short TLV_BUDDYHASH  = 0x00D5;
const unsigned short TLV_ALIAS      = 0x0131;
const unsigned short TLV_CELLULAR   = 0x013A;
const unsigned short TLV_UNKNOWN4	= 0x015c;
const unsigned short TLV_UNKNOWN5	= 0x015d;

const unsigned LIST_REQUEST_TIMEOUT = 50;

class ListServerRequest
{
public:
    ListServerRequest(unsigned short seq) : m_seq(seq), m_time(QDateTime::currentDateTime()) {}
    virtual ~ListServerRequest() {};

    virtual void process(ICQClient *client, unsigned short res)
    {
        Q_UNUSED(res);
		if(client->isSSITransaction())
		{
			client->snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_SAVE, true, false);
			client->sendPacket(true);
		}
    }

    unsigned short seq() const { return m_seq; }
    QDateTime getTime() const { return m_time; }
protected:
    unsigned short m_seq;
    QDateTime m_time;
};

class GroupServerRequest : public ListServerRequest
{
public:
    GroupServerRequest(unsigned short seq, unsigned long id, unsigned short icq_id, const QString &name);
    virtual void process(ICQClient *client, unsigned short res);
protected:
    unsigned long   m_id;
    unsigned short  m_icqId;
    QString         m_name;
};

class ContactServerRequest : public ListServerRequest
{
public:
    ContactServerRequest(unsigned short seq, const QString &screen,
                         unsigned short icq_id, unsigned short grp_id, TlvList *tlv = NULL);
    ~ContactServerRequest();
    virtual void process(ICQClient *client, unsigned short res);
protected:
    QString         m_screen;
    unsigned short  m_icqId;
    unsigned short  m_grpId;
    TlvList        *m_tlv;
};

class SetListRequest : public ListServerRequest
{
public:
    SetListRequest(unsigned short seq, const QString &screen,
                   unsigned short icq_id, unsigned short type);
    virtual void process(ICQClient *client, unsigned short res);
protected:
    QString             m_screen;
    unsigned short	m_icqId;
    unsigned short	m_type;
};

class SetBuddyRequest : public ListServerRequest
{
public:
    SetBuddyRequest(unsigned short seq, const ICQUserData *icqUserData);
    virtual void process(ICQClient *client, unsigned short res);
protected:
    const ICQUserData *m_icqUserData;
};

static char PLEASE_UPGRADE[] = "PleaseUpgrade";

void ICQClient::parseRosterItem(unsigned short type,
                                const QString &str,
                                unsigned short grp_id,
                                unsigned short id,
                                TlvList *inf,
                                bool &bIgnoreTime)
{
    int seq;

    switch (type){
    case ICQ_USER: {
            if (str.length()){
                if (str.startsWith(PLEASE_UPGRADE)){
                    log(L_DEBUG, "Upgrade warning");
                    return;
                }
                log(L_DEBUG, "User %s", qPrintable(str));
                // check for own uin in contact list
                if (!m_bAIM && (str.toULong() == getUin())) {
                    log(L_DEBUG, "Own UIN in contact list - removing!");
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, QString::null, grp_id, id);
                    m_listRequest = new ContactServerRequest(seq, QString::number(id), 0, 0);
                    break;
                }
                ListRequest *lr = findContactListRequest(str);
                if (lr){
                    log(L_DEBUG, "Request found");
                    lr->icq_id = id;
                    lr->grp_id = grp_id;
                    Contact *contact;
                    ICQUserData *data = findContact(lr->screen, NULL, false, contact);
                    if (data){
                        data->IcqID.asULong() = id;
                        data->GrpId.asULong() = grp_id;
                    }
                }else{
                    bool bChanged = false;
                    QString alias;
                    Tlv *tlv_name = inf ? (*inf)(TLV_ALIAS) : NULL;
                    if (tlv_name)
                        alias = QString::fromUtf8(*tlv_name);
                    log(L_DEBUG, "User %s [%s] id %u - group %u", qPrintable(str), qPrintable(alias), id, grp_id);
                    Contact *contact;
                    Group *grp = NULL;
                    ICQUserData *data = findGroup(grp_id, NULL, grp);
                    data = findContact(str, &alias, true, contact, grp);
					if(data)
					{
						Tlv* unknown2 = inf ? (*inf)(TLV_UNKNOWN2) : NULL;
						if(unknown2)
						{
							data->unknown2.setBinary(unknown2->byteArray());
						}
						Tlv* unknown4 = inf ? (*inf)(TLV_UNKNOWN4) : NULL;
						if(unknown4)
						{
							data->unknown4.setBinary(unknown4->byteArray());
						}
						Tlv* unknown5 = inf ? (*inf)(TLV_UNKNOWN5) : NULL;
						if(unknown5)
						{
							data->unknown5.setBinary(unknown5->byteArray());
						}
					}
                    if (inf && (*inf)(TLV_WAIT_AUTH)){
                        if (!data->WaitAuth.toBool()){
                            data->WaitAuth.asBool() = true;
                            bChanged = true;
                        }
                    } else {
                        /* if not TLV(WAIT_AUTH) we are authorized ... */
                        if (inf && !(*inf)(TLV_WAIT_AUTH)) {
                            if (data->WaitAuth.toBool()){
                                data->WaitAuth.asBool() = false;
                                bChanged = true;
                            }
                        }
                    }
                    data->IcqID.asULong() = id;
                    data->GrpId.asULong() = grp_id;
                    Tlv *tlv_phone = inf ? (*inf)(TLV_CELLULAR) : NULL;
                    if (tlv_phone){
                        data->Cellular.str() = QString::fromUtf8(*tlv_phone);
                        QString phone = trimPhone(data->Cellular.str());
                        QString phone_str = quoteChars(phone, ",");
                        phone_str += ",Private Cellular,";
                        phone_str += QString::number(CELLULAR);
                        bChanged |= contact->setPhones(phone_str, NULL);
                    }else{
                        data->Cellular.clear();
                    }
                    if (bChanged){
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                    }
                    if ((data->InfoFetchTime.toULong() == 0) && data->Uin.toULong())
                        addFullInfoRequest(data->Uin.toULong());
                }
            }else{
                bIgnoreTime = true;
            }
            break;
        }
    case ICQ_GROUPS:{
            if (str.isEmpty())
                break;
            log(L_DEBUG, "group %s %u", qPrintable(str), grp_id);
            ListRequest *lr = findGroupListRequest(grp_id);
            if (lr)
                break;
            Group *grp;
            ICQUserData *data = findGroup(grp_id, &str, grp);
            if (data->IcqID.toULong()){
                lr = findGroupListRequest((unsigned short)(data->IcqID.toULong()));
                if (lr)
                    removeListRequest(lr);
            }
            data->IcqID.asULong()   = grp_id;
            data->bChecked.asBool() = true;
            if (grp->getName() != str){
                grp->setName(str);
                EventGroup e(grp, EventGroup::eChanged);
                e.process();
            }
            break;
        }
    case ICQ_VISIBLE_LIST:{
            if (str.length()){
                log(L_DEBUG, "Visible %s", qPrintable(str));
                ListRequest *lr = findContactListRequest(str);
                if (lr)
                    lr->visible_id = id;
                if ((lr == NULL) || (lr->type != LIST_USER_DELETED)){
                    Contact *contact;
                    ICQUserData *data = findContact(str, NULL, true, contact);
                    data->ContactVisibleId.asULong() = id;
                    if ((lr == NULL) && (data->VisibleId.toULong() != id)){
                        data->VisibleId.asULong() = id;
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                    }
                    if ((data->InfoFetchTime.toULong() == 0) && data->Uin.toULong())
                        addFullInfoRequest(data->Uin.toULong());
                }
            }
            break;
        }
    case ICQ_INVISIBLE_LIST:{
            if (str.length()){
                log(L_DEBUG, "Invisible %s", qPrintable(str));
                ListRequest *lr = findContactListRequest(str);
                if (lr)
                    lr->invisible_id = id;
                if ((lr == NULL) || (lr->type != LIST_USER_DELETED)){
                    Contact *contact;
                    ICQUserData *data = findContact(str, NULL, true, contact);
                    data->ContactInvisibleId.asULong() = id;
                    if ((lr == NULL) && (data->InvisibleId.toULong() != id)){
                        data->InvisibleId.asULong() = id;
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                    }
                    if ((data->InfoFetchTime.toULong() == 0) && data->Uin.toULong())
                        addFullInfoRequest(data->Uin.toULong());
                }
            }
            break;
        }
    case ICQ_IGNORE_LIST:{
            if (str.length()){
                log(L_DEBUG, "Ignore %s", qPrintable(str));
                ListRequest *lr = findContactListRequest(str);
                if (lr)
                    lr->ignore_id = id;
                Contact *contact;
                ICQUserData *data = findContact(str, NULL, true, contact);
                if (data->IgnoreId.toULong() != id){
                    data->IgnoreId.asULong() = id;
                    if (lr == NULL){
                        contact->setIgnore(true);
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                    }
                }
            }
            break;
        }
    case ICQ_INVISIBLE_STATE:
        setContactsInvisible(id);
        break;
    case ICQ_PRESENCE_INFO:
        break;
    case ICQ_BUDDY_CHECKSUM: {
            if (str.length()){
                Tlv *tlv_buddyHash = inf ? (*inf)(TLV_BUDDYHASH) : NULL;
                if(tlv_buddyHash) {
                    const QByteArray &ba = data.owner.buddyHash.toBinary();
                    unsigned char flag, size;
                    QByteArray buddyHash;

                    flag = tlv_buddyHash->Data()[0];
                    size = tlv_buddyHash->Data()[1];
                    buddyHash.resize(size);
                    memcpy(buddyHash.data(), &tlv_buddyHash->Data()[2], size);

                    if(ba != buddyHash) {
                        data.owner.buddyHash.asBinary() = buddyHash;
                        data.owner.buddyID.asULong() = flag;
                    }
                    data.owner.buddyRosterID.asULong() = id;
                }
            }
            break;
        }
    case ICQ_LAST_UPDATE:
	case ICQ_UNKNOWN:
        break;
	case ICQ_UNKNOWN3:
        if(inf) {
            Tlv *tlv_uk3 = NULL;
            tlv_uk3 = (*inf)(TLV_UNKNOWN3);
        }
        break;
	case ICQ_SHORTCUT_BAR:
        if(inf) {
            Tlv *tlv_sc = NULL;
            tlv_sc = (*inf)(TLV_SHORTCUT_BAR);
        }
        break;
	case ICQ_UNKNOWN2:
        if(inf) {
            Tlv *tlv_uk2 = NULL;
            tlv_uk2 = (*inf)(TLV_UNKNOWN2);
        }
        break;
    case ICQ_NON_IM: {
            Tlv *tlv_name = NULL;
            Tlv *tlv_phone = NULL;
            QString alias;
            QString phone;

            if (inf) {
                tlv_name = (*inf)(TLV_ALIAS);
                if (tlv_name)
                    alias = (char*)(*tlv_name);
                tlv_phone = (*inf)(TLV_CELLULAR);
                if (tlv_phone){
                    phone = (char*)(*tlv_phone);
                }
                log (L_DEBUG,"External Contact: %s Phone: %s",
                     qPrintable(alias),
                     qPrintable(phone));
            }
            break;
        }
    case ICQ_IMPORT_TIME:{
            Tlv *tlv_time = NULL;
            QDateTime qt_time;

            if (inf) {
                tlv_time = (*inf)(TLV_TIME);
                qt_time.setTime_t((uint32_t)(*tlv_time));
                log (L_DEBUG, "Import Time %s",qPrintable(qt_time.toString()));
            }
            break;
        }
    case ICQ_AWAITING_AUTH: {
            Contact *contact;
            if (str.length()){
                log(L_DEBUG, "%s is awaiting auth", qPrintable(str));
                ICQUserData *data;
                if ((data = findContact(str, NULL, false, contact)) != NULL) {
                    addFullInfoRequest(str.toULong());
                    data->WantAuth.asBool() = true;
                } else {
                   log(L_DEBUG, "not in contact list, skipped");
                }
            }
            break;
        }
    default:
        log(L_WARN,"Unknown roster type %04X", type);
    }
}

void ICQClient::snac_lists(unsigned short type, unsigned short seq)
{
    switch (type){
    case ICQ_SNACxLISTS_ERROR:
        break;
    case ICQ_SNACxLISTS_RIGHTS:
        log(L_DEBUG, "List rights");
        break;
    case ICQ_SNACxLISTS_EDIT: {
            log(L_DEBUG, "Server begins SSI transaction");
            break;
        }
    case ICQ_SNACxLISTS_SAVE: {
            log(L_DEBUG, "Server ends SSI transaction");
            break;
        }
    case ICQ_SNACxLISTS_CREATE: {
            log(L_DEBUG, "Server adds new item");
            break;
        }
    case ICQ_SNACxLISTS_UPDATE: {
            QString name;
            unsigned short id, grp_id, type, len;
            bool tmp;

            socket()->readBuffer().unpackStr(name);
            socket()->readBuffer() >> grp_id >> id >> type >> len;
            TlvList *inf = NULL;
            if (len){
                ICQBuffer b(len);
                b.pack(socket()->readBuffer().data(socket()->readBuffer().readPos()), len);
                socket()->readBuffer().incReadPos(len);
                inf = new TlvList(b);
            }
            log(L_DEBUG, "Server updates item: group_id = %d; id = %d", grp_id, id);
            parseRosterItem(type,name,grp_id,id,inf,tmp);
            delete inf;
            break;
        }
    case ICQ_SNACxLISTS_DELETE: {
            log(L_DEBUG, "Server deletes item");
            break;
        }
    case ICQ_SNACxLISTS_ROSTER:{
            char c;
            unsigned short list_len;
            log(L_DEBUG,"Rosters");
            socket()->readBuffer() >> c;
            if (c){
                log(L_WARN, "Unknown SSI-Version 0x%02X", c);
                break;
            }
            bool bIgnoreTime = false;
            if (!m_bRosters){
                m_bRosters = true;
                m_bJoin    = false;
                setContactsInvisible(0);
                Group *grp;
                ContactList::GroupIterator it_g;
                while ((grp = ++it_g) != NULL){
                    ICQUserData *data;
                    ClientDataIterator it(grp->clientData, this);
                    while ((data = toICQUserData(++it)) != NULL){
                        if (data->IcqID.toULong() == 0){
                            data->bChecked.asBool() = true;
                            continue;
                        }
                        data->bChecked.asBool() = false;
                    }
                }
                Contact *contact;
                ContactList::ContactIterator it_c;
                while ((contact = ++it_c) != NULL){
                    ICQUserData *data;
                    ClientDataIterator it(contact->clientData, this);
                    while ((data = toICQUserData(++it)) != NULL){
                        data->bChecked.asBool() = false;
                        data->GrpId.asULong() = 0;
                        data->IgnoreId.asULong() = 0;
                        data->VisibleId.asULong() = 0;
                        data->InvisibleId.asULong() = 0;
                    }
                }
            }
            socket()->readBuffer() >> list_len;
            for (unsigned i = 0; i < list_len; i++){
                QString name;
                unsigned short id, grp_id, type, len;
                socket()->readBuffer().unpackStr(name);
                socket()->readBuffer() >> grp_id >> id >> type >> len;
                TlvList *inf = NULL;
                if (len){
                    ICQBuffer b(len);
                    b.pack(socket()->readBuffer().data(socket()->readBuffer().readPos()), len);
                    socket()->readBuffer().incReadPos(len);
                    inf = new TlvList(b);
                }
                parseRosterItem(type, name, grp_id, id, inf, bIgnoreTime);
                delete inf;
            }
            unsigned long time;
            socket()->readBuffer() >> time;
            if ((time == 0) && list_len && !bIgnoreTime)
                break;
            setContactsTime(time);
            Group *grp;
            ContactList::GroupIterator it_g;
            list<Group*> forRemove;
            while ((grp = ++it_g) != NULL){
                ICQUserData *data = toICQUserData((SIM::clientData*)grp->clientData.getData(this)); // FIXME unsafe type conversion
                QString n;
                if (grp->id())
                    n = grp->getName();
                log(L_DEBUG, "Check %ld %s %p %u", grp->id(), qPrintable(n), data, data ? data->bChecked.toBool() : 0);
                if ((data == NULL) || data->bChecked.toBool())
                    continue;
                ListRequest *lr = findGroupListRequest((unsigned short)(data->IcqID.toULong()));
                if (lr)
                    continue;
                forRemove.push_back(grp);
            }
            for (list<Group*>::iterator it = forRemove.begin(); it != forRemove.end(); ++it){
                delete *it;
            }

            Contact *contact;
            ContactList::ContactIterator it_c;
            while ((contact = ++it_c) != NULL){
                ICQUserData *data;
                SIM::clientData * client_data;
                ClientDataIterator it_d(contact->clientData);
                bool bOther = (contact->clientData.size() == 0);
                bool bMy = false;
                unsigned long newGroup = 0;
                while ((client_data = ++it_d) != NULL){
                    if (it_d.client() != this){
                        bOther = true;
                        continue;
                    }
                    data=toICQUserData(client_data); // Will get here only when client is our's so feel free to converse
                    unsigned grpId = data->GrpId.toULong();
                    ContactList::GroupIterator it_g;
                    while ((grp = ++it_g) != NULL){
                        ICQUserData *data = toICQUserData((SIM::clientData*)grp->clientData.getData(this)); // FIXME unsafe type conversion
                        if (data && (data->IcqID.toULong() == grpId))
                            break;
                    }
                    if (grp)
                        newGroup = grp->id();
                    bMy = true;
                }
                if ((int)newGroup != contact->getGroup()){
                    if ((newGroup == 0) && bOther){
                        if (bMy)
                            addContactRequest(contact);
                    }else{
                        contact->setGroup(newGroup);
                        EventContact e(contact, EventContact::eChanged);
                        e.process();
                    }
                }
            }
        }
        getContacts()->save();
        if (m_bJoin){
            EventJoinAlert(this).process();
            m_bJoin = false;
        }
    case ICQ_SNACxLISTS_ROSTERxOK:	// FALLTHROUGH
        {
            log(L_DEBUG, "Rosters OK");
            QTimer::singleShot(PING_TIMEOUT * 1000, this, SLOT(ping()));
            setPreviousPassword(QString::null);
			snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_ACTIVATE, true, false);
			sendPacket(true);
            if (m_bAIM){
                Group *grp;
                ContactList::GroupIterator it;
                while ((grp = ++it) != NULL){
                    if (grp->id())
                        break;
                }
                if (grp == NULL){
                    grp = getContacts()->group(0, true);
                    grp->setName("General");
                    EventGroup e(grp, EventGroup::eChanged);
                    e.process();
                }
                data.owner.OnlineTime.asULong() = QDateTime::currentDateTime().toTime_t();
                if (m_logonStatus == STATUS_ONLINE){
                    m_status = STATUS_ONLINE;
                    sendCapability();
                    snacICBM()->sendICMB(1, 11);
                    snacICBM()->sendICMB(2,  3);
                    snacICBM()->sendICMB(4,  3);
                    fetchProfiles();
                }else{
                    m_status = STATUS_AWAY;

                    ar_request req;
                    req.bDirect = false;
                    arRequests.push_back(req);

                    ARRequest ar;
                    ar.contact  = NULL;
                    ar.param    = &arRequests.back();
                    ar.receiver = this;
                    ar.status   = m_logonStatus;
                    EventARRequest(&ar).process();
                }
                m_snacService->sendClientReady();
                setState(Connected);
                m_bReady = true;
                snacICBM()->processSendQueue();
                break;
            }
            sendCapability();
			// 0x070b will send html text.
			// Probably, later we can use it.
            snacICBM()->sendICMB(0, 0x000b);
            snacService()->sendLogonStatus();
            snacService()->sendClientReady();
            sendMessageRequest();

            setState(Connected);
            fetchProfiles();
            break;
        }
    case ICQ_SNACxLISTS_ADDED:{
            QString screen = socket()->readBuffer().unpackScreen();
            messageReceived(new AuthMessage(MessageAdded), screen);
            break;
        }
    case ICQ_SNACxLISTS_AUTHxREQUEST:{
            QString screen = socket()->readBuffer().unpackScreen();
            QByteArray message;
            QByteArray charset;
            unsigned short have_charset;
            socket()->readBuffer().unpackStr(message);
            socket()->readBuffer() >> have_charset;
            if (have_charset){
                socket()->readBuffer().incReadPos(2);
                socket()->readBuffer().unpackStr(charset);
            }
            log(L_DEBUG, "Auth request %s", qPrintable(screen));
            Message *m = NULL;
            if (charset.isEmpty()){
                AuthMessage *msg = new AuthMessage(MessageAuthRequest);
                msg->setText(QString::fromUtf8(message));
                m = msg;
            }else{
                ICQAuthMessage *msg = new ICQAuthMessage(MessageICQAuthRequest, MessageAuthRequest);
                msg->setServerText(message);
                msg->setCharset(charset);
                m = msg;
            }
            messageReceived(m, screen);
            Contact *contact;
            ICQUserData *data = findContact(screen, NULL, false, contact);
            if (data)
                data->WantAuth.asBool() = true;
            break;
        }
    case ICQ_SNACxLISTS_FUTURE_GRANT:{
            /* we treat future grant as normal grant but it isn't the same...
               http://iserverd1.khstu.ru/oscar/snac_13_15.html */
            QString screen = socket()->readBuffer().unpackScreen();
            QByteArray message;
            Message *m = NULL;

            socket()->readBuffer().unpackStr(message);
            AuthMessage *msg = new AuthMessage(MessageAuthGranted);
            msg->setText(QString::fromUtf8(message));   // toUnicode ??
            m = msg;
            messageReceived(m, screen);
            Contact *contact;
            ICQUserData *data = findContact(screen, NULL, false, contact);
            if (data){
                data->WaitAuth.asBool() = false;
                EventContact e(contact, EventContact::eChanged);
                e.process();
                addPluginInfoRequest(data->Uin.toULong(), PLUGIN_QUERYxSTATUS);
                addPluginInfoRequest(data->Uin.toULong(), PLUGIN_QUERYxINFO);
            }
        }
    case ICQ_SNACxLISTS_AUTH:{
            QString screen = socket()->readBuffer().unpackScreen();
            char auth_ok;
            socket()->readBuffer() >> auth_ok;
            QByteArray message;
            QByteArray charset;
            unsigned short have_charset;
            socket()->readBuffer().unpackStr(message);
            socket()->readBuffer() >> have_charset;
            if (have_charset){
                socket()->readBuffer().incReadPos(2);
                socket()->readBuffer().unpackStr(charset);
            }
            log(L_DEBUG, "Auth %u %s", auth_ok, qPrintable(screen));
            Message *m = NULL;
            if (charset.isEmpty()){
                AuthMessage *msg = new AuthMessage(auth_ok ? MessageAuthGranted : MessageAuthRefused);
                msg->setText(QString::fromUtf8(message));
                m = msg;
            }else{
                ICQAuthMessage *msg = new ICQAuthMessage(auth_ok ? MessageICQAuthGranted : MessageICQAuthRefused, auth_ok ? MessageAuthGranted : MessageAuthRefused);
                msg->setServerText(message);
                msg->setCharset(charset);
                m = msg;
            }
            messageReceived(m, screen);
            if (auth_ok){
                Contact *contact;
                ICQUserData *data = findContact(screen, NULL, false, contact);
                if (data){
                    data->WaitAuth.asBool() = false;
                    EventContact e(contact, EventContact::eChanged);
                    e.process();
                    addPluginInfoRequest(data->Uin.toULong(), PLUGIN_QUERYxSTATUS);
                    addPluginInfoRequest(data->Uin.toULong(), PLUGIN_QUERYxINFO);
                }
            }
            break;
        }
    case ICQ_SNACxLISTS_DONE:
        if (m_listRequest && m_listRequest->seq() == seq)
		{
            unsigned short res;
            const char *msg;
            socket()->readBuffer() >> res;
            switch (res) {
            case 0x00:
                msg = "No errors (success)";
                break;
            case 0x02:
                msg = "Item you want to modify not found in list";
                break;
            case 0x03:
                msg = "Item you want to add already exists";
                break;
            case 0x0a:
                msg = "Error adding item (invalid id, already in list, invalid data)";
                break;
            case 0x0c:
                msg = "Can't add item. Limit for this type of items exceeded";
                break;
            case 0x0d:
                msg = "Trying to add ICQ contact to an AIM list";
                break;
            case 0x0e:
                msg = "Can't add this contact because it requires authorization";
                break;
            default:
                msg = NULL;
            }
            if (msg)
                log(L_DEBUG, "%s", msg);
            else
                log(L_DEBUG, "Unknown list request answer %u", res);
            m_listRequest->process(this, res);
            delete m_listRequest;
            m_listRequest = NULL;
            snacICBM()->processSendQueue();
        }
        break;
    default:
        log(L_WARN, "Unknown lists foodgroup type %04X", type);
    }
}

void ICQClient::listsRequest()
{
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_REQxRIGHTS, true, false);
    sendPacket(true);
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_REQxROSTER, true, false);
    unsigned long	contactsTime	= getContactsTime();
    unsigned short	contactsLength	= getContactsLength();
    socket()->writeBuffer() << contactsTime << contactsLength;
    sendPacket(true);
}

void ICQClient::sendInvisible(bool bInvisible)
{
    unsigned short cmd = ICQ_SNACxLISTS_UPDATE;
    if (getContactsInvisible() == 0){
        cmd = ICQ_SNACxLISTS_CREATE;
        setContactsInvisible((unsigned short)(get_random() & 0x7FFF));
    }
    char data = bInvisible ? 4 : 3;
    TlvList tlvs;
    tlvs += new Tlv(0xCA, 1, &data);
    sendRoster(cmd, NULL, 0, getContactsInvisible(), ICQ_INVISIBLE_STATE, &tlvs);
}

ListRequest *ICQClient::findContactListRequest(const QString &screen)
{
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (((it->type == LIST_USER_CHANGED) || (it->type == LIST_USER_DELETED)) &&
                (it->screen == screen))
            return &(*it);
    }
    return NULL;
}

ListRequest *ICQClient::findGroupListRequest(unsigned short id)
{
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        switch (it->type){
        case LIST_GROUP_DELETED:
        case LIST_GROUP_CHANGED:
            if (it->icq_id == id)
                return &(*it);
            break;
        }
    }
    return NULL;
}

void ICQClient::removeListRequest(ListRequest *lr)
{
    for (list<ListRequest>::iterator it = listRequests.begin(); it != listRequests.end(); ++it){
        if (&(*it) == lr){
            listRequests.erase(it);
            return;
        }
    }
}

void ICQClient::clearListServerRequest()
{
    if (m_listRequest){
        delete m_listRequest;
        m_listRequest = NULL;
    }
}

//-----------------------------------------------------------------------------

GroupServerRequest::GroupServerRequest(unsigned short seq, unsigned long id, unsigned short icq_id, const QString &name)
        : ListServerRequest(seq), m_id(id), m_icqId(icq_id), m_name(name)
{
}

void GroupServerRequest::process(ICQClient *client, unsigned short res)
{
    ListServerRequest::process(client, res);

    ListRequest *lr = client->findGroupListRequest(m_icqId);
    if (lr && (lr->type == LIST_GROUP_DELETED)){
        lr->icq_id = 0;
        return;
    }
    Group *group = getContacts()->group(m_id);
    if (group == NULL)
        return;
    ICQUserData *data = client->toICQUserData((SIM::clientData*)group->clientData.getData(client)); // FIXME unsafe type conversion
    if (data == NULL)
        data = client->toICQUserData((SIM::clientData*)group->clientData.createData(client)); // FIXME unsafe type conversion
    data->IcqID.asULong() = m_icqId;
    data->Alias.str() = m_name;
}

//-----------------------------------------------------------------------------

ContactServerRequest::ContactServerRequest(unsigned short seq, const QString &screen,
        unsigned short icq_id, unsigned short grp_id, TlvList *tlv)
        : ListServerRequest(seq), m_screen(screen), m_icqId(icq_id), m_grpId(grp_id), m_tlv(tlv)
{
}

ContactServerRequest::~ContactServerRequest()
{
    delete m_tlv;
}

void ContactServerRequest::process(ICQClient *client, unsigned short res)
{
    ListRequest *lr = client->findContactListRequest(m_screen);
    if (lr && (lr->type == LIST_USER_DELETED)){
        lr->screen = QString::null;
        lr->icq_id = 0;
        lr->grp_id = 0;
        return;
    }
    Contact *contact;
    ICQUserData *data = client->findContact(m_screen, NULL, true, contact);
	if(res == 0x0e)
	{
		//data->GrpId.setULong(0);
		if(data->WaitAuth.toBool())
		{
			client->ssiEndTransaction();
			client->ssiStartTransaction();
			TlvList *tlv = client->createListTlv(data, contact);
			client->ssiAddBuddy(m_screen, m_grpId, (unsigned short) data->IcqID.toULong(), 0, tlv);
			data->WaitAuth.setBool(true);
		}
		EventContact e(contact, EventContact::eChanged);
		e.process();
		client->ssiEndTransaction();
		return;
	}
    data->IcqID.asULong() = m_icqId;
    data->GrpId.asULong() = m_grpId;
    if ((data->GrpId.toULong() == 0) && data->WaitAuth.toBool()){
        data->WaitAuth.asBool() = false;
        EventContact e(contact, EventContact::eChanged);
        e.process();
    }
    if (m_tlv){
        Tlv *tlv_alias = (*m_tlv)(TLV_ALIAS);
        if (tlv_alias){
            // ok here since Alias is utf8 and TLV_ALIAS too
            data->Alias.str() = QString::fromUtf8(*tlv_alias);
        }else{
            data->Alias.clear();
        }
        Tlv *tlv_cell = (*m_tlv)(TLV_CELLULAR);
        if (tlv_cell){
            data->Cellular.str() = QString::fromUtf8(*tlv_cell);
        }else{
            data->Cellular.clear();
        }
    }
    ListServerRequest::process(client, res);
}

//-----------------------------------------------------------------------------

SetListRequest::SetListRequest(unsigned short seq, const QString &screen,
                               unsigned short icq_id, unsigned short type)
        : ListServerRequest(seq), m_screen(screen), m_icqId(icq_id), m_type(type)
{
}

void SetListRequest::process(ICQClient *client, unsigned short res)
{
    ListServerRequest::process(client, res);

    ListRequest *lr = client->findContactListRequest(m_screen);
    if (lr && (lr->type == LIST_USER_DELETED)){
        switch (m_type){
        case ICQ_VISIBLE_LIST:
            lr->visible_id = 0;
            break;
        case ICQ_INVISIBLE_LIST:
            lr->invisible_id = 0;
            break;
        case ICQ_IGNORE_LIST:
            lr->ignore_id = 0;
            break;
        }
        return;
    }
    Contact *contact;
    ICQUserData *data = client->findContact(m_screen, NULL, true, contact);
    switch (m_type){
    case ICQ_VISIBLE_LIST:
        data->ContactVisibleId.asULong() = m_icqId;
        break;
    case ICQ_INVISIBLE_LIST:
        data->ContactInvisibleId.asULong() = m_icqId;
        break;
    case ICQ_IGNORE_LIST:
        data->IgnoreId.asULong() = m_icqId;
        break;
    }
}

//-----------------------------------------------------------------------------
SetBuddyRequest::SetBuddyRequest(unsigned short seq, const ICQUserData *icqUserData)
        : ListServerRequest(seq), m_icqUserData(icqUserData)
{
}

void SetBuddyRequest::process(ICQClient *client, unsigned short res)
{
    ListServerRequest::process(client, res);

    client->listRequests.erase(client->listRequests.begin());
    // item does not exist
    if(res == 2) {
        ListRequest lr;
        lr.type        = LIST_BUDDY_CHECKSUM;
        lr.icq_id      = m_icqUserData->buddyRosterID.toULong();
        lr.icqUserData = m_icqUserData;
        client->listRequests.push_back(lr);
        client->snacICBM()->processSendQueue();
    }
    if(res != 0)
        return;
}

//-----------------------------------------------------------------------------

unsigned short ICQClient::getListId()
{
    unsigned short id;
    for (id = (unsigned short)(get_random() & 0x7FFF) ;; id++){
        id &= 0x7FFF;
        if (id == 0) continue;
        Group *group;
        ContactList::GroupIterator it_grp;
        while ((group = ++it_grp) != NULL){
            ICQUserData *data = toICQUserData((SIM::clientData*)group->clientData.getData(this)); // FIXME unsafe type conversion
            if (data == NULL)
                continue;
            if (data->IcqID.toULong() == id)
                break;
        }
        if (group)
            continue;
        Contact *contact;
        ContactList::ContactIterator it_cnt;
        while ((contact = ++it_cnt) != NULL){
            ClientDataIterator it(contact->clientData, this);
            ICQUserData *data;
            while((data = toICQUserData(++it)) != NULL){
                if((data->IcqID.toULong() == id) || (data->IgnoreId.toULong() == id) ||
                        (data->VisibleId.toULong() == id) || (data->InvisibleId.toULong() == id))
                    break;
            }
            if (data)
                break;
        }
        if (contact)
            continue;
        break;
    }
    return id;
}

TlvList *ICQClient::createListTlv(ICQUserData *data, Contact *contact)
{
    TlvList *tlv = new TlvList; //Fixme Leak, warning C6211: Leaking memory 'tlv' due to an exception. Consider using a local catch block to clean up memory: Lines: 1086, 1087, 1088
    QByteArray name = contact->getName().toUtf8();
    *tlv += new Tlv(TLV_ALIAS, (unsigned short)(name.length()), name);
    if(data->WaitAuth.toBool())
		*tlv += new Tlv(TLV_WAIT_AUTH, 0, NULL);
    QString cell = getUserCellular(contact);
    if (cell.length())
        *tlv += new Tlv(TLV_CELLULAR, (unsigned short)(cell.length()), cell.toLatin1());
	if(data->unknown2.asBinary().size() > 0)
		*tlv += new Tlv(TLV_UNKNOWN2, data->unknown2.asBinary().size() - 1, data->unknown2.asBinary().data());
	if(data->unknown4.asBinary().size() > 0)
		*tlv += new Tlv(TLV_UNKNOWN4, data->unknown4.asBinary().size() - 1, data->unknown4.asBinary().data());
	if(data->unknown5.asBinary().size() > 0)
		*tlv += new Tlv(TLV_UNKNOWN5, data->unknown5.asBinary().size() - 1, data->unknown5.asBinary().data());
    return tlv;
}

void ICQClient::uploadBuddy(const ICQUserData *data)
{
    ListRequest lr;
    lr.type        = LIST_BUDDY_CHECKSUM;
    lr.icq_id      = data->buddyRosterID.toULong();
    lr.icqUserData = data;
    listRequests.push_back(lr);
    snacICBM()->processSendQueue();
}

void ICQClient::ssiStartTransaction()
{
	log(L_DEBUG, "ICQClient::ssiStartTransaction");
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_EDIT, true, false);
    sendPacket(true);
}

void ICQClient::ssiEndTransaction()
{
	log(L_DEBUG, "ICQClient::ssiEndTransaction");
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_SAVE, true, false);
    sendPacket(true);
}

unsigned short ICQClient::ssiAddBuddy(QString& screen, unsigned short group_id, unsigned short buddy_id, unsigned short buddy_type, TlvList* tlvs)
{
	log(L_DEBUG, "ICQClient::ssiAddBuddy");
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_CREATE, true, false);
    QByteArray utfscreen = screen.toUtf8();
	socket()->writeBuffer() << (unsigned short)utfscreen.length();
	socket()->writeBuffer().pack(utfscreen.data(), utfscreen.length());
	socket()->writeBuffer() << group_id << buddy_id << buddy_type;
	if(!tlvs)
		socket()->writeBuffer() << (unsigned short) 0x0000;
	else
		socket()->writeBuffer() << *tlvs;
	sendPacket(true);
    return m_nMsgSequence;
}

unsigned short ICQClient::ssiDeleteBuddy(QString& screen, unsigned short group_id, unsigned short buddy_id, unsigned short buddy_type, TlvList* tlvs)
{
	log(L_DEBUG, "ICQClient::ssiDeleteBuddy");
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_DELETE, true, false);
    QByteArray utfscreen = screen.toUtf8();
	socket()->writeBuffer() << (unsigned short)utfscreen.length();
	socket()->writeBuffer().pack(utfscreen.data(), utfscreen.length());
	socket()->writeBuffer() << group_id << buddy_id << buddy_type;
	if(!tlvs)
		socket()->writeBuffer() << (unsigned short)0x0000;
	else
		socket()->writeBuffer() << *tlvs;
	sendPacket(true);
    return m_nMsgSequence;
}

void ICQClient::getGroupIDs(unsigned short group_id, ICQBuffer* buf)
{
	if(!buf)
		return;
    ContactList::ContactIterator it;
	Contact* contact;
	ICQUserData *data;
	while((contact = ++it) != NULL)
	{
		ClientDataIterator it(contact->clientData, this);
		data = toICQUserData(++it);
		if(!data)
			continue;
		if(data->GrpId.toULong() == group_id)
		{
			(*buf) << (unsigned short)data->IcqID.toULong();
		}
	}
}

unsigned short ICQClient::ssiAddToGroup(QString& groupname, unsigned short buddy_id, unsigned short group_id)
{
    QByteArray sName = groupname.toUtf8();
	snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_UPDATE, true, false);
	socket()->writeBuffer() << (unsigned short)sName.length();
	socket()->writeBuffer().pack(sName.data(), sName.length());
	socket()->writeBuffer() << group_id << (unsigned short)0x0000 << (unsigned short)0x0001;
	ICQBuffer b;
	getGroupIDs(group_id, &b);
	b << buddy_id;
	TlvList tlvs;
	tlvs += new Tlv(TLV_SUBITEMS, b.writePos(), b.data());
	socket()->writeBuffer() << tlvs;
	sendPacket(true);
    return m_nMsgSequence;

}

unsigned short ICQClient::ssiRemoveFromGroup(QString& groupname, unsigned short buddy_id, unsigned short group_id)
{
    ContactList::ContactIterator it;
	Contact* contact;
	ICQUserData *data;
	while((contact = ++it) != NULL)
	{
		ClientDataIterator it(contact->clientData, this);
		data = toICQUserData(++it);
		if(!data)
			continue;
		if(data->IcqID.toULong() == buddy_id)
		{
			data->GrpId.setULong(0);
			break;
		}
	}

    QByteArray sName = groupname.toUtf8();
	snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_UPDATE, true, false);
	socket()->writeBuffer() << (unsigned short)sName.length();
	socket()->writeBuffer().pack(sName.data(), sName.length());
	socket()->writeBuffer() << group_id << (unsigned short)0x0000 << (unsigned short)0x0001;
	ICQBuffer b;
	getGroupIDs(group_id, &b);
	TlvList tlvs;
	tlvs += new Tlv(TLV_SUBITEMS, b.writePos(), b.data());
	socket()->writeBuffer() << tlvs;
	sendPacket(true);
    return m_nMsgSequence;
}

unsigned short ICQClient::ssiModifyBuddy(const QString& name, unsigned short grp_id, unsigned short usr_id, unsigned short subCmd, TlvList* tlv)
{
	snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_UPDATE, true, false);
	QByteArray sName = name.toUtf8();
	socket()->writeBuffer().pack(static_cast<unsigned short>(htons(sName.size())));
	socket()->writeBuffer().pack(sName.data(), sName.size());
	socket()->writeBuffer()
		<< grp_id
		<< usr_id
		<< subCmd;
	if (tlv){
		socket()->writeBuffer() << *tlv;
	}else{
		socket()->writeBuffer() << (unsigned short)0;
	}
	sendPacket(true);
	return m_nMsgSequence;
}

unsigned short ICQClient::sendRoster(unsigned short cmd, const QString &name, unsigned short grp_id,
                                     unsigned short usr_id, unsigned short subCmd, TlvList *tlv)
{
	log(L_DEBUG, "ICQClient::sendRoster");
    // start edit SSI
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_EDIT, true, false);
    sendPacket(true);

    snac(ICQ_SNACxFOOD_LISTS, cmd, true, false);
    QByteArray sName = name.toUtf8();
    socket()->writeBuffer().pack(static_cast<unsigned short>( htons(sName.length()) ) );
    socket()->writeBuffer().pack(sName.data(), sName.length());
    socket()->writeBuffer()
    << grp_id
    << usr_id
    << subCmd;
    if (tlv){
        socket()->writeBuffer() << *tlv;
    }else{
        socket()->writeBuffer() << (unsigned short)0;
    }
    sendPacket(true);
    return m_nMsgSequence;
}

void ICQClient::sendRosterGrp(const QString &name, unsigned short grpId, unsigned short usrId)
{
    QByteArray sName = name.toUtf8();
    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_UPDATE, true, false);
    socket()->writeBuffer().pack(sName.data(), sName.length());
    socket()->writeBuffer()
    << grpId
    << (unsigned long) ICQ_GROUPS;
    if (usrId){
        socket()->writeBuffer()
        << (unsigned short) 6
        << (unsigned short) 0xC8
        << (unsigned short) 2
        << (unsigned short) usrId;
    }else{
        socket()->writeBuffer()
        << (unsigned short) 4
        << (unsigned short) 0xC8
        << (unsigned short) 0;
    }
    sendPacket(true);
}

static QString userStr(Contact *contact, const ICQUserData *data)
{
    QString name = contact ? contact->getName() : "unknown";
    return QString::number(data->Uin.toULong()) + '[' + name + ']';
}

unsigned ICQClient::processListRequest()
{
    if (m_listRequest || (getState() != Connected) || !m_bReady)
        return false;
    for (;;){
        if (listRequests.size() == 0)
            return 0;
        unsigned delay = delayTime(SNAC(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_CREATE));
        if (delay)
            return delay;
        ListRequest &lr = listRequests.front();
        unsigned short seq = 0;
        unsigned short icq_id;
        Group *group = NULL;
        Contact *contact;
        ICQUserData *data;
        unsigned grp_id = 0;
        switch (lr.type){
        case LIST_USER_CHANGED:
            data = findContact(lr.screen, NULL, false, contact);
            if (data == NULL)
                break;
            if (data->VisibleId.toULong() != data->ContactVisibleId.toULong()){
                if ((data->VisibleId.toULong() == 0) || (data->ContactVisibleId.toULong() == 0)){
                    if (data->VisibleId.toULong()){
                        log(L_DEBUG, "%s add to visible list", qPrintable(userStr(contact, data)));
                        seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data), 0, data->VisibleId.toULong(), ICQ_VISIBLE_LIST);
                    }else{
                        log(L_DEBUG, "%s remove from visible list", qPrintable(userStr(contact, data)));
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data), 0, data->ContactVisibleId.toULong(), ICQ_VISIBLE_LIST);
                    }
                    m_listRequest = new SetListRequest(seq, screen(data), data->VisibleId.toULong(), ICQ_VISIBLE_LIST);
                    break;
                }
                data->VisibleId.asULong() = data->ContactVisibleId.toULong();
            }
            if (data->InvisibleId.toULong() != data->ContactInvisibleId.toULong()){
                if ((data->InvisibleId.toULong() == 0) || (data->ContactInvisibleId.toULong() == 0)){
                    if (data->InvisibleId.toULong()){
                        log(L_DEBUG, "%s add to invisible list", qPrintable(userStr(contact, data)));
                        seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data), 0, data->InvisibleId.toULong(), ICQ_INVISIBLE_LIST);
                    }else{
                        log(L_DEBUG, "%s remove from invisible list", qPrintable(userStr(contact, data)));
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data), 0, data->ContactInvisibleId.toULong(), ICQ_INVISIBLE_LIST);
                    }
                    m_listRequest = new SetListRequest(seq, screen(data), data->InvisibleId.toULong(), ICQ_INVISIBLE_LIST);
                    break;
                }
                data->InvisibleId.asULong() = data->ContactInvisibleId.toULong();
            }
            if (contact->getIgnore() != (data->IgnoreId.toULong() != 0)){
                unsigned short ignore_id = 0;
                if (data->IgnoreId.toULong()){
                    log(L_DEBUG, "%s remove from ignore list", qPrintable(userStr(contact, data)));
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, screen(data), 0, data->IgnoreId.toULong(), ICQ_IGNORE_LIST);
                }else{
                    ignore_id = getListId();
                    log(L_DEBUG, "%s add to ignore list", qPrintable(userStr(contact, data)));
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data), 0, ignore_id, ICQ_IGNORE_LIST);
                }
                m_listRequest = new SetListRequest(seq, screen(data), ignore_id, ICQ_IGNORE_LIST);
                break;
            }
            if (contact->getGroup()){
                group = getContacts()->group(contact->getGroup());
                if (group){
                    ICQUserData *grp_data = toICQUserData((SIM::clientData*)group->clientData.getData(this)); // FIXME unsafe type conversion
                    if (grp_data)
                        grp_id = grp_data->IcqID.toULong();
                }
            }
            if (data->GrpId.toULong() != grp_id)
			{
                if (grp_id)
				{
                    if (data->GrpId.toULong() == 0)
					{
                        snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_FUTURE_AUTH, true, false);
                        socket()->writeBuffer().packScreen(screen(data));
                        socket()->writeBuffer() << 0x00000000L;
						data->WaitAuth.setBool(true);
                        sendPacket(true);
                    }
                    if (data->IcqID.toULong() == 0)
                        data->IcqID.asULong() = getListId();
                    TlvList *tlv = createListTlv(data, contact);
					/*
                    if (data->GrpId.toULong())
                        seq = sendRoster(ICQ_SNACxLISTS_DELETE, QString::null, data->GrpId.toULong(), data->IcqID.toULong());
						*/
					QString name = screen(data);
					QString groupname = group->getName();
					ssiStartTransaction();
                    if(data->GrpId.toULong())
					{
						seq = ssiDeleteBuddy(name, data->GrpId.toULong(), data->IcqID.toULong(), 0, tlv);
					}
					seq = ssiAddBuddy(name, grp_id, (unsigned short) data->IcqID.toULong(), 0, tlv);
					ssiAddToGroup(groupname, data->IcqID.toULong(), grp_id);
					data->GrpId.setULong(grp_id);
					//ssiEndTransaction();
					/*
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, screen(data), grp_id, data->IcqID.toULong(), 0, tlv);
                    sendRosterGrp(group->getName(), grp_id, data->IcqID.toULong());
					*/
                    log(L_DEBUG, "%s move to group %s", qPrintable(userStr(contact, data)), qPrintable(group->getName()));
                    m_listRequest = new ContactServerRequest(seq, screen(data), data->IcqID.toULong(), grp_id, tlv);
                }
				else
				{
                    log(L_DEBUG, "%s remove from contact list", qPrintable(userStr(contact, data)));
                    seq = sendRoster(ICQ_SNACxLISTS_DELETE, QString::null, data->GrpId.toULong(), data->IcqID.toULong());
                    m_listRequest = new ContactServerRequest(seq, screen(data), 0, 0);
                }
                break;
            }
            if ((data->IcqID.toULong() == 0) || (data->Uin.toULong() == 0))
                break;
            if (isContactRenamed(data, contact)){
                log(L_DEBUG, "%s rename", qPrintable(userStr(contact, data)));
                TlvList *tlv = createListTlv(data, contact);
                seq = sendRoster(ICQ_SNACxLISTS_UPDATE, screen(data), data->GrpId.toULong(), data->IcqID.toULong(), 0, tlv);
                m_listRequest = new ContactServerRequest(seq, screen(data), data->IcqID.toULong(), data->GrpId.toULong(), tlv);
                break;
            }
            break;
        case LIST_USER_DELETED:
            data = findContact(lr.screen, NULL, false, contact);
            if (data == NULL)
                break;
            if (lr.visible_id){
                log(L_DEBUG, "%s remove from visible list", qPrintable(lr.screen));
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen, 0, lr.visible_id, ICQ_VISIBLE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen, 0, ICQ_VISIBLE_LIST);
                break;
            }
            if (lr.invisible_id){
                log(L_DEBUG, "%s remove from invisible list", qPrintable(lr.screen));
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen, 0, lr.invisible_id, ICQ_INVISIBLE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen, 0, ICQ_INVISIBLE_LIST);
                break;
            }
            if (lr.ignore_id){
                log(L_DEBUG, "%s remove from ignore list", qPrintable(lr.screen));
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, lr.screen, 0, lr.ignore_id, ICQ_IGNORE_LIST);
                m_listRequest = new SetListRequest(seq, lr.screen, 0, ICQ_IGNORE_LIST);
                break;
            }
            if (lr.screen.length() && lr.grp_id){
                group = getContacts()->group(contact->getGroup());
				QString groupname = group->getName();
				/*
                if(group)
				{
                    ICQUserData *grp_data = toICQUserData((SIM::clientData*)group->clientData.getData(this)); // FIXME unsafe type conversion
                    if(grp_data)
                        grp_id = grp_data->IcqID.toULong();
                }
				*/
                log(L_DEBUG, "%s remove from contact list", qPrintable(lr.screen));
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, QString::null, lr.grp_id, lr.icq_id);
				seq = ssiRemoveFromGroup(groupname, lr.icq_id, lr.grp_id);
                m_listRequest = new ContactServerRequest(seq, lr.screen, 0, 0);
            }
            break;
        case LIST_GROUP_CHANGED:
            group = getContacts()->group(lr.screen.toULong());
            if (group){
                QString name = group->getName();
                data = toICQUserData((SIM::clientData*)group->clientData.getData(this)); // FIXME unsafe type conversion
                if (data){
                    icq_id = (unsigned short)(data->IcqID.toULong());
                    QString alias = data->Alias.str();
                    if (alias != name){
                        log(L_DEBUG, "rename group %s", qPrintable(group->getName()));
                        seq = sendRoster(ICQ_SNACxLISTS_UPDATE, name, icq_id, 0, ICQ_GROUPS);
                    }
                }else{
                    log(L_DEBUG, "create group %s", qPrintable(group->getName()));
                    icq_id = getListId();
                    seq = sendRoster(ICQ_SNACxLISTS_CREATE, name, icq_id, 0, ICQ_GROUPS);
                }
                if (seq)
                    m_listRequest = new GroupServerRequest(seq, group->id(), icq_id, name);
            }
            break;
        case LIST_GROUP_DELETED:
            if (lr.icq_id){
                log(L_DEBUG, "delete group");
                seq = sendRoster(ICQ_SNACxLISTS_DELETE, QString::null, lr.icq_id, 0, ICQ_GROUPS);
                m_listRequest = new GroupServerRequest(seq, 0, lr.icq_id, QString::null);
            }
            break;
		case LIST_BUDDY_CHECKSUM:
            if (lr.icqUserData){
                log(L_DEBUG, "Add/Modify buddy icon checksum");

                QImage img(getPicture());
                if(img.isNull())
                    break;

                QByteArray ba;
                QBuffer buf(&ba);
                if(!buf.open(QIODevice::WriteOnly)) {
                    log(L_ERROR, "Can't open QByteArray for writing!");
                    break;
                }
                if(!img.save(&buf, "JPEG")) {
                    log(L_ERROR, "Can't save QImage to QBuffer");
                    break;
                }
                buf.close();
                QByteArray hash = QCryptographicHash::hash(ba, QCryptographicHash::Md5);
                if(hash == this->data.owner.buddyHash.toBinary() &&
                   1 == this->data.owner.buddyID.toULong()) {
                    log(L_DEBUG, "No need to upload buddy");
                 //   break;
                }

                TlvList *tlvList = new TlvList; //Fixme Leak, warning C6211: Leaking memory 'tlvList' due to an exception. Consider using a local catch block to clean up memory: Lines: 1304, 1307, 1309, 1310, 1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1495, 1496, 1497, 1499, 1500, 1503, 1504, 1505, 1509, 1513, 1514, 1515, 1521, 1523, 1524, 1525, 1526, 1527

                ba.resize(hash.size() + 2);
                ba.data()[0] = 0x01;
                ba.data()[1] = hash.size();
                memcpy(&ba.data()[2], hash.data(), hash.size());
                *tlvList += new Tlv(TLV_ALIAS, 0, NULL);
                *tlvList += new Tlv(TLV_BUDDYHASH, ba.size(), ba.data());

				//unsigned short seq = sendRoster(lr.icq_id ? ICQ_SNACxLISTS_UPDATE : ICQ_SNACxLISTS_CREATE,
				//		"1", lr.grp_id, lr.icq_id, ICQ_BUDDY_CHECKSUM, tlvList);
				ssiStartTransaction();
				unsigned short seq = ssiModifyBuddy("1", lr.grp_id, lr.icq_id, ICQ_BUDDY_CHECKSUM, tlvList);
				ssiEndTransaction();
                m_listRequest = new SetBuddyRequest(seq, &this->data.owner);
            }
            break;
        }
        if (m_listRequest)
            break;
        listRequests.erase(listRequests.begin());
    }
    return 0;
}

void ICQClient::checkListRequest()
{
    if (m_listRequest == NULL)
        return;
    if (QDateTime::currentDateTime() > (m_listRequest->getTime().addSecs(LIST_REQUEST_TIMEOUT))){
        log(L_WARN, "List request timeout");
        m_listRequest->process(this, USHRT_MAX);
        delete m_listRequest;
        m_listRequest = NULL;
        snacICBM()->processSendQueue();
    }
}

void ICQClient::addGroupRequest(Group *group)
{
    QString name;
    name = group->getName();
    ICQUserData *data = toICQUserData((SIM::clientData*)group->clientData.getData(this)); // FIXME unsafe type conversion
    if (data == NULL){
        list<ListRequest>::iterator it;
        for (it = listRequests.begin(); it != listRequests.end(); it++){
            if (it->type != LIST_GROUP_CHANGED)
                continue;
            if (it->screen.toULong() == group->id())
                return;
        }
        ListRequest lr;
        lr.type   = LIST_GROUP_CHANGED;
        lr.screen = QString::number(group->id());
        listRequests.push_back(lr);
        snacICBM()->processSendQueue();
        return;
    }
    list<ListRequest>::iterator it;
    for (it = listRequests.begin(); it != listRequests.end(); it++){
        if (it->type != LIST_GROUP_CHANGED)
            continue;
        if (it->icq_id == data->IcqID.toULong())
            return;
    }
    QString alias = data->Alias.str();
    if (alias == name)
        return;
    ListRequest lr;
    lr.type = LIST_GROUP_CHANGED;
    lr.icq_id  = (unsigned short)(data->IcqID.toULong());
    lr.screen  = QString::number(group->id());
    listRequests.push_back(lr);
    snacICBM()->processSendQueue();
}

void ICQClient::addContactRequest(Contact *contact)
{
    ICQUserData *data;
    ClientDataIterator it(contact->clientData, this);
    while ((data = toICQUserData(++it)) != NULL){
        list<ListRequest>::iterator it;
        for (it = listRequests.begin(); it != listRequests.end(); it++){
            if (it->type != LIST_USER_CHANGED)
                continue;
            if (it->screen == screen(data))
                return;
        }

        bool bChanged = false;
        if (data->VisibleId.toULong() != data->ContactVisibleId.toULong()){
            if ((data->VisibleId.toULong() == 0) || (data->ContactVisibleId.toULong() == 0)){
                bChanged = true;
                log(L_DEBUG, "%s change visible state", qPrintable(userStr(contact, data)));
            }else{
                data->VisibleId.asULong() = data->ContactVisibleId.toULong();
            }
        }
        if (data->InvisibleId.toULong() != data->ContactInvisibleId.toULong()){
            if ((data->InvisibleId.toULong() == 0) || (data->ContactInvisibleId.toULong() == 0)){
                bChanged = true;
                log(L_DEBUG, "%s change invisible state", qPrintable(userStr(contact, data)));
            }else{
                data->InvisibleId.asULong() = data->ContactInvisibleId.toULong();
            }
        }
        if (contact->getIgnore() != (data->IgnoreId.toULong() != 0)){
            log(L_DEBUG, "%s change ignore state", qPrintable(userStr(contact, data)));
            bChanged = true;
        }
        if (!bChanged){
            unsigned grp_id = 0;
            if (contact->getGroup()){
                Group *group = getContacts()->group(contact->getGroup());
                if (group){
                    ICQUserData *grp_data = toICQUserData((SIM::clientData*)group->clientData.getData(this)); // FIXME unsafe type conversion
                    if (grp_data){
                        grp_id = grp_data->IcqID.toULong();
                    }else{
                        addGroupRequest(group);
                    }
                }
            }
            if (data->GrpId.toULong() != grp_id){
                if (grp_id == 0) {
                    // <hack>
                    // fix for #5302
                    grp_id = 1;
                    contact->setGroup(grp_id);
                    unsigned oldGrpId = data->GrpId.toULong();
                    data->GrpId.asULong() = grp_id;
                    log(L_WARN, "%s change group %u->%u, because otherewise the contact would be deleted",
                        qPrintable(userStr(contact, data)), oldGrpId, grp_id);
                    return;
                    // </hack>
                } else {
                    log(L_DEBUG, "%s change group %lu->%u",
                        qPrintable(userStr(contact, data)), data->GrpId.toULong(), grp_id);
                    bChanged = true;
                }
            }
            if (!bChanged && (data->IcqID.toULong() == 0))
                return;
            if (!bChanged && !isContactRenamed(data, contact))
                return;
        }

        ListRequest lr;
        lr.type   = LIST_USER_CHANGED;
        lr.screen = screen(data);
        listRequests.push_back(lr);
        snacICBM()->processSendQueue();
    }
}

bool ICQClient::isContactRenamed(ICQUserData *data, Contact *contact)
{
    QString alias = data->Alias.str();
    if(alias.isEmpty())
        alias.sprintf("%lu", data->Uin.toULong());

    if (contact->getName() != alias){
        log(L_DEBUG, "%lu renamed %s->%s", data->Uin.toULong(), qPrintable(alias), qPrintable(contact->getName()));
        return true;
    }
    QString cell  = getUserCellular(contact);
    QString phone = data->Cellular.str();
    if (cell != phone){
        log(L_DEBUG, "%s phone changed %s->%s", qPrintable(userStr(contact, data)), qPrintable(phone), qPrintable(cell));
        return true;
    }
    return false;
}

QString ICQClient::getUserCellular(Contact *contact)
{
    QString phones = contact->getPhones();
    while (phones.length()){
        QString phoneItem = getToken(phones, ';', false);
        QString phone = getToken(phoneItem, '/', false);
        if (phoneItem != "-")
            continue;
        QString value = getToken(phone, ',');
        getToken(phone, ',');
        if (phone.toUInt() == CELLULAR){
            return value;
        }
    }
    return QString();
}

bool ICQClient::sendAuthRequest(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    ICQUserData *data = toICQUserData((SIM::clientData*)_data); // FIXME unsafe type conversion

    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_REQUEST_AUTH, true, false);
    socket()->writeBuffer().packScreen(screen(data));
    QByteArray message;
    QString charset;
    if (hasCap(data, CAP_RTF) || hasCap(data, CAP_UTF))
	{
        message = msg->getPlainText().toUtf8();
        charset = "utf-8";
    }
	else
	{
        message = getContacts()->fromUnicode(NULL, msg->getPlainText());
    }
    socket()->writeBuffer()
    << (unsigned short)(message.length())
    << message.data()
    << (char)0x00;
    if (charset.isEmpty()){
        socket()->writeBuffer() << (char)0x00;
    }else{
        socket()->writeBuffer()
        << (char)0x01
        << (unsigned short)1
        << (unsigned short)(charset.length())
        << charset.toLatin1();
    }
    sendPacket(true);

    msg->setClient(dataName(data));
    EventSent(msg).process();
    EventMessageSent(msg).process();
    delete msg;
    return true;
}

bool ICQClient::sendAuthGranted(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    ICQUserData *data = toICQUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    data->WantAuth.asBool() = false;

    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_AUTHxSEND, true, false);
    socket()->writeBuffer().packScreen(screen(data));
    socket()->writeBuffer()
    << (char)0x01
    << (unsigned long)0;
    sendPacket(true);

    msg->setClient(dataName(data));
    EventSent(msg).process();
    EventMessageSent(msg).process();
    delete msg;
    return true;
}

bool ICQClient::sendAuthRefused(Message *msg, void *_data)
{
    if ((getState() != Connected) || (_data == NULL))
        return false;
    ICQUserData *data = toICQUserData((SIM::clientData*)_data); // FIXME unsafe type conversion
    data->WantAuth.asBool() = false;

    snac(ICQ_SNACxFOOD_LISTS, ICQ_SNACxLISTS_AUTHxSEND, true, false);
    socket()->writeBuffer().packScreen(screen(data));

    QByteArray message;
    QByteArray charset;
    if (hasCap(data, CAP_RTF) || hasCap(data, CAP_UTF)){
        message = msg->getPlainText().toUtf8();
        charset = "utf-8";
    }else{
        message = getContacts()->fromUnicode(NULL, msg->getPlainText());
    }
    socket()->writeBuffer()
    << (char) 0
    << (unsigned short)(message.length())
    << message
    << (char)0x00;
    if (charset.isEmpty()){
        socket()->writeBuffer() << (char)0x00;
    }else{
        socket()->writeBuffer() << (char)0x01
        << (unsigned short)1
        << (unsigned short)(charset.length())
        << charset;
    }
    sendPacket(true);

    msg->setClient(dataName(data));
    EventSent(msg).process();
    EventMessageSent(msg).process();
    delete msg;
    return true;
}

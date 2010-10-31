#ifndef JABBERCONTACT_H
#define JABBERCONTACT_H

#include <QSharedPointer>
#include "contacts/imcontact.h"
#include "contacts/client.h"
#include "jabberstatus.h"
#include "jabber_api.h"

class JabberClient;

class JABBER_EXPORT JabberContact : public SIM::IMContact
{
public:
    JabberContact(JabberClient* cl);

    virtual bool deserialize(const QString& data);

    virtual void serialize(QDomElement& element);
    virtual void deserialize(QDomElement& element);

    virtual SIM::Client* client();

    virtual SIM::IMStatusPtr status() const;
    virtual QString name() const;

    virtual bool sendMessage(const SIM::MessagePtr& message);
    virtual bool hasUnreadMessages();

    virtual SIM::MessagePtr dequeueUnreadMessage();
    virtual void enqueueUnreadMessage(const SIM::MessagePtr& message);

    virtual SIM::IMGroupWeakPtr group();

    virtual QString makeToolTipText();


    QString getId() const { return m_id; }
    void setId(const QString& id) { m_id = id; }

    QString getNode() const { return m_node; }
    void setNode(const QString& node) { m_node = node; }

    QString getResource() const { return m_resource; }
    void setResource(const QString& resource) { m_resource = resource; }

    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    unsigned long getStatus() const { return m_status; }
    void setStatus(unsigned long status) { m_status = status; }

    QString getFirstName() const { return m_firstName; }
    void setFirstName(const QString& name) { m_firstName = name; }

    QString getNick() const { return m_nick; }
    void setNick(const QString& nick) { m_nick = nick; }

    QString getDesc() const { return m_desc; }
    void setDesc(const QString& desc) { m_desc = desc; }

    QString getBirthday() const { return m_birthday; }
    void setBirthday(const QString& birthday) { m_birthday = birthday; }

    QString getUrl() const { return m_url; }
    void setUrl(const QString& url) { m_url = url; }

    QString getOrgName() const { return m_orgName; }
    void setOrgName(const QString& orgname) { m_orgName = orgname; }

    QString getOrgUnit() const { return m_orgUnit; }
    void setOrgUnit(const QString& orgunit) { m_orgUnit = orgunit; }

    QString getTitle() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    QString getRole() const { return m_role; }
    void setRole(const QString& role) { m_role = role; }

    QString getStreet() const { return m_street; }
    void setStreet(const QString& street) { m_street = street; }

    QString getExtAddr() const { return m_extAddr; }
    void setExtAddr(const QString& addr) { m_extAddr = addr; }

    QString getCity() const { return m_city; }
    void setCity(const QString& city) { m_city = city; }

    QString getRegion() const { return m_region; }
    void setRegion(const QString& region) { m_region = region; }

    QString getPCode() const { return m_pcode; }
    void setPCode(const QString& pcode) { m_pcode = pcode; }

    QString getCountry() const { return m_country; }
    void setCountry(const QString& country) { m_country = country; }

    QString getEmail() const { return m_email; }
    void setEmail(const QString& email) { m_email = email; }

    QString getPhone() const { return m_phone; }
    void setPhone(const QString& phone) { m_phone = phone; }

    unsigned long getStatusTime() const { return m_statusTime; }
    void setStatusTime(unsigned long statustime) { m_statusTime = statustime; }

    unsigned long getOnlineTime() const { return m_onlineTime; }
    void setOnlineTime(unsigned long onlinetime) { m_onlineTime = onlinetime; }

    unsigned long getSubscribe() const { return m_subscribe; }
    void setSubscribe(unsigned long subscribe) { m_subscribe = subscribe; }

    QString getGroup() const { return m_group; }
    void setGroup(const QString& group) { m_group = group; }

    bool isChecked() const { return m_checked; }
    void setChecked(bool c) { m_checked = c; }

    QString getTypingId() const { return m_typingId; }
    void setTypingId(const QString& id) { m_typingId = id; }

    bool getSendTypingEvents() const { return m_sendTypingEvents; }
    void setSendTypingEvents(bool b) { m_sendTypingEvents = b; }

    bool isTyping() const { return m_typing; }
    void setTyping(bool t) { m_typing = t; }

    unsigned long getComposeId() const { return m_composeId; }
    void setComposeId(unsigned long id) { m_composeId = id; }

    bool isRichText() const { return m_richText; }
    void setRichText(bool rt) { m_richText = rt; }

    bool isInvisible() const { return m_invisible; }
    void setInvisible(bool i) { m_invisible = i; }

    unsigned long getPhotoWidth() const { return m_photoSize.width(); }
    void setPhotoWidth(unsigned long width) { m_photoSize.setWidth(width); }

    unsigned long getPhotoHeight() const { return m_photoSize.height(); }
    void setPhotoHeight(unsigned long height) { m_photoSize.setHeight(height); }

    unsigned long getLogoWidth() const { return m_logoSize.width(); }
    void setLogoWidth(unsigned long width) { m_logoSize.setWidth(width); }

    unsigned long getLogoHeight() const { return m_logoSize.height(); }
    void setLogoHeight(unsigned long height) { m_logoSize.setHeight(height); }

    unsigned long getNResources() const { return m_nResources; }
    void setNResources(unsigned long nResources) { m_nResources = nResources; }

    QString getResource(int n) const { return m_resources.at(n); }
    void setResource(int n, const QString& resource) { m_resources.replace(n, resource); }
    void appendResource(const QString& resource) { m_resources.append(resource); }
    void clearResources() { m_resources.clear(); }

    QString getResourceStatus(int n) const { return m_resourceStatuses.at(n); }
    void setResourceStatus(int n, const QString& resourcestatus) { m_resourceStatuses.replace(n, resourcestatus); }
    void appendResourceStatus(const QString& resourcestatus) { m_resourceStatuses.append(resourcestatus); }
    void clearResourceStatuses() { m_resourceStatuses.clear(); }

    QString getResourceReply(int n) const { return m_resourceReplies.at(n); }
    void setResourceReply(int n, const QString& resourcereply) { m_resourceReplies.replace(n, resourcereply); }
    void appendResourceReply(const QString& resourcereply) { m_resourceReplies.append(resourcereply); }
    void clearResourceReplies() { m_resourceReplies.clear(); }

    QString getResourceStatusTime(int n) const { return m_resourceStatusTimes.at(n); }
    void setResourceStatusTime(int n, const QString& resourcestatustimes) { m_resourceStatusTimes.replace(n, resourcestatustimes); }
    void appendResourceStatusTime(const QString& resourcestatustimes) { m_resourceStatusTimes.append(resourcestatustimes); }
    void clearResourceStatusTimes() { m_resourceStatusTimes.clear(); }

    QString getResourceOnlineTime(int n) const { return m_resourceOnlineTimes.at(n); }
    void setResourceOnlineTime(int n, const QString& resourceonlinetimes) { m_resourceOnlineTimes.replace(n, resourceonlinetimes); }
    void appendResourceOnlineTime(const QString& resourceonlinetimes) { m_resourceOnlineTimes.append(resourceonlinetimes); }
    void clearResourceOnlineTimes() { m_resourceOnlineTimes.clear(); }

    QString getAutoReply() const { return m_autoReply; }
    void setAutoReply(const QString& autoreply) { m_autoReply = autoreply; }

    QString getResourceClientName(int n) const { return m_resourceClientNames.at(n); }
    void setResourceClientName(int n, const QString& resourceclientname) { m_resourceClientNames.replace(n, resourceclientname); }
    void appendResourceClientName(const QString& resourceclientname) { m_resourceClientNames.append(resourceclientname); }
    void clearResourceClientNames() { m_resourceClientNames.clear(); }

    QString getResourceClientVersion(int n) const { return m_resourceClientVersions.at(n); }
    void setResourceClientVersion(int n, const QString& resourceclientversion) { m_resourceClientVersions.replace(n, resourceclientversion); }
    void appendResourceClientVersion(const QString& resourceclientversion) { m_resourceClientVersions.append(resourceclientversion); }
    void clearResourceClientVersions() { m_resourceClientVersions.clear(); }

    QString getResourceClientOS(int n) const { return m_resourceClientOSes.at(n); }
    void setResourceClientOS(int n, const QString& resourceclientos) { m_resourceClientOSes.replace(n, resourceclientos); }
    void appendResourceClientOS(const QString& resourceclientos) { m_resourceClientOSes.append(resourceclientos); }
    void clearResourceClientOSes() { m_resourceClientOSes.clear(); }

    virtual void deserializeLine(const QString& key, const QString& value);
public:

    QString m_id;
    QString m_node;
    QString m_resource;
    QString m_name;
    unsigned long m_status;
    QString m_firstName;
    QString m_nick;
    QString m_desc;
    QString m_birthday;
    QString m_url;
    QString m_orgName;
    QString m_orgUnit;
    QString m_title;
    QString m_role;
    QString m_street;
    QString m_extAddr;
    QString m_city;
    QString m_region;
    QString m_pcode;
    QString m_country;
    QString m_email;
    QString m_phone;
    unsigned long m_statusTime;
    unsigned long m_onlineTime;
    unsigned long m_subscribe;
    QString m_group;
    bool m_checked;
    QString m_typingId;
    bool m_sendTypingEvents;
    bool m_typing;
    unsigned long m_composeId;
    bool m_richText;
    bool m_invisible;
    QSize m_photoSize;
    QSize m_logoSize;
    unsigned long m_nResources;
    QStringList m_resources;
    QStringList m_resourceStatuses;
    QStringList m_resourceReplies;
    QStringList m_resourceStatusTimes;
    QStringList m_resourceOnlineTimes;
    QString m_autoReply;
    QStringList m_resourceClientNames;
    QStringList m_resourceClientVersions;
    QStringList m_resourceClientOSes;

    JabberClient* m_client;
    JabberStatusPtr m_jabberStatus;
};

typedef QSharedPointer<JabberContact> JabberContactPtr;

#endif // JABBERCONTACT_H

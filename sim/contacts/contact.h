
#ifndef SIM_CONTACT_H
#define SIM_CONTACT_H

#include <QSharedPointer>
#include "userdata.h"
#include "propertyhub.h"
#include "clientuserdata.h"
#include "misc.h"

namespace SIM
{
    const unsigned CONTACT_TEMP             = 0x0001;
    const unsigned CONTACT_DRAG             = 0x0002;
    const unsigned CONTACT_NOREMOVE_HISTORY = 0x1000;

    const unsigned CONTACT_TEMPORARY    = CONTACT_TEMP | CONTACT_DRAG;

    class EXPORT Contact
    {
    public:
        Contact(unsigned long id = 0, Buffer *cfg = NULL);
        virtual ~Contact();
        unsigned long id() const { return m_id; }

        bool serialize(QDomElement& element);
        bool deserialize(const QDomElement& element);

        int getGroup();
        void setGroup(int g);

        QString getName();
        void setName(const QString& s);

        bool getIgnore();
        void setIgnore(bool i);

        int getLastActive();
        void setLastActive(int la);

        QString getEMails();
        void setEMails(const QString& e);

        QString getPhones();
        void setPhones(const QString& p);

        void addPhone(const QString& p);
//        QStringList getPhones();

        int getPhoneStatus();
        void setPhoneStatus(int ps);

        QString getFirstName();
        void setFirstName(const QString& n);

        QString getLastName();
        void setLastName(const QString& n);

        QString getNotes();
        void setNotes(const QString& n);

        int getFlags();
        void setFlags(int flags);

        QString getEncoding();
        void setEncoding(const QString& enc);

        PropertyHubPtr getUserData(const QString& id, bool bCreate = false);
        bool setFirstName(const QString &name, const QString &client);
        bool setLastName(const QString &name, const QString &client);
        bool setEMails(const QString &mails, const QString &client);
        bool setPhones(const QString &phones, const QString &client);
        unsigned long contactInfo(unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL);
        QString tipText();
        const DataDef *dataDef();
        void setup();

        UserDataPtr getUserData() { return m_userData; }

        // m_clientData accessors
        ClientDataIterator clientDataIterator(Client* client = NULL);
        QByteArray saveUserData() const;
        void loadUserData(Client *client, Buffer *cfg);
        IMContact* createData(Client* client);
        IMContact* getData(Client *client);
        IMContact* getData(const QString& clientName);
        QStringList clientNames();
        bool have(IMContact*);
        void sort();
        void join(Contact* c);
        void join(SIM::IMContact *cData, Contact* c);
        unsigned size();
        Client *activeClient(void *&data, Client *client);
        void freeData(SIM::IMContact*);
        void freeClientData(Client *client);
        // m_clientData accessors end
    protected:
        bool serializeMainInfo(QDomElement& element);
        bool deserializeMainInfo(const QDomElement& element);

        unsigned long m_id;
        friend class ContactList;
        friend class ContactListPrivate;

    private:
        UserDataPtr m_userData;
        ClientUserData m_clientData;

        QString m_encoding;
    };
    typedef QSharedPointer<Contact> ContactPtr;

}

#endif

// vim: set expandtab:


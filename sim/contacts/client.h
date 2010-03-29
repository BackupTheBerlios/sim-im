
#ifndef SIM_CLIENT_H
#define SIM_CLIENT_H

#include <QSharedPointer>
#include "cfg.h"
#include "contacts/contact.h"
#include "imstatus.h"
#include "simapi.h"

namespace SIM
{
    class Protocol;
    struct ClientData
    {
        Data    ManualStatus;
        Data    CommonStatus;
        Data    Password;
        Data    SavePassword;
        Data    PreviousPassword;
        Data    Invisible;
        Data    LastSend;
    };

    class EXPORT Client
    {
    public:
        Client(Protocol* protocol);
        virtual ~Client();
        enum State
        {
            Offline,
            Connecting,
            Connected,
            Error
        };
        virtual QString name() = 0;
        virtual QString dataName(void*) = 0;
        Protocol *protocol() const { return m_protocol; }
        virtual QWidget *setupWnd() = 0;
        SIM_DEPRECATED virtual void setStatus(unsigned status, bool bCommon);
        SIM_DEPRECATED virtual unsigned getStatus() const { return m_status; }

        virtual void changeStatus(const IMStatusPtr& status);
        IMStatusPtr currentStatus();

        virtual IMContact*  getOwnerContact() = 0;
        virtual void setOwnerContact(IMContact* contact) = 0;

        virtual QByteArray getConfig();

        virtual bool serialize(QDomElement& element);
        virtual bool deserialize(QDomElement& element);
        virtual bool deserialize(Buffer* cfg);

        virtual QImage userPicture(unsigned) {return QImage();};
        virtual CommandDef *configWindows();
        virtual QWidget *configWindow(QWidget *parent, unsigned id);
        virtual QWidget *searchWindow(QWidget *parent) = 0;
        void    removeGroup(Group *grp);
        void    setState(State, const QString &text = QString::null, unsigned code = 0);
        State   getState() const { return m_state; }
        virtual void contactsLoaded();
        PropertyHubPtr properties() { return m_data; }

        void setManualStatus(const IMStatusPtr& status);
        IMStatusPtr manualStatus();

        unsigned long getManualStatus() const { return m_ulStatus; }
        void setManualStatus(unsigned long status) { m_ulStatus = status; }

        bool getCommonStatus() const { return m_commonStatus; }
        void setCommonStatus(bool commonStatus) { m_commonStatus = commonStatus; }

        QString getPassword() const { return m_password; }
        void setPassword(const QString& password) { m_password = password; }

        bool getSavePassword() const { return m_savePassword; }
        void setSavePassword(bool sp) { m_savePassword = sp; }

        QString getPreviousPassword() const { return m_previousPassword; }
        void setPreviousPassword(const QString& password) { m_previousPassword = password; }

        QString getLastSend(int i) const { return m_lastSend.at(i); }
        void setLastSend(int i, const QString& ls) { return m_lastSend.replace(i, ls); }
        void appendLastSend(const QString& ls) { m_lastSend.append(ls); }
        void clearLastSend() { m_lastSend.clear(); }

        virtual bool getInvisible() const { return m_invisible; }
        virtual void setInvisible(bool b) { m_invisible = b; }

        // Deprecated interface
        SIM_DEPRECATED Client(Protocol*, Buffer *cfg);
        virtual bool compareData(void*, void*);
        virtual bool isMyData(IMContact*&, Contact*&) = 0;
        virtual bool createData(IMContact*&, Contact*) = 0;
        virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL) = 0;
        virtual void setupContact(Contact*, void *data) = 0;
        virtual bool send(Message*, void *data) = 0;
        virtual bool canSend(unsigned type, void *data) = 0;
        virtual QString contactTip(void *clientData);
        virtual CommandDef *infoWindows(Contact *contact, void *clientData);
        virtual QWidget *infoWindow(QWidget *parent, Contact *contact, void *clientData, unsigned id);
        virtual void updateInfo(Contact *contact, void *clientData);
        virtual void setClientInfo(void *data);
        virtual QString resources(void *data);
        virtual QString contactName(void *clientData);

    protected:
        void  freeData();
        //ClientData  data;
        unsigned m_status;

    private:
        void deserializeLine(const QString& key, const QString& value);
        QString cryptPassword(const QString& passwd);
        QString uncryptPassword(const QString& passwd);

        unsigned long m_ulStatus;
        bool m_commonStatus;
        QString m_password;
        bool m_savePassword;
        QString m_previousPassword;
        QStringList m_lastSend;
        bool m_invisible;

        State m_state;
        IMStatusPtr m_currentStatus;
        IMStatusPtr m_manualStatus;
        PropertyHubPtr m_data;
        Protocol* m_protocol;
    };

    typedef QSharedPointer<Client> ClientPtr;

}

#endif

// vim: set expandtab:


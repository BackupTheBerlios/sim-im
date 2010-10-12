
#ifndef SIM_GROUP_H
#define SIM_GROUP_H

#include <QSharedPointer>
#include <QWeakPointer>
#include <QBitArray>

#include "simapi.h"
#include "userdata.h"
#include "clientuserdata.h"
#include "propertyhub.h"
#include "misc.h"
#include "contacts/imgroup.h"

namespace SIM
{
    class EXPORT Group
    {
    public:
        enum Flag
        {
            flMaxFlag
        };

        Group(int id);
        virtual ~Group();

        int id() const { return m_id; }

        QString name() const;
        void setName(const QString& name);

        UserDataPtr userdata() const { return m_userData; }

        void addClientGroup(const IMGroupPtr& group);
        IMGroupPtr clientGroup(const QString& clientId) const;
        QStringList clientIds() const;

        bool flag(Flag fl) const;
        void setFlag(Flag fl, bool value);

        bool serialize(QDomElement& el);
        bool deserialize(QDomElement& el);
        bool deserialize(const QString& data);
    private:
        bool serializeMainInfo(QDomElement& element);
        bool deserializeMainInfo(const QDomElement& element);

        int m_id;
        QString m_name;
        UserDataPtr m_userData;
        QBitArray m_flags;
        QList<IMGroupPtr> m_imGroups;
    };
    typedef QSharedPointer<Group> GroupPtr;
    typedef QWeakPointer<Group> GroupWeakPtr;
}

#endif

// vim: set expandtab:


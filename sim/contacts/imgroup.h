#ifndef IMGROUP_H
#define IMGROUP_H

#include <QSharedPointer>
#include <QWeakPointer>
#include <QList>
#include <QDomElement>

namespace SIM {

class Client;
typedef QWeakPointer<Client> ClientWeakPtr;
class IMContact;
typedef QSharedPointer<IMContact> IMContactPtr;

class IMGroup
{
public:
    virtual ~IMGroup() {}
    virtual ClientWeakPtr client() = 0;
    virtual QList<IMContactPtr> contacts() = 0;

    virtual bool serialize(QDomElement& element) = 0;
    virtual bool deserialize(QDomElement& element) = 0;
};

typedef QSharedPointer<IMGroup> IMGroupPtr;
typedef QWeakPointer<IMGroup> IMGroupWeakPtr;

}

#endif // IMGROUP_H

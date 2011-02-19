#ifndef CLIENTCAPABILITIESREGISTRY_H
#define CLIENTCAPABILITIESREGISTRY_H

#include <QList>
#include "clientcapability.h"

class ClientCapabilitiesRegistry
{
public:
    ClientCapabilitiesRegistry();
    ~ClientCapabilitiesRegistry();

    ClientCapability* capabilityByGuid(const QByteArray& guid);
    ClientCapability* capabilityByName(const QString& name);
    ClientCapability* capabilityByShortId(int shortId);

private:
    void addCapability(const QByteArray& guid, int shortId, const QString& name, const QString& description);
    QList<ClientCapability*> m_capabilites;
    ClientCapability* m_nullCapability;
};

#endif // CLIENTCAPABILITIESREGISTRY_H

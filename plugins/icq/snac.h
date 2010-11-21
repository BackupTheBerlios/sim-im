
#ifndef _SNAC_H
#define _SNAC_H

#include <QObject>
#include <QByteArray>

class ICQClient;
class ICQBuffer;
class SnacHandler : public QObject
{
    Q_OBJECT
public:
    SnacHandler(ICQClient* client, unsigned short snac);
    virtual ~SnacHandler();

    unsigned short getType() { return m_snac; }
    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId) = 0;

    ICQClient* client() const;

signals:
    void error(const QString& errorMessage, bool reconnect);

protected:
    unsigned short m_snac;
    ICQClient* m_client;

};

#endif

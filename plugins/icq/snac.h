
#ifndef _SNAC_H
#define _SNAC_H

#include <QByteArray>

class ICQClient;
class ICQBuffer;
class SnacHandler
{
public:
	SnacHandler(ICQClient* client, unsigned short snac);
	virtual ~SnacHandler();

    unsigned short getType() { return m_snac; }
    virtual bool process(unsigned short subtype, const QByteArray& data, unsigned short seq) = 0;

    ICQClient* client() const;
protected:
	unsigned short m_snac;
	ICQClient* m_client;

};

#endif

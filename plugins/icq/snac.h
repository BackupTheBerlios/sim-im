
#ifndef _SNAC_H
#define _SNAC_H

class ICQClient;
class ICQBuffer;
class SnacHandler
{
public:
	SnacHandler(ICQClient* client, unsigned short snac);
	virtual ~SnacHandler();

	unsigned short getType() { return m_snac; };
	virtual bool process(unsigned short subtype, ICQBuffer* buf, unsigned short seq) = 0;

	void snac(const unsigned short subtype, bool bMsgID = true, bool bType = false);

	ICQClient* client();
protected:
	unsigned short m_snac;
	ICQClient* m_client;

};

#endif

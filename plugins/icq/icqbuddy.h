
#ifndef _ICQBUDDY_H
#define _ICQBUDDY_H

#include "snac.h"
#include "contacts.h"

class ICQClient;
class SnacIcqBuddy : public SnacHandler
{
public:
	SnacIcqBuddy(ICQClient* client);
	virtual ~SnacIcqBuddy();

	virtual bool process(unsigned short subtype, ICQBuffer* buf, unsigned short seq);
    void addBuddy(SIM::Contact *contact);
    void removeBuddy(SIM::Contact *contact);
};

#endif

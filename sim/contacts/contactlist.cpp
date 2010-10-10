#include "contactlist.h"
#include "standardcontactlist.h"

namespace SIM {

static ContactList* gs_contactList = 0;

EXPORT void createContactList()
{
    if(gs_contactList)
        return;
    gs_contactList = new StandardContactList();
}

EXPORT void destroyContactList()
{
    if(!gs_contactList)
        return;
    delete gs_contactList;
    gs_contactList = 0;
}

EXPORT void setContactList(ContactList* cl)
{
    if(gs_contactList)
        delete gs_contactList;
    gs_contactList = cl;
}

EXPORT ContactList *getContactList()
{
    return gs_contactList;
}

} // namespace SIM

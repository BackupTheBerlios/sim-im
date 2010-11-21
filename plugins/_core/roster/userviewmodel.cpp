#include "userviewmodel.h"
#include "contacts/contact.h"
#include "imagestorage/imagestorage.h"
#include <cstdio>

UserViewModel::UserViewModel(SIM::ContactList* contactList, QObject *parent) :
        QAbstractItemModel(parent), m_contactList(contactList), m_contactsCacheValid(false)
{
    m_onlineItemsParent = createIndex(OnlineRow, 0, 0);
    m_offlineItemsParent = createIndex(OfflineRow, 0, 0);
}

UserViewModel::~UserViewModel()
{

}

int UserViewModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant UserViewModel::data(const QModelIndex& index, int role) const
{
    if(index.column() != 0)
        return QVariant();
    if(!index.parent().isValid())
    {
        if(role == Qt::DisplayRole)
        {
            if(index.row() == OnlineRow)
                return I18N_NOOP("Online");
            else
                return I18N_NOOP("Offline");
        }
    }
    else
    {
        return contactData(index, role);
    }
    return QVariant();
}

QVariant UserViewModel::contactData(const QModelIndex& index, int role) const
{
    if(!m_contactsCacheValid)
        fillCaches();

    int row = index.row();
    QList<int>* ids = 0;
    if(index.parent().row() == OnlineRow)
        ids = &m_onlineContacts;
    else if(index.parent().row() == OfflineRow)
        ids = &m_offlineContacts;
    if(!ids)
        return QVariant();

    if((row < 0) || (row >= ids->size()))
        return QVariant();

    SIM::ContactPtr contact = m_contactList->contact(ids->at(row));
    if(!contact)
        return QVariant();

    switch(role)
    {
    case ContactName:
    case Qt::DisplayRole:
        {
            return contact->name();
        }
    case CurrentStatusIcon:
    case Qt::DecorationRole:
        {
            if(contact->clientContactCount() == 0)
                return SIM::getImageStorage()->pixmap("nonim");
            return contact->clientContact(0)->status()->icon();
        }
    }
    return QVariant();
}

QModelIndex UserViewModel::index(int row, int column, const QModelIndex& parent) const
{
    if(column != 0)
        return QModelIndex();
    if(!parent.isValid())
    {
        if(row == OnlineRow)
            return m_onlineItemsParent;
        else if(row == OfflineRow)
            return m_offlineItemsParent;
        else return QModelIndex();
    }
    else
    {
        if(parent.parent().isValid())
            return QModelIndex();

        int parentId = -1;
        if(parent.row() == OnlineRow)
            parentId = IdOnlineParent;
        else if(parent.row() == OfflineRow)
            parentId = IdOfflineParent;
        if(parentId < 0)
            return QModelIndex();

        return createIndex(row, column, (quint32)parentId);
    }
    return QModelIndex(); //Fixme Unreachable
}

QModelIndex UserViewModel::parent(const QModelIndex& index) const
{
    if(index.internalId() == IdOnlineParent)
        return m_onlineItemsParent;
    else if(index.internalId() == IdOfflineParent)
        return m_offlineItemsParent;
    return QModelIndex();
}

int UserViewModel::rowCount(const QModelIndex& parent) const
{
    if(!parent.isValid())
        return 2; // Online and offline
    else
    {
        fillCaches();
        if(parent.parent().isValid())
            return 0;
        if(parent.row() == OnlineRow)
            return m_onlineContacts.size();
        else if(parent.row() == OfflineRow)
            return m_offlineContacts.size();
    }
    return 0;
}

void UserViewModel::invalidateCache()
{
    m_contactsCacheValid = false;
}

void UserViewModel::fillCaches() const
{
    m_onlineContacts.clear();
    m_offlineContacts.clear();
    QList<int> allContacts = m_contactList->contactIds();
    foreach(int id, allContacts)
    {
        SIM::ContactPtr contact = m_contactList->contact(id);
        if(contact->isOnline())
            m_onlineContacts.append(id);
        else
            m_offlineContacts.append(id);
    }
    m_contactsCacheValid = true;
}


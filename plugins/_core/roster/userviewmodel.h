#ifndef USERVIEWMODEL_H
#define USERVIEWMODEL_H

#include <QAbstractItemModel>

#include "contacts/contactlist.h"
#include "core_api.h"

class CORE_EXPORT UserViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit UserViewModel(SIM::ContactList* contactList, QObject *parent = 0);
    virtual ~UserViewModel();

    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

    static const int OnlineRow = 0;
    static const int OfflineRow = 1;

    enum ContactItemRole
    {
        ContactId = Qt::UserRole + 1,
        ContactName,
        CurrentStatusIcon
    };

signals:

public slots:
    void invalidateCache();

private:
    QVariant contactData(const QModelIndex& index, int role = Qt::DisplayRole) const;
    void fillCaches() const;

    SIM::ContactList* m_contactList;
    QModelIndex m_onlineItemsParent;
    QModelIndex m_offlineItemsParent;

    static const int IdOnlineParent = 1000000;
    static const int IdOnlineBase = 1000001;
    static const int IdOfflineParent = 2000000;
    static const int IdOfflineBase = 2000001;

    mutable bool m_contactsCacheValid;
    mutable QList<int> m_onlineContacts;
    mutable QList<int> m_offlineContacts;

};

#endif // USERVIEWMODEL_H

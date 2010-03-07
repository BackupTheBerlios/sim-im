
#include "clientuserdata.h"
#include "contacts.h"
#include "clientuserdataprivate.h"
#include "client.h"

namespace SIM
{
    ClientUserData::ClientUserData()
    {
        p = new ClientUserDataPrivate;
    }

    ClientUserData::~ClientUserData()
    {
        delete p;
    }

    unsigned ClientUserData::size()
    {
        return p->size();
    }

    QString ClientUserData::property(const char *name)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            _ClientUserData &d = *it;
            Data *user_data = (Data*)d.data;
            for (const DataDef *def = d.client->protocol()->userDataDef(); def->name; def++){
                if (!strcmp(def->name, name)){
                    switch (def->type){
                        case DATA_STRING:
                        case DATA_UTF:
                            if (!user_data->str().isEmpty())
                                return user_data->str();
                        case DATA_ULONG:
                            if (user_data->toULong() != (unsigned long)(def->def_value))
                                return QString::number(user_data->toULong());
                        case DATA_LONG:
                            if (user_data->toLong() != (long)(def->def_value))
                                return QString::number(user_data->toLong());
                        default:
                            break;
                    }
                }
                user_data += def->n_values;
            }
        }
        return QString::null;
    }

    bool ClientUserData::have(void *data)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            if (it->data == data)
                return true;
        }
        return false;
    }

    Client *ClientUserData::activeClient(void *&data, Client *client)
    {
        ClientUserDataPrivate::iterator it;
        for (it = p->begin(); it != p->end(); ++it){
            if ((it->client == client) && (it->data == data))
                break;
            if (((clientData*)(it->data))->Sign.toULong() != ((clientData*)data)->Sign.toULong())
                continue;
            if (client->compareData(data, it->data))
                return NULL;
        }
        if (it == p->end())
            return NULL;
        if (client->getState() == Client::Connected)
            return client;
        for (++it; it != p->end(); ++it){
            if (it->client->getState() != Client::Connected)
                continue;
            if (((clientData*)(it->data))->Sign.toULong() != ((clientData*)data)->Sign.toULong())
                continue;
            if (client->compareData(data, it->data)){
                data = it->data;
                return it->client;
            }
        }
        return client;
    }

    QByteArray ClientUserData::save() const
    {
        QByteArray res;
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            _ClientUserData &d = *it;
            if (d.client->protocol()->description()->flags & PROTOCOL_TEMP_DATA)
                continue;
            QByteArray cfg = save_data(d.client->protocol()->userDataDef(), d.data);
            if (cfg.length()){
                if (res.length())
                    res += '\n';
                res += '[';
                res += d.client->name().toUtf8();
                res += "]\n";
                res += cfg;
            }
        }
        return res;
    }

    void ClientUserData::load(Client *client, Buffer *cfg)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            Client *c = it->client;
            if(c == client)
                return;
        }
        _ClientUserData data;
        data.client = client;
        const DataDef *def = client->protocol()->userDataDef();
        size_t size = 0;
        for (const DataDef *d = def; d->name; ++d)
            size += d->n_values;
        data.data = new Data[size];
        load_data(def, data.data, cfg);
        p->push_back(data);
    }

    void *ClientUserData::createData(Client *client)
    {
        _ClientUserData data;
        data.client = client;
        const DataDef *def = client->protocol()->userDataDef();
        size_t size = 0;
        for (const DataDef *d = def; d->name; ++d)
            size += d->n_values;
        data.data = new Data[size];
        load_data(def, data.data, NULL);
        p->push_back(data);
        return data.data;
    }

    void *ClientUserData::getData(Client *client)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            if (it->client == client)
                return it->data;
        }
        return NULL;
    }

    void ClientUserData::freeData(void *_data)
    {
        SIM::Data *data = (SIM::Data*)_data;
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end(); ++it){
            if (it->data == data){
                free_data(it->client->protocol()->userDataDef(), data);
                delete[] data;
                p->erase(it);
                return;
            }
        }
    }

    void ClientUserData::freeClientData(Client *client)
    {
        for (ClientUserDataPrivate::iterator it = p->begin(); it != p->end();){
            if (it->client != client){
                ++it;
                continue;
            }
            free_data(it->client->protocol()->userDataDef(), it->data);
            delete[] it->data;
            p->erase(it);
            it = p->begin();
        }
    }

    void ClientUserData::join(ClientUserData &data)
    {
        for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it)
            p->push_back(*it);
        data.p->clear();
        sort();
    }

    void ClientUserData::join(clientData *cData, ClientUserData &data)
    {
        for (ClientUserDataPrivate::iterator it = data.p->begin(); it != data.p->end(); ++it){
            if (it->data == &(cData->Sign)){
                p->push_back(*it);
                data.p->erase(it);
                break;
            }
        }
        sort();
    }
}

// vim: set expandtab:


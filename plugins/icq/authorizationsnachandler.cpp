#include <QString>
#include <QCryptographicHash>
#include "authorizationsnachandler.h"
#include "icqclient.h"
#include "log.h"
#include "bytearrayparser.h"
#include "oscarsocket.h"
#include "tlvlist.h"

using namespace SIM;
AuthorizationSnacHandler::AuthorizationSnacHandler(ICQClient* client) : SnacHandler(client, ICQ_SNACxFOOD_LOGIN)
{
}

bool AuthorizationSnacHandler::process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId)
{
    switch(subtype)
    {
    case SnacAuthKeyResponse:
        return handleAuthKeyResponse(data);
        break;
    case SnacAuthLoginReply:
        return handleLoginRedirect(data);
        break;
    }

    return true;
}

bool AuthorizationSnacHandler::handleAuthKeyResponse(const QByteArray& data)
{
    OscarSocket* socket = client()->oscarSocket();
    if(!socket)
        return false;

    ByteArrayParser parser(data);
    log(L_DEBUG, "Sending MD5 key");
    if(!client()->clientPersistentData->owner.getScreen().isEmpty() || client()->clientPersistentData->owner.getUin()) {
        int keylength = parser.readWord();
        QByteArray md5_key = parser.readBytes(keylength);

        TlvList tlvs;

        if (client()->clientPersistentData->owner.getUin()){
            QString uin;
            uin.sprintf("%lu", client()->clientPersistentData->owner.getUin());
            tlvs.append(Tlv(0x01, uin.toAscii()));
        }
        else
        {
            tlvs.append(Tlv(0x01, client()->clientPersistentData->owner.getScreen().toUtf8()));
        }
        QByteArray hash = QCryptographicHash::hash(md5_key +
                                                   client()->password().toAscii() +
                                                   magicHashString(), QCryptographicHash::Md5);
        tlvs.append(Tlv(0x25, hash));

        if(client()->clientPersistentData->owner.getUin()) {
            tlvs.append(Tlv(0x03, "ICQBasic"));
            tlvs.append(Tlv::fromUint16(0x16, 0x010a));
            tlvs.append(Tlv::fromUint16(0x17, 0x0014)); // major
            tlvs.append(Tlv::fromUint16(0x18, 0x0034)); // minor
            tlvs.append(Tlv::fromUint16(0x19, 0x0009));
            tlvs.append(Tlv::fromUint16(0x1A, 0x0c18));
            tlvs.append(Tlv::fromUint32(0x14, 0x0000043dL));
            tlvs.append(Tlv(0x0f, "en"));
            tlvs.append(Tlv(0x0e, "us"));
        } else {
            tlvs.append(Tlv(0x03, "AOL Instant Messenger, version 5.1.3036/WIN32")); //ToDo: Should be updated anytime
            tlvs.append(Tlv::fromUint16(0x16, (unsigned short)0x0109));
            tlvs.append(Tlv::fromUint16(0x17, (unsigned short)0x0005));
            tlvs.append(Tlv::fromUint16(0x18, (unsigned short)0x0001));
            tlvs.append(Tlv::fromUint16(0x19, (unsigned short)0x0000));
            tlvs.append(Tlv::fromUint16(0x1A, (unsigned short)0x0BDC));
            tlvs.append(Tlv::fromUint32(0x14, 0x000000D2L));
            tlvs.append(Tlv(0x0f, "en"));
            tlvs.append(Tlv(0x0e, "us"));
            tlvs.append(Tlv(0x4A, "\x01"));
        }
        socket->snac(ICQ_SNACxFOOD_LOGIN, SnacAuthMd5Login, 0, tlvs.toByteArray());
    }
    return true;
}

bool AuthorizationSnacHandler::handleLoginRedirect(const QByteArray& data)
{
    TlvList tlvs = TlvList::fromByteArray(data);
    Tlv tlv_host = tlvs.firstTlv(5);
    Tlv tlv_cookie = tlvs.firstTlv(6);
    if((!tlv_host.isValid()) || (!tlv_cookie.isValid())) {
        //socket()->error_state(I18N_NOOP("Close packet from server"));
        return false;
    }
    QString host = tlv_host.data();
    int idx = host.indexOf(':');
    if(idx == -1) {
        log(L_ERROR, "Bad host address %s", qPrintable(host));
        //socket()->error_state(I18N_NOOP("Bad host address"));
        return false;
    }
    unsigned short port = host.mid(idx + 1).toUShort();
    host = host.left(idx);

    client()->oscarSocket()->disconnectFromHost();
    client()->oscarSocket()->connectToHost(host, port);
    m_authCookie = tlv_cookie.data();
    m_authCookie.resize(m_authCookie.size());
    return true;
}

QByteArray AuthorizationSnacHandler::magicHashString()
{
    return QByteArray("AOL Instant Messenger (SM)");
}

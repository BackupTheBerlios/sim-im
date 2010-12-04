#include <QString>
#include <QCryptographicHash>
#include "authorizationsnachandler.h"
#include "icqclient.h"
#include "log.h"
#include "bytearrayparser.h"
#include "bytearraybuilder.h"
#include "oscarsocket.h"
#include "tlvlist.h"


const unsigned ICQ_LOGIN_ERRxBAD_PASSWD1			= 0x0001;
const unsigned ICQ_LOGIN_ERRxBAD_PASSWD2			= 0x0004;
const unsigned ICQ_LOGIN_ERRxBAD_PASSWD3			= 0x0005;
const unsigned ICQ_LOGIN_ERRxBAD_LOGIN				= 0x0006;
const unsigned ICQ_LOGIN_ERRxNOT_EXISTS1			= 0x0007;
const unsigned ICQ_LOGIN_ERRxNOT_EXISTS2			= 0x0008;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE1			= 0x000c;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE2			= 0x000d;
const unsigned ICQ_LOGIN_ERRxSUSPENDED1				= 0x0011;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE3			= 0x0012;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE4			= 0x0013;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE5			= 0x0014;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE6			= 0x0015;
const unsigned ICQ_LOGIN_ERRxIP_RATE_LIMIT1			= 0x0016;
const unsigned ICQ_LOGIN_ERRxIP_RATE_LIMIT2			= 0x0017;
const unsigned ICQ_LOGIN_ERRxRATE_LIMIT1			= 0x0018;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE7			= 0x001a;
const unsigned ICQ_LOGIN_ERRxOLDCLIENT1				= 0x001b;
const unsigned ICQ_LOGIN_ERRxOLDCLIENT2				= 0x001c;
const unsigned ICQ_LOGIN_ERRxRATE_LIMIT2			= 0x001d;
const unsigned ICQ_LOGIN_ERRxCANT_REGISTER			= 0x001e;
const unsigned ICQ_LOGIN_ERRxUNAVAILABLE8			= 0x001f;
const unsigned ICQ_LOGIN_ERRxINVALID_ID 			= 0x0020;
const unsigned ICQ_LOGIN_ERRxTOO_YOUNG				= 0x0022;

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
    Tlv tlv_error = tlvs.firstTlv(8);
    if (tlv_error.isValid()) {
        int error = tlv_error.toUint16();
        emitError(error);
        return false;
    }

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

void AuthorizationSnacHandler::emitError(int errorCode)
{
    QString errString;
    bool reconnect = false;
    switch(errorCode) {
    case ICQ_LOGIN_ERRxOLDCLIENT1:
    case ICQ_LOGIN_ERRxOLDCLIENT2:
        errString = I18N_NOOP("This client is outdated");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxIP_RATE_LIMIT1:
    case ICQ_LOGIN_ERRxIP_RATE_LIMIT2:
        errString = I18N_NOOP("Too many clients from same IP");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxRATE_LIMIT1:
    case ICQ_LOGIN_ERRxRATE_LIMIT2:
        errString = I18N_NOOP("Rate limit");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxBAD_PASSWD1:
    case ICQ_LOGIN_ERRxBAD_PASSWD2:
    case ICQ_LOGIN_ERRxBAD_PASSWD3:
        errString = I18N_NOOP("Invalid UIN and password combination");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxNOT_EXISTS1:
    case ICQ_LOGIN_ERRxNOT_EXISTS2:
        errString = I18N_NOOP("Non-existant UIN");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxBAD_LOGIN:
        errString = I18N_NOOP("Bad login procedure");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxUNAVAILABLE1:
    case ICQ_LOGIN_ERRxUNAVAILABLE2:
    case ICQ_LOGIN_ERRxUNAVAILABLE3:
    case ICQ_LOGIN_ERRxUNAVAILABLE4:
    case ICQ_LOGIN_ERRxUNAVAILABLE5:
    case ICQ_LOGIN_ERRxUNAVAILABLE6:
    case ICQ_LOGIN_ERRxUNAVAILABLE7:
    case ICQ_LOGIN_ERRxUNAVAILABLE8:
        errString = I18N_NOOP("Service temporarly unavailable");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxINVALID_ID:
        errString = I18N_NOOP("Invalid SecureID");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxTOO_YOUNG:
        errString = I18N_NOOP("Too young!");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxSUSPENDED1:
        errString = I18N_NOOP("UIN was suspended");
        reconnect = false;
        break;
    case ICQ_LOGIN_ERRxCANT_REGISTER:
        errString = I18N_NOOP("Can't login to ICQ network - Please try again later");
        reconnect = false;
        break;
    case 0:
        return;
    default:
        errString = "Unknown error ";
        errString += QString::number(errorCode);
    }

    log(L_ERROR, "%s", qPrintable(errString));
    emit error(errString, reconnect);
}

bool AuthorizationSnacHandler::handleLoginAndCloseChannels(int channel, const QByteArray& data)
{
    if(channel == OscarSocket::FlapChannelNewConnection)
        return handleNewConnection(data);
    else if(channel == OscarSocket::FlapChannelCloseConnection)
        return handleCloseConnection(data);
    return false;
}

bool AuthorizationSnacHandler::handleNewConnection(const QByteArray& data)
{
    if(m_authCookie.size()) {
        ByteArrayBuilder builder;
        builder.appendDword(1);
        TlvList list;
        list.append(Tlv(6, m_authCookie));
        builder.appendBytes(list.toByteArray());

        client()->oscarSocket()->flap(OscarSocket::FlapChannelNewConnection, builder.getArray());
        m_authCookie.clear();
        return true;
    }
    if(client()->clientPersistentData->owner.getUin() && !client()->getUseMD5()) {
        QByteArray pswd = cryptPassword(client()->password());
        log(L_DEBUG, "Login %lu [%s]", client()->clientPersistentData->owner.getUin(), /*pswd.c_str()*/"");
        QString uin;
        uin.sprintf("%lu", client()->clientPersistentData->owner.getUin());

        ByteArrayBuilder builder;
        builder.appendDword(1);
        TlvList list;
        list.append(Tlv(0x01, uin.toAscii()));
        list.append(Tlv(0x02, pswd));
        list.append(Tlv(0x03, QByteArray("ICQBasic")));
        list.append(Tlv::fromUint16(0x0016, 0x10a));
        list.append(Tlv::fromUint16(0x0017, 0x0014));
        list.append(Tlv::fromUint16(0x0018, 0x0034));
        list.append(Tlv::fromUint16(0x0019, 0x0000));
        list.append(Tlv::fromUint16(0x001a, 0x0c18));
        list.append(Tlv::fromUint32(0x0014, 0x0000043dL));
        list.append(Tlv(0x0f, QByteArray("en")));
        list.append(Tlv(0x0e, QByteArray("us")));
        builder.appendBytes(list.toByteArray());

        client()->oscarSocket()->flap(OscarSocket::FlapChannelNewConnection, builder.getArray());
        return true;
    }
    if (!client()->clientPersistentData->owner.getScreen().isEmpty() || client()->getUseMD5()){
        log(L_DEBUG, "Requesting MD5 salt");
        ByteArrayBuilder builder;
        builder.appendDword(1);
        client()->oscarSocket()->flap(OscarSocket::FlapChannelNewConnection, builder.getArray());

        TlvList list;

        if(client()->clientPersistentData->owner.getUin()) {
            QString uin = QString::number(client()->clientPersistentData->owner.getUin());
            list.append(Tlv(0x0001, uin.toUtf8()));
        } else {
            list.append(Tlv(0x0001, client()->clientPersistentData->owner.getScreen().toUtf8()));
        }
        list.append(Tlv(0x004B, QByteArray()));
        list.append(Tlv(0x005A, QByteArray()));
        client()->oscarSocket()->snac(ICQ_SNACxFOOD_LOGIN, SnacAuthKeyRequest, 0, list.toByteArray());
        return true;
    }
    return false;
}

bool AuthorizationSnacHandler::handleCloseConnection(const QByteArray& data)
{
    return handleLoginRedirect(data);
}

QByteArray AuthorizationSnacHandler::magicHashString()
{
    return QByteArray("AOL Instant Messenger (SM)");
}

QByteArray AuthorizationSnacHandler::cryptPassword(const QString& password)
{
    unsigned char xor_table[] =
        {
            0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
            0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c
        };
    QByteArray pswd = password.toAscii();
    char buf[8];
    int len=0;
    for (int j = 0; j < 8; j++)
    {
        char c = pswd[j];
        if (c == 0)
            break;
        c = (char)(c ^ xor_table[j]);
        buf[j] = c;
        len++;
    }
    QByteArray res( buf,len );
    return res;
}

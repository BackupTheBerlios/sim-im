#ifndef AUTHORIZATIONSNACHANDLER_H
#define AUTHORIZATIONSNACHANDLER_H

#include "snac.h"
#include "icq_defines.h"

class ICQ_EXPORT AuthorizationSnacHandler : public SnacHandler
{
public:
    AuthorizationSnacHandler(ICQClient* client);

    bool handleLoginAndCloseChannels(int channel, const QByteArray& data);

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    static QByteArray magicHashString();
    static const int SnacAuthMd5Login = 0x02;
    static const int SnacAuthLoginReply = 0x03;
    static const int SnacAuthKeyRequest = 0x06;
    static const int SnacAuthKeyResponse = 0x07;

private:
    bool handleAuthKeyResponse(const QByteArray& data);
    bool handleLoginRedirect(const QByteArray& data);

    bool handleNewConnection(const QByteArray& data);
    bool handleCloseConnection(const QByteArray& data);

    void emitError(int errorCode);

    QByteArray m_authCookie;

    friend class ICQClient;
};

#endif // AUTHORIZATIONSNACHANDLER_H
